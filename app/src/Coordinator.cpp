#include <iostream>
#include <string>

#include "Coordinator.h"
#include "Merging.h"
#include "SearcherClient.h"
#include "service.pb.h"
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <vector>

    
CoordinatorImpl::CoordinatorImpl(const std::string& etcd_addr) 
    : etcd_client(EtcdClient(etcd_addr)) {
    UpdateSearchersState();
}

grpc::Status CoordinatorImpl::ProcessSearchRequest(grpc::ServerContext* context, const SearchRequest* request, SearchResponse* response) {
    std::cout << "Hello from Coordinator Server!" << std::endl;
    
    UpdateSearchersState();

    std::vector<SearchResponse> search_responses;
    search_responses.reserve(searcher_clients.size());
    size_t k = request->k();
    for (auto& client: searcher_clients) {
        search_responses.emplace_back(std::move(AskSingleSearcher(client.second, request)));
        std::cout << "searcher #" + client.first + " returned:\n" << search_responses.back().DebugString() << "\n";
    }
    *response = MergeSearcherAnswers(search_responses, k);

    return grpc::Status::OK;
}

void CoordinatorImpl::UpdateSearchersState() {
    searcher_clients.clear();
    auto searcher_hosts = etcd_client.ListSearcherHostsByIndexId("0");
    for (const auto& host : searcher_hosts) {
        std::cout << "Updated host " + host + "\n";
        std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(host, grpc::InsecureChannelCredentials());
        searcher_clients.emplace(host, SearcherClient(channel));
    }
}

SearchResponse CoordinatorImpl::AskSingleSearcher(const SearcherClient& client, const SearchRequest* request) const {
    return client.getProcessedDocuments(*request);
}


Coordinator::Coordinator(const json& config)
    : BaseServer(config), 
    service(CoordinatorImpl(config["etcd_address"].template get_ref<const std::string&>())) {}

void Coordinator::Run() {
    InternalRun(service); 
}
