#pragma once

#include <grpc/grpc.h>
#include <grpcpp/server_context.h>
#include <string>
#include <unordered_map>
#include <optional>
#include <mutex>

#include "service.pb.h"
#include "service.grpc.pb.h"
#include "SearcherClient.h"
#include "EtcdClient.h"
#include "Base.h"

#include "utils/Stats.h"

using service::SearchRequest;
using service::SearchResponse;
using service::BaseService;


class CoordinatorImpl final : public BaseService::Service {
public:
    explicit CoordinatorImpl(const std::string& etcd_addr);
    ~CoordinatorImpl() {}
    grpc::Status ProcessSearchRequest(grpc::ServerContext*, const SearchRequest*, SearchResponse*) override;

private:
    using ShardToSearcher = std::unordered_map<std::string, std::vector<SearcherClient>>;
    using ShardToStats = std::unordered_map<std::string, StatsManager>;

    std::optional<SearchResponse> AskSingleSearcher(
        const SearcherClient&, const SearchRequest*, StatsManager::ResponseStatsCollector&&);

    void UpdateSearchersState(const SearchRequest*);

private:
    std::unordered_map<std::string, ShardToSearcher> searchers_map; // index_id -> shard_id -> searcher_client 
    std::unordered_map<std::string, ShardToStats> searchers_stats; // index_id -> shard_id -> stats_manager 
    EtcdClient etcd_client;
    std::mutex mutex_;
private:
    SearcherClient CreateSearcherClient(const std::string& addr) const;

private:
    void UpdateStateIfNeeded(const std::string& index_id, const std::string& shard_id, const SearcherClient& client, const StatsManager& stats);

    static constexpr size_t kRequestsToNextUpdate = 1;

    static constexpr size_t kHostErrorsLimit = 1;


};


class Coordinator final : public BaseServer {
public:
    Coordinator(const json& config);
    void Run() override;
private:
    CoordinatorImpl service;
};