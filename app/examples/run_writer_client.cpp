#include "WriterClient.h"

#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include <iostream>

int main(int argc, char* argv[]) {
    std::string port;

    if (argc > 1 && argv[1]) {
        port = std::string(argv[1]);
    }
    else {
        throw std::runtime_error("port not found");
    }

    WriterClient client = WriterClient(grpc::CreateChannel("0.0.0.0:" + port, grpc::InsecureChannelCredentials()));

    service::Document doc;
    doc.set_id(21);

    std::vector<float> data = {1.0, 2.1, 5.3, 4.9};
    doc.mutable_vector_data()->Assign(data.begin(), data.end());

    std::unordered_map<std::string, std::string> attributes = {{"key1", "value1"}, {"key2", "value2"}};
    for (auto it = attributes.begin(); it != attributes.end(); ++it) {
        (*doc.mutable_attributes())[it->first] = it->second;
    }

    service::WriteRequest request;
    *request.mutable_data() = doc;

    WriteResponse response = client.writeDocument(request);
    std::cout << response.id() << std::endl;

    return 0;
}