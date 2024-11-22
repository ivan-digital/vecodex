#include "coordinator.h"
#include <iostream>
#include <string>

grpc::Status CoordinatorImpl::ProcessSearchRequest(grpc::ServerContext* context, const SearchRequest* request, SearchResponse* response) {
    std::cout << "Hello from Coordinator Server!" << std::endl;

    auto new_document = response->add_result();
    *new_document = request->data();
    return grpc::Status::OK;
}


Coordinator::Coordinator(const std::string& host, const std::string& port) : BaseServer(host, port) {}

void Coordinator::Run() {
    CoordinatorImpl service = CoordinatorImpl();
    InternalRun(service); 
}


CoordinatorClient::CoordinatorClient(const std::shared_ptr<grpc::ChannelInterface> channel) : 
    stub_(BaseService::NewStub(channel)) {}

SearchResponse CoordinatorClient::getProcessedDocuments(const SearchRequest& request) {
    SearchResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->ProcessSearchRequest(&context, request, &response);
    std::cout << "Hello from Coordinator Client!" << std::endl;
    if (!status.ok()) {
        throw std::runtime_error("Error while processing request. gRPC status code: " + std::to_string(status.error_code()));
    }
    return response;
}