#include "BaseClient.h"

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>


BaseClient::BaseClient(const std::shared_ptr<grpc::ChannelInterface> channel, std::string&& addr) : 
    stub_(BaseService::NewStub(channel)), address(std::move(addr)) {}

BaseClient::BaseClient(const std::shared_ptr<grpc::ChannelInterface> channel, const std::string& addr) : 
    stub_(BaseService::NewStub(channel)), address(addr) {}

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

std::string BaseClient::GetAddress() const {
    return address;
}