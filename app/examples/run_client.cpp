#include "coordinator.h"
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <iostream>

int main(int argc, char* argv[]) {

    std::string port;
    if (argc > 1 && argv[1] != nullptr) {
        port = std::string(argv[1]);
    } else {
        throw std::runtime_error("Port not found.\nUsage: ./example_client <server-side port>\n");
    }

    CoordinatorClient client = CoordinatorClient(
        grpc::CreateChannel("0.0.0.0:" + port, grpc::InsecureChannelCredentials())
    );

    service::Document doc;
    doc.set_id(42);

    service::SearchRequest req;
    *req.mutable_data() = doc;

    auto resp = client.getProcessedDocuments(req);
    std::cout << resp.DebugString() << std::endl;
}