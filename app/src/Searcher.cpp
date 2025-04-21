#include "Searcher.h"

#include <iostream>
#include <string>
#include <memory>
#include <cstdio>

#include "IndexFactory.h"

SearcherImpl::SearcherImpl(const std::string& host, const std::string& port, const std::string& etcd_addr, const std::string& s3_host, const json& shards_configs)
    : host(host), port(port), etcd_client(EtcdClient(etcd_addr)), storage_client(s3_host) {
    for (const auto& item : shards_configs) {
        std::string index_id = item["index_id"].get<std::string>();
        std::string shard_id = item["shard_id"].get<std::string>();
        shards[index_id][shard_id] = vecodex::CreateIndex<std::string>(item);
        std::cout << "Created shard [" << index_id << ", " << shard_id << "]" << std::endl;
    }
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

    std::string index_id = request->index_id();

    if (request->has_shard_id()) {
        std::string shard_id = request->shard_id();
        std::vector<std::string> result = shards[index_id][shard_id]->search(query, k);
        response->mutable_ids()->Assign(result.begin(), result.end());
        return grpc::Status::OK;
    }

    std::vector<std::string> result;
    for (auto& shard : shards[index_id]) {
        std::vector<std::string> shard_result = shard.second->search(query, k);
        result.insert(result.end(), shard_result.begin(), shard_result.end());
    }
    response->mutable_ids()->Assign(result.begin(), result.end());
    return grpc::Status::OK;
}

grpc::Status SearcherImpl::ProcessUpdateRequest(grpc::ServerContext* context, const UpdateRequest* request, UpdateResponse* response) {
    std::vector<size_t> added(request->added().begin(), request->added().end());
    std::vector<size_t> deleted(request->deleted().begin(), request->deleted().end());

    std::string index_id = request->index_id();
    std::string shard_id = request->shard_id();

    std::cout << "Received update\n";
    std::cout << "Shard [" << index_id << ", " << shard_id << "]" << std::endl;
    std::cout << "added:\n";
    for (auto id : added) {
        std::cout << id << " ";
    }
    std::cout << std::endl;
    std::cout << "deleted:\n";
    for (auto id : deleted) {
        std::cout << id << " ";
    }
    std::cout << std::endl;

    for (size_t added_id : added) {
        std::string segment_filename = std::to_string(added_id);
        bool ok = storage_client.getObject(index_id, segment_filename);
        if (!ok) {
            // todo
        }

        FILE* fd = std::fopen(segment_filename.c_str(), "r");
        auto segment = vecodex::DeserealizeSegment<std::string>(fd);
        shards[index_id][shard_id]->pushSegment(segment);

        std::fclose(fd);
        std::remove(segment_filename.c_str());
    }

    for (size_t deleted_id : deleted) {
        shards[index_id][shard_id]->eraseSegment(deleted_id);
    }
    return grpc::Status::OK;
}

void SearcherImpl::Init() {
	for (auto& p1 : shards) {
        for (auto& p2 : p1.second) {
            std::string index_id = p1.first;
            std::string shard_id = p2.first;
            etcd_client.AddSearcherHost(index_id, shard_id, host + ":" + port);
        }
	}
}

void SearcherImpl::GracefulShutdown() {
	for (auto& p1 : shards) {
        for (auto& p2 : p1.second) {
            std::string index_id = p1.first;
            std::string shard_id = p2.first;
            etcd_client.RemoveSearcherHost(index_id, shard_id, host + ":" + port);
        }
	}
}

Searcher::Searcher(const json& config)
    : BaseServer(config),
      service(SearcherImpl(host, port,
          config["etcd_address"].template get_ref<const std::string&>(),
          config["s3-host"].template get_ref<const std::string&>(),
          config["indexes"])) {}

void Searcher::Run() {
    InternalRun(service);
}
