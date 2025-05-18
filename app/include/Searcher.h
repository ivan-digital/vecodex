#pragma once

#include <grpc/grpc.h>
#include <grpcpp/server_context.h>

#include "service.pb.h"
#include "service.grpc.pb.h"
#include "Base.h"
#include "StorageClient.h"

#include "utils/PrometheusExposer.h"

#include "IIndex.h"
#include "ISegment.h"
#include "IndexFactory.h"

using service::SearchRequest;
using service::SearchResponse;
using service::UpdateRequest;
using service::UpdateResponse;
using service::BaseService;

class SearcherImpl final : public BaseService::Service {
    using VecodexIndex = std::shared_ptr<vecodex::IIndex<std::string>>;
    using VecodexSegment = std::shared_ptr<vecodex::ISegment<std::string>>;
public:
    SearcherImpl(const std::string& host, const std::string& port, const std::string& etcd_addr, const std::string& s3_host, const json& shards_configs);

    ~SearcherImpl() override;

    grpc::Status ProcessSearchRequest(grpc::ServerContext* context, const SearchRequest* request, SearchResponse* response) override;

    grpc::Status ProcessUpdateRequest(grpc::ServerContext* context, const UpdateRequest* request, UpdateResponse* response) override;
private:
    void Init();

    void GracefulShutdown();

    std::string host;
    std::string port;
    EtcdClient etcd_client;
    StorageClient storage_client;
    PrometheusExposer prom_exposer;

    // [index_id][shard_id]
    std::unordered_map<std::string, std::unordered_map<std::string, VecodexIndex>> shards;
};

class Searcher final : public BaseServer {
    using BaseServer::BaseServer;
public:
    Searcher(const json& config);
    void Run() override;
private:
    SearcherImpl service;
};

