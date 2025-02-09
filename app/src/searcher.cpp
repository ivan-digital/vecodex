#include "searcher.h"
#include <iostream>

SearcherImpl::SearcherImpl(const std::string& host, const std::string& port, const std::string& etcd_addr) 
    : host(host), port(port), etcd_client(EtcdClient(etcd_addr)) {
    Init();
}

SearcherImpl::~SearcherImpl() {
    GracefulShutdown();
}

grpc::Status SearcherImpl::ProcessSearchRequest(grpc::ServerContext* context, const SearchRequest* request, SearchResponse* response) {
    std::cout << "Hello from Searcher Server!" << std::endl;
    
    std::vector<float> data = {42., -42.};
    service::Document doc;
    doc.mutable_vector_data()->Assign(data.begin(), data.end());

    auto new_document = response->add_result();
    *new_document = doc;

    return grpc::Status::OK;
}

void SearcherImpl::Init() {
    etcd_client.AddSearcherHost(host + ":" + port);
    etcd_client.AddSearcherHostByIndexId("0", host + ":" + port);
}

void SearcherImpl::GracefulShutdown() {
    etcd_client.RemoveSearcherHost(host + ":" + port);
    etcd_client.RemoveSearcherHostByIndexId("0", host + ":" + port);
}


Searcher::Searcher(const std::string& host, const std::string& port, const std::string& etcd_addr) 
    : BaseServer(host, port, etcd_addr), service(SearcherImpl(host, port, etcd_addr)) {}

void Searcher::Run() {
    InternalRun(service);
}
