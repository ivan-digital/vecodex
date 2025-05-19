#include <exception>
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
    etcd_client.DebugPrintEverything(); // debug
}

grpc::Status CoordinatorImpl::ProcessSearchRequest(grpc::ServerContext* context, const SearchRequest* request, SearchResponse* response) {
    std::cout << "Hello from Coordinator Server!" << std::endl;

    prom_exposer.IncrementCounter("requests_count", 1);

    std::lock_guard guard(mutex_);

    UpdateSearchersState(request);

    auto index_id = request->index_id();
    std::vector<SearchResponse> search_responses;
    size_t k = request->k();

    for (auto& [shard_id, searchers] : searchers_map[index_id]) {
        for (auto& client : searchers) {
            std::cout << "Request to host with:\nindex_id: " << index_id << "\nshard_id: " 
                << shard_id << "\naddress: " << client.GetAddress() << std::endl; // debug
            auto stats_collector = searchers_stats[index_id][shard_id].CreateResponseData();
            auto response = AskSingleSearcher(client, request, std::move(stats_collector));
            UpdateStateIfNeeded(index_id, shard_id, client, searchers_stats[index_id][shard_id]);
            if (response.has_value()) {
                search_responses.emplace_back(std::move(response.value()));
            }
        }
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

std::optional<SearchResponse> CoordinatorImpl::AskSingleSearcher(const SearcherClient& client, const SearchRequest* request, StatsManager::ResponseStatsCollector&& collector) {
    try {
        auto tmp = client.getProcessedDocuments(*request);
        std::cout << "asking result: " << tmp.DebugString() << std::endl; // debug
        return tmp;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        collector.SetError();
        return std::nullopt;
    }
    // todo: error handling, retry policy, stats collectioning
}

SearcherClient CoordinatorImpl::CreateSearcherClient(const std::string& addr) const {
    std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(addr, grpc::InsecureChannelCredentials());
    return SearcherClient(channel, addr);
}

void CoordinatorImpl::UpdateStateIfNeeded(const std::string& index_id, const std::string& shard_id, const SearcherClient& client, const StatsManager& stats) {
    if (stats.GetErrorsCount() >= kHostErrorsLimit) {
        try {
            etcd_client.RemoveSearcherHost(index_id, shard_id, client.GetAddress());
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
        return;
    }
    if (stats.GetResponsesCount() % kRequestsToNextUpdate == 0) {
        std::map<std::string, std::string> labels {{"index_id", index_id}, {"shard_id", shard_id}};
        prom_exposer.SetGauge("requests_shard_count", stats.GetResponsesCount(), labels);
        prom_exposer.SetGauge("errors_count", stats.GetErrorsCount(), labels);
        prom_exposer.SetGauge("request_latency_avg", stats.GetAverageResponseTime(), labels);

        etcd_client.UpdateAverageResponseTime(index_id, shard_id, stats.GetAverageResponseTime());
    }
}


Coordinator::Coordinator(const json& config)
    : BaseServer(config), 
    service(CoordinatorImpl(config["etcd_address"].template get_ref<const std::string&>())) {}

void Coordinator::Run() {
    InternalRun(service); 
}
