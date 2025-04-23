#include "CoordinatorClient.h"
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
        grpc::CreateChannel("[::]:" + port, grpc::InsecureChannelCredentials()),
        "[::]:44400"
    );

    std::vector<float> data{0.1, 0.1};

    service::SearchRequest req;
    req.mutable_vector_data()->Assign(data.begin(), data.end());
    req.set_k(3);
    req.set_index_id("0");
    req.set_shard_id("0");

    auto resp = client.getProcessedDocuments(req);
    std::cout << resp.DebugString() << std::endl;
}