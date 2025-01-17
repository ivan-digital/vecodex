#pragma once

#include <grpc/grpc.h>
#include <grpcpp/server_context.h>

#include "service.pb.h"
#include "service.grpc.pb.h"
#include "base.h"
#include "searcher.h"

using service::SearchRequest;
using service::SearchResponse;
using service::BaseService;


class CoordinatorImpl final : public BaseService::Service {
public:
    explicit CoordinatorImpl(const std::vector<std::string>& searcher_hosts);
    ~CoordinatorImpl() {}
    grpc::Status ProcessSearchRequest(grpc::ServerContext* context, const SearchRequest* request, SearchResponse* response) override;

private:
    SearchResponse AskSingleSearcher(size_t id, const SearchRequest* request) const;

    std::vector<std::string> searcher_hosts;

    std::vector<SearcherClient> searcher_clients;
};


class Coordinator final : public BaseServer {
public:
    Coordinator(
        const std::string& host,
        const std::string& port,
        const std::vector<std::string>& searcher_hosts
    );
    void Run() override;
private:
    CoordinatorImpl service;
};


class CoordinatorClient final : public BaseClient {
    using BaseClient::BaseClient;
};
