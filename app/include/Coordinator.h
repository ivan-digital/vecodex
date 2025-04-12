#pragma once

#include <grpc/grpc.h>
#include <grpcpp/server_context.h>
#include <string>
#include <unordered_map>

#include "service.pb.h"
#include "service.grpc.pb.h"
#include "SearcherClient.h"
#include "EtcdClient.h"
#include "Base.h"

using service::SearchRequest;
using service::SearchResponse;
using service::BaseService;


class CoordinatorImpl final : public BaseService::Service {
public:
    explicit CoordinatorImpl(const std::string& etcd_addr);
    ~CoordinatorImpl() {}
    grpc::Status ProcessSearchRequest(grpc::ServerContext* context, const SearchRequest* request, SearchResponse* response) override;

private:
    using ShardToSearcher = std::unordered_map<std::string, std::vector<SearcherClient>>;

    SearchResponse AskSingleSearcher(const SearcherClient& client, const SearchRequest* request) const;

    void UpdateSearchersState(const SearchRequest* request);

private:
    std::unordered_map<std::string, ShardToSearcher> searchers_map; // index_id -> shard_id -> searcher_client 
    EtcdClient etcd_client;

private:
    SearcherClient CreateSearcherClient(const std::string& addr) const;
};


class Coordinator final : public BaseServer {
public:
    Coordinator(const json& config);
    void Run() override;
private:
    CoordinatorImpl service;
};