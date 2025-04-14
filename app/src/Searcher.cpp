#include "Searcher.h"
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

    auto new_document = response->add_ids();
    *new_document = "message";

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


Searcher::Searcher(const json& config) 
    : BaseServer(config), 
    service(SearcherImpl(host, port, config["etcd_address"].template get_ref<const std::string&>())) {}

void Searcher::Run() {
    InternalRun(service);
}
