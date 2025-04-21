#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include "Coordinator.h"
#include "Merging.h"
#include "SearcherClient.h"
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <vector>

    
CoordinatorImpl::CoordinatorImpl(const std::string& etcd_addr) 
    : etcd_client(EtcdClient(etcd_addr)) {
}

grpc::Status CoordinatorImpl::ProcessSearchRequest(grpc::ServerContext* context, const SearchRequest* request, SearchResponse* response) {
    std::cout << "Hello from Coordinator Server!" << std::endl;
    
    UpdateSearchersState(request);

    auto index_id = request->index_id();
    std::vector<SearchResponse> search_responses;
    size_t k = request->k();

    for (auto& [shard_id, searchers] : searchers_map[index_id]) {
        for (auto& client : searchers) {
            auto stats_collector = searchers_stats[index_id][shard_id].CreateResponseData();
            search_responses.emplace_back(std::move(AskSingleSearcher(client, request, std::move(stats_collector))));
        }
        UpdateStatsIfNeeded(index_id, shard_id, searchers_stats[index_id][shard_id]);
    }
    *response = MergeSearcherAnswers(search_responses, k);

    return grpc::Status::OK;
}

void CoordinatorImpl::UpdateSearchersState(const SearchRequest* request) {
    const auto& index_id = request->index_id();

    auto update_with_index_shard_info = [this](const std::string& index_id, const std::string& shard_id) {
        auto hosts = etcd_client.ListSearcherHosts(index_id, shard_id);
        searchers_map[index_id][shard_id] = std::vector<SearcherClient>();

        // create searcher clients for given hosts
        for (const auto& addr : hosts) {
            searchers_map[index_id][shard_id].emplace_back(CreateSearcherClient(addr));
        }

        // create StatsManager for given index and shard
        if (searchers_stats.find(index_id) == searchers_stats.end()) {
            searchers_stats[index_id] = ShardToStats();
            if (searchers_stats[index_id].find(shard_id) == searchers_stats[index_id].end()) {
                searchers_stats[index_id].emplace(shard_id, StatsManager());
            }
        }
    };

    if (request->has_shard_id()) {
        const auto& shard_id = request->shard_id();
        update_with_index_shard_info(index_id, shard_id);
    } else {
        auto shards = etcd_client.ListShardIds(index_id);
        for (const auto& shard_id : shards) {
            update_with_index_shard_info(index_id, shard_id);
        }
    }
}

SearchResponse CoordinatorImpl::AskSingleSearcher(const SearcherClient& client, const SearchRequest* request, StatsManager::ResponseStatsCollector&& collector) {
    return client.getProcessedDocuments(*request);
    // todo: error handling, retry policy, stats collectioning
}

SearcherClient CoordinatorImpl::CreateSearcherClient(const std::string& addr) const {
    std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(addr, grpc::InsecureChannelCredentials());
    return SearcherClient(channel);
}

void CoordinatorImpl::UpdateStatsIfNeeded(const std::string& index_id, const std::string& shard_id, const StatsManager& stats) {
    if (stats.GetResponsesCount() % kRequestsToNextUpdate == 0) {
        etcd_client.UpdateAverageResponseTime(index_id, shard_id, stats.GetAverageResponseTime());
    }
}


Coordinator::Coordinator(const json& config)
    : BaseServer(config), 
    service(CoordinatorImpl(config["etcd_address"].template get_ref<const std::string&>())) {}

void Coordinator::Run() {
    InternalRun(service); 
}
