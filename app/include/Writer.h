#pragma once

#include "Base.h"

#include "service.pb.h"
#include "service.grpc.pb.h"

#include "StorageClient.h"

#include <vector>
#include <string>
#include <unordered_map>

#include "IIndex.h"
#include "ISegment.h"
#include "IndexFactory.h"

using service::BaseService;
using service::WriteRequest;
using service::WriteResponse;
using service::UpdateRequest;
using service::UpdateResponse;

class WriterImpl final : public BaseService::Service {
public:
    WriterImpl(const std::string& host, const std::string& port, const std::string& etcd_addr, const std::string& s3_host, const json& indexes_config);

    ~WriterImpl();

    grpc::Status ProcessWriteRequest(grpc::ServerContext* context, const WriteRequest* request, WriteResponse* response) override;

private:
    void indexUpdateCallback(std::vector<size_t>&& ids, std::vector<std::shared_ptr<vecodex::ISegment<std::string>>>&& segs, const std::string& index_id);

    std::string host;
    std::string port;

    std::unordered_map<std::string, std::shared_ptr<vecodex::IIndex<std::string>>> indexes;

    EtcdClient etcd_client;
    StorageClient storage_client;
};

class Writer final : public BaseServer {
    using BaseServer::BaseServer;
public:
    Writer(const json& config);

    void Run() override;

private:
    WriterImpl service;
};