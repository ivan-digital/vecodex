#include "coordinator.h"
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <iostream>

int main(int argc, char* argv[]) {

    std::string port;
    try {
        port = argv[1];
    } 
    catch (const std::exception& err) {
        std::cerr << "Cannot parse first argument\nUsage: ./example_client <server-side port>";
        std::exit();
    }

    CoordinatorClient client = CoordinatorClient(
        grpc::CreateChannel("127.0.0.1:" + port, grpc::InsecureChannelCredentials())
    );

    service::Document doc;
    doc.set_id(42);

    service::SearchRequest req;
    *req.mutable_data() = doc;

    auto resp = client.getProcessedDocuments(req);
    std::cout << "Server response id: " + resp.result(0).id() << std::endl;

    return 0;
}