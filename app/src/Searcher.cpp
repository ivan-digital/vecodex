#include "Searcher.h"

#include <iostream>
#include <string>
#include <memory>
#include <cstdio>
#include <optional>

SearcherImpl::SearcherImpl(const std::string& host, const std::string& port, const std::string& etcd_addr, const std::string& s3_host, const std::string& index_id)
    : host(host), port(port), etcd_client(EtcdClient(etcd_addr)), storage_client(s3_host), index(2, 3, std::nullopt, 2, 2, faiss::MetricType::METRIC_L2), index_id(index_id) {
    Init();
    storage_client.logIn("user", "password"); // todo
}

SearcherImpl::~SearcherImpl() {
    GracefulShutdown();
}

grpc::Status SearcherImpl::ProcessSearchRequest(grpc::ServerContext* context, const SearchRequest* request, SearchResponse* response) {
    std::cout << "Hello from Searcher Server!" << std::endl;

    std::vector<float> query(request->vector_data().begin(), request->vector_data().end());
    int k = request->k();

    std::vector<std::string> result = index.search(query, k);

    response->mutable_ids()->Assign(result.begin(), result.end());
    return grpc::Status::OK;
}

grpc::Status SearcherImpl::ProcessUpdateRequest(grpc::ServerContext* context, const UpdateRequest* request, UpdateResponse* response) {
    std::vector<size_t> added(request->added().begin(), request->added().end());
    std::vector<size_t> deleted(request->deleted().begin(), request->deleted().end());
	for (size_t added_id : added) {
  		std::string segment_filename = std::to_string(added_id);
  		bool ok = storage_client.getObject(index_id, segment_filename);
  		if (!ok) {
      		// todo
  		}

        FILE* fd = std::fopen(segment_filename.c_str(), "r");
        auto segment = std::make_shared<SegmentHNSWType>(fd);
        index.pushSegment(segment);

        std::fclose(fd);
        std::remove(segment_filename.c_str());
	}

    for (size_t deleted_id : deleted) {
      std::string segment_filename = std::to_string(deleted_id);
      bool ok = storage_client.getObject(index_id, segment_filename);
    }
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
    service(SearcherImpl(host, port, config["etcd_address"].template get_ref<const std::string&>(),
                                     config["s3-host"].template get_ref<const std::string&>(),
                           			 config["indexes"][0]["id"].template get_ref<const std::string&>())) {}

void Searcher::Run() {
    InternalRun(service);
}
