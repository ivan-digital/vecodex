#include "SearcherClient.h"

UpdateResponse SearcherClient::updateIndex(const UpdateRequest& request) const {
    UpdateResponse response;
    grpc::ClientContext context;
    grpc::Status status = stub_->ProcessUpdateRequest(&context, request, &response);
    if (!status.ok()) {
        throw std::runtime_error("Error while processing request. gRPC status code: " + std::to_string(status.error_code()));
    }
    return response;
}