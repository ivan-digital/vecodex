#pragma once

#include "base.h"
#include "service.pb.h"
#include "service.grpc.pb.h"
#include "VecodexIndex.h"
#include "StorageClient.h"
#include "ThreadPool.h"

#include <vector>
#include <string>
#include <unordered_map>

using service::BaseService;
using service::WriteRequest;
using service::WriteResponse;

class WriterImpl final : public BaseService::Service {
public:
    WriterImpl(const std::string& s3_host, size_t threshold, const std::string& config_filename);

    ~WriterImpl();

    grpc::Status ProcessWriteRequest(grpc::ServerContext* context, const WriteRequest* request, WriteResponse* response) override;

private:
    void updateIndex(int id, const std::vector<float>& vector, const std::unordered_map<std::string, std::string>& attributes);

    void pushUpdate();

    std::string s3_host;
    std::optional<VecodexIndex> index;
    std::optional<StorageClient> storage_client;
};

class Writer : public BaseServer {
public:
    Writer(const std::string& host, const std::string& port, const std::string& s3_host, size_t threshold, const std::string& config_filename);

    void Run() override;

private:
    WriterImpl service;
};

