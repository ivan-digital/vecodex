#include <iostream>
#include <string>

#include "coordinator.h"
#include "searcher.h"
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

    
CoordinatorImpl::CoordinatorImpl(const std::string& etcd_addr) 
    : etcd_client(EtcdClient(etcd_addr)) {
    UpdateSearchersState();
}

grpc::Status CoordinatorImpl::ProcessSearchRequest(grpc::ServerContext* context, const SearchRequest* request, SearchResponse* response) {
    std::cout << "Hello from Coordinator Server!" << std::endl;
    
    UpdateSearchersState();
    for (auto& client: searcher_clients) { 
        std::cout << "searcher " + client.first + " returned:\n" 
            << AskSingleSearcher(client.second, request).DebugString() 
            << std::endl;
    }

    auto new_document = response->add_result();
    *new_document = request->data();
    return grpc::Status::OK;
}

void CoordinatorImpl::UpdateSearchersState() {
    searcher_clients.clear();
    auto searcher_hosts = etcd_client.ListSearcherHostsByIndexId("0");
    for (const auto& host : searcher_hosts) {
        std::cout << "updated host " + host + "\n";
        std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(host, grpc::InsecureChannelCredentials());
        searcher_clients.emplace(host, SearcherClient(channel));
    }
}

SearchResponse CoordinatorImpl::AskSingleSearcher(const SearcherClient& client, const SearchRequest* request) const {
    return client.getProcessedDocuments(*request);
}


Coordinator::Coordinator(const std::string& host, const std::string& port, const std::string& etcd_addr) 
    : BaseServer(host, port, etcd_addr), service(CoordinatorImpl(etcd_addr)) {}

void Coordinator::Run() {
    InternalRun(service); 
}
