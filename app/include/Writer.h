#pragma once

#include "Base.h"
#include "StorageClient.h"

#include "service.pb.h"
#include "service.grpc.pb.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <thread>

#include <boost/asio.hpp>

#include "IIndex.h"
#include "ISegment.h"
#include "IndexFactory.h"

using service::BaseService;
using service::WriteRequest;
using service::WriteResponse;
using service::UpdateRequest;
using service::UpdateResponse;

class WriterImpl final : public BaseService::Service {
    using VecodexIndex = std::shared_ptr<vecodex::IIndex<std::string>>;
    using VecodexSegment = std::shared_ptr<vecodex::ISegment<std::string>>;
public:
    WriterImpl(const std::string& host, const std::string& port, const std::string& etcd_addr, const std::string& s3_host, const json& indexes_config);

    ~WriterImpl() override;

    grpc::Status ProcessWriteRequest(grpc::ServerContext* context, const WriteRequest* request, WriteResponse* response) override;

private:
    void indexUpdateCallback(std::vector<size_t>&& ids, std::vector<VecodexSegment>&& segs, const std::string& index_id, const std::string& shard_id);

    void updateIndexesState();

    std::string host;
    std::string port;

    std::unordered_map<std::string, json> indexes_create_agrs;
    std::unordered_map<std::string, std::unordered_map<std::string, VecodexIndex>> shards;

    EtcdClient etcd_client;
    StorageClient storage_client;

    boost::asio::thread_pool callback_runner;
    std::mutex runner_lock;
};

class Writer final : public BaseServer {
    using BaseServer::BaseServer;
public:
    Writer(const json& config);

    void Run() override;

private:
    WriterImpl service;
};