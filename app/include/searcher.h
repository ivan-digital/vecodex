#pragma once

#include <grpc/grpc.h>
#include <grpcpp/server_context.h>

#include "service.pb.h"
#include "service.grpc.pb.h"
#include "base.h"

using service::SearchRequest;
using service::SearchResponse;
using service::BaseService;

class SearcherImpl final : public BaseService::Service {
public:
    SearcherImpl(const std::string& host, const std::string& port, const std::string& etcd_addr /* and smth related to index */);
    ~SearcherImpl();
    grpc::Status ProcessSearchRequest(grpc::ServerContext* context, const SearchRequest* request, SearchResponse* response) override;
private:
    void Init();

    void GracefulShutdown();

    std::string host;
    std::string port;
    EtcdClient etcd_client;
    /* smth related to index */
};

class Searcher final : public BaseServer {
    using BaseServer::BaseServer;
public:
    Searcher(const std::string& host, const std::string& port, const std::string& etcd_addr);
    void Run() override;
private:
    SearcherImpl service;
};

class SearcherClient final : public BaseClient {
    using BaseClient::BaseClient;
};