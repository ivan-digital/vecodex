#include "Writer.h"
#include "VecodexIndex.h"
#include "IndexConfig.h"

Writer::Writer(const std::string& host, const std::string& port, const std::string& s3_host, size_t threshold, const std::string& config_filename)
: BaseServer(host, port), s3_host(s3_host) {
    IndexConfig index_config(config_filename);
    index.emplace(threshold, index_config);
    storage_client.emplace(s3_host);
}

Writer::~Writer() {
    storage_client.reset();
    index.reset();
}

void Writer::Run() {
    storage_client->logIn("", ""); // todo: credentials?
    // todo
}

void Writer::receiveUpdate(const std::string& id, const std::vector<float>& vector, const std::unordered_map <std::string, std::string>& attributes) {
    index->addVector(id, vector, pushUpdate); // todo: callbacks in index
}

void Writer::pushUpdate(const VecodexSegment& segment) {
    // todo: push segment and notify
}



