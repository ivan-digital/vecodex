#pragma once

#include <grpc/grpc.h>
#include <grpcpp/server_context.h>

#include "service.pb.h"
#include "service.grpc.pb.h"
#include "Base.h"
#include "StorageClient.h"

#include "faiss.h"
#include "Index.h"
#include "Segment.h"

#include "IBaseIndex.h"
#include "IBaseSegment.h"

using SegmentHNSWType =
	vecodex::Segment<baseline::FaissIndex<faiss::IndexHNSWFlat, std::string>>;
using SegmentFLatType =
	vecodex::Segment<baseline::FaissIndex<faiss::IndexFlat, std::string>>;
using IndexHNSWType =
	vecodex::Index<baseline::FaissIndex<faiss::IndexHNSWFlat, std::string>>;
using IndexFlatType =
	vecodex::Index<baseline::FaissIndex<faiss::IndexFlat, std::string>>;

using service::SearchRequest;
using service::SearchResponse;
using service::UpdateRequest;
using service::UpdateResponse;
using service::BaseService;

class SearcherImpl final : public BaseService::Service {
public:
    SearcherImpl(const std::string& host, const std::string& port, const std::string& etcd_addr, const std::string& s3_host, const std::string& index_id);

    ~SearcherImpl();

    grpc::Status ProcessSearchRequest(grpc::ServerContext* context, const SearchRequest* request, SearchResponse* response) override;

    grpc::Status ProcessUpdateRequest(grpc::ServerContext* context, const UpdateRequest* request, UpdateResponse* response) override;
private:
    void Init();

    void GracefulShutdown();

    std::string host;
    std::string port;
    EtcdClient etcd_client;
    StorageClient storage_client;
    IndexHNSWType index;
    std::string index_id;
};

class Searcher final : public BaseServer {
    using BaseServer::BaseServer;
public:
    Searcher(const json& config);
    void Run() override;
private:
    SearcherImpl service;
};

