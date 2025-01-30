#include "WriterClient.h"

WriterClient::WriterClient(const std::shared_ptr<grpc::ChannelInterface>& channel) : stub_(BaseService::NewStub(channel)) {}

WriteResponse WriterClient::writeDocument(const WriteRequest& request) {
    WriteResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->ProcessWriteRequest(&context, request, &response);
    std::cout << "Hello from Writer Client!" << std::endl;
    if (!status.ok()) {
        throw std::runtime_error("Error while processing request. gRPC status code: " + std::to_string(status.error_code()));
    }
    return response;
}