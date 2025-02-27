#pragma once

#include <grpc/grpc.h>
#include <grpcpp/server_context.h>
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
    SearchResponse AskSingleSearcher(const SearcherClient& client, const SearchRequest* request) const;

    void UpdateSearchersState();

    std::unordered_map<std::string, SearcherClient> searcher_clients;
    EtcdClient etcd_client;
};


class Coordinator final : public BaseServer {
public:
    Coordinator(const json& config);
    void Run() override;
private:
    CoordinatorImpl service;
};