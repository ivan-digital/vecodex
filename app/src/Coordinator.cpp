#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include "Coordinator.h"
#include "Merging.h"
#include "SearcherClient.h"
#include "service.pb.h"
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <vector>

    
CoordinatorImpl::CoordinatorImpl(const std::string& etcd_addr) 
    : etcd_client(EtcdClient(etcd_addr)) {
    UpdateSearchersState(nullptr);
}

grpc::Status CoordinatorImpl::ProcessSearchRequest(grpc::ServerContext* context, const SearchRequest* request, SearchResponse* response) {
    std::cout << "Hello from Coordinator Server!" << std::endl;
    
    UpdateSearchersState(request);

    auto index_id = request->index_id();
    for (auto& [shard_id, searchers] : searchers_map[index_id]) {
        for (auto& client : searchers) {
            AskSingleSearcher(client, request);
        }
    }

    std::vector<SearchResponse> search_responses;
    search_responses.reserve(searcher_clients.size());
    size_t k = request->k();
    for (auto& client: searcher_clients) {
        search_responses.emplace_back(std::move(AskSingleSearcher(client.second, request)));
        std::cout << "searcher #" + client.first + " returned:\n" << search_responses.back().DebugString() << "\n";
    }
    *response = MergeSearcherAnswers(search_responses, k);

    return grpc::Status::OK;
}

void CoordinatorImpl::UpdateSearchersState(const SearchRequest* request) {
    const auto& index_id = request->index_id();
    std::optional<std::string> shard_id = std::nullopt;
    
    auto update_with_index_shard_info = [this](const std::string& index_id, const std::string& shard_id) {
        auto hosts = etcd_client.ListSearcherHosts(index_id, shard_id);
        searchers_map[index_id][shard_id] = {};
        for (const auto& addr : hosts) {
            searchers_map[index_id][shard_id].emplace_back(CreateSearcherClient(addr));
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

SearchResponse CoordinatorImpl::AskSingleSearcher(const SearcherClient& client, const SearchRequest* request) const {
    return client.getProcessedDocuments(*request);
}

SearcherClient CoordinatorImpl::CreateSearcherClient(const std::string& addr) const {
    std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(addr, grpc::InsecureChannelCredentials());
    return SearcherClient(channel);
}


Coordinator::Coordinator(const json& config)
    : BaseServer(config), 
    service(CoordinatorImpl(config["etcd_address"].template get_ref<const std::string&>())) {}

void Coordinator::Run() {
    InternalRun(service); 
}
