#include "Writer.h"
#include "VecodexIndex.h"
#include "IndexConfig.h"

WriterImpl::WriterImpl(const std::string& s3_host, size_t threshold, const std::string& config_filename) : s3_host(s3_host) {
    IndexConfig index_config(config_filename);
    index.emplace(threshold, index_config);
    storage_client.emplace(s3_host);

    storage_client->logIn("", ""); // todo: credentials?
}

grpc::Status WriterImpl::ProcessWriteRequest(grpc::ServerContext* context, const WriteRequest* request, WriteResponse* response) {

    std::vector<float> vector(request->data().vector_data().begin(), request->data().vector_data().end());
    std::unordered_map<std::string, std::string> attributes(request->data().attributes().begin(), request->data().attributes().end());

    // ----- Testing -----
    std::cout << "Document received\nid: " << request->data().id() << std::endl;
    for (float i : vector) {
        std::cout << i << ", ";
    }
    std::cout << std::endl;
    for (auto& pair : attributes) {
        std::cout << "{" << pair.first << ": " << pair.second << "}" << std::endl;
    }
    // -------------------

    updateIndex(request->data().id(), vector, attributes);

    response->set_id(request->data().id());
    return grpc::Status::OK;
}

WriterImpl::~WriterImpl() {
    storage_client.reset();
    index.reset();
}

void WriterImpl::updateIndex(int id, const std::vector<float>& vector, const std::unordered_map <std::string, std::string>& attributes) {
    // todo: add vector into index with callback
}

void WriterImpl::pushUpdate() {
    // todo: push segment and notify
}

Writer::Writer(const std::string& host, const std::string& port, const std::string& s3_host, size_t threshold, const std::string& config_filename)
: BaseServer(host, port), service(WriterImpl(s3_host, threshold, config_filename)) {}

void Writer::Run() {
    InternalRun(service);
}
