#include <grpc/grpc.h>
#include <grpcpp/server_context.h>

#include "service.pb.h"
#include "service.grpc.pb.h"
#include "base.h"

using service::SearchRequest;
using service::SearchResponse;
using service::BaseService;


class CoordinatorImpl final : public BaseService::Service {
public:
    explicit CoordinatorImpl() {}
    ~CoordinatorImpl() {}
    grpc::Status ProcessSearchRequest(grpc::ServerContext* context, const SearchRequest* request, SearchResponse* response) override;
};


class Coordinator : public BaseServer {
public:
    Coordinator(const std::string& host, const std::string& port);
    void Run();
};


class CoordinatorClient {
public:
    CoordinatorClient(const std::shared_ptr<grpc::ChannelInterface> channel);

    SearchResponse getProcessedDocuments(const SearchRequest& request);
private:
    std::unique_ptr<BaseService::Stub> stub_;
};