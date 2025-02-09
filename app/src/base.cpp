#include "base.h"

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>


BaseServer::BaseServer(const std::string& host, const std::string& port, const std::string& etcd_addr)
    : host(host), port(port) {}

void BaseServer::InternalRun(BaseService::Service& service) {
    grpc::ServerBuilder builder;

    // std::string fullAddress = host + ":" + port; 
    std::string fullAddress = "[::]:" + port; // host + port?

    builder.AddListeningPort(fullAddress, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << port << std::endl;
    server->Wait();
}


BaseClient::BaseClient(const std::shared_ptr<grpc::ChannelInterface> channel) : 
    stub_(BaseService::NewStub(channel)) {}

SearchResponse BaseClient::getProcessedDocuments(const SearchRequest& request) const {
    SearchResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->ProcessSearchRequest(&context, request, &response);
    std::cout << "Hello from Base Client!" << std::endl;
    if (!status.ok()) {
        throw std::runtime_error("Error while processing request. gRPC status code: " + std::to_string(status.error_code()));
    }
    return response;
}