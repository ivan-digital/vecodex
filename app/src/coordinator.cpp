#include <iostream>
#include <string>

#include "coordinator.h"
#include "searcher.h"
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

    
CoordinatorImpl::CoordinatorImpl(const std::vector<std::string>& searcher_hosts) 
    : searcher_hosts(std::move(searcher_hosts)) {
    for (size_t i = 0; i < searcher_hosts.size(); ++i) {
        searcher_clients.push_back(
            SearcherClient(grpc::CreateChannel(searcher_hosts[i], grpc::InsecureChannelCredentials()))
        );
    }
}

grpc::Status CoordinatorImpl::ProcessSearchRequest(grpc::ServerContext* context, const SearchRequest* request, SearchResponse* response) {
    std::cout << "Hello from Coordinator Server!" << std::endl;
    
    for (size_t i = 0; i < searcher_hosts.size(); ++i) { 
        std::cout << "searcher " << i << " returned:\n" << AskSingleSearcher(i, request).DebugString() << std::endl;
    }

    auto new_document = response->add_result();
    *new_document = request->data();
    return grpc::Status::OK;
}

SearchResponse CoordinatorImpl::AskSingleSearcher(size_t searcher_id, const SearchRequest* request) const {
    return searcher_clients[searcher_id].getProcessedDocuments(*request);
}


Coordinator::Coordinator(const std::string& host, const std::string& port, const std::vector<std::string>& searcher_hosts) 
    : BaseServer(host, port), service(CoordinatorImpl(searcher_hosts)) {}

void Coordinator::Run() {
    InternalRun(service); 
}
