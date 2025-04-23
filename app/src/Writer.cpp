#include "Writer.h"
#include "SearcherClient.h"

#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include <iostream>

WriterImpl::WriterImpl(const std::string& host, const std::string& port, const std::string& etcd_addr, const std::string& s3_host, const json& indexes_config)
    : host(host), port(port), etcd_client(etcd_addr), storage_client(s3_host), callback_runner(1) {
    storage_client.logIn("user", "password"); // todo

    for (const auto& item : indexes_config) {
        std::string id = item["id"].get<std::string>();
        indexes[id] = vecodex::CreateIndex<std::string>(item);

        auto callback = [this, id](auto&& PH1, auto&& PH2) {
            indexUpdateCallback(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), id);
        };
        indexes[id]->setUpdateCallback(callback);

        std::cout << "Created index " << id << std::endl;
        bool ok = storage_client.createBucket(id);
        if (!ok) {
            // todo
        }
    }
}

grpc::Status WriterImpl::ProcessWriteRequest(grpc::ServerContext* context, const WriteRequest* request, WriteResponse* response) {
    std::lock_guard guard(runner_lock);

    std::vector<float> vec(request->data().vector_data().begin(), request->data().vector_data().end());
    std::unordered_map<std::string, std::string> attributes(request->data().attributes().begin(), request->data().attributes().end());
    std::string vec_id = request->data().id();

    // ----- Testing -----
    std::cout << "Document received\nid: " << vec_id << std::endl;
    for (float i : vec) {
        std::cout << i << ", ";
    }
    std::cout << std::endl;
    for (auto& pair : attributes) {
        std::cout << "{" << pair.first << ": " << pair.second << "}" << std::endl;
    }
    // -------------------

    if (attributes.find("index-id") == attributes.end()) {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Missing index id");
    }
    std::string index_id = attributes.at("index-id");
    if (indexes.find(index_id) == indexes.end()) {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Wrong index id");
    }

    if (attributes.find("delete") == attributes.end()) {
        indexes[index_id]->add(1, &vec_id, vec.data());
    }
    else {
        indexes[index_id]->erase(1, &vec_id);
    }
    return grpc::Status::OK;
}

WriterImpl::~WriterImpl() {
    callback_runner.join();
}

void WriterImpl::indexUpdateCallback(std::vector<size_t>&& ids, std::vector<std::shared_ptr<vecodex::ISegment<std::string>>>&& segs, const std::string& index_id) {
    auto task = [this, ids = std::move(ids), segs = std::move(segs), index_id = index_id] {
        std::lock_guard guard(runner_lock);

        std::cout << "Run callback\n";
        std::cout << "index id: " << index_id << std::endl;
        std::cout << "added:\n";
        for (const auto& added_seg_ptr : segs) {
            std::cout << added_seg_ptr->getID() << " ";
        }
        std::cout << std::endl;
        std::cout << "deleted:\n";
        for (auto id : ids) {
            std::cout << id << " ";
        }
        std::cout << std::endl;

        std::vector<size_t> added;
        std::vector<size_t> deleted;
        for (const auto& added_seg_ptr : segs) {
            size_t id = added_seg_ptr->getID();
            std::string added_filename = std::to_string(id);
            added.push_back(id);
            vecodex::SerializeSegment<std::string>(added_filename, added_seg_ptr);
            bool ok = storage_client.putObject(index_id, added_filename);
            if (!ok) {
                // todo
            }
            std::remove(added_filename.c_str());
        }

        for (auto id : ids) {
            deleted.push_back(id);
            bool ok = storage_client.delObject(index_id, std::to_string(id));
            if (!ok) {
                // todo
            }
        }

        auto hosts = etcd_client.ListSearcherHosts(index_id);
        std::cout << "searcher hosts:\n";
        for (const auto& host : hosts) {
            std::cout << host << std::endl;
        }
        UpdateRequest request;
        request.mutable_added()->Assign(added.begin(), added.end());
        request.mutable_deleted()->Assign(deleted.begin(), deleted.end());
        for (auto& host : hosts) {
            std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(host, grpc::InsecureChannelCredentials());
            SearcherClient client(channel, host);
            UpdateResponse response = client.updateIndex(request);
        }
    };

    boost::asio::post(callback_runner, task);
}

Writer::Writer(const json& config)
    : BaseServer(config),
      service(WriterImpl(host, port,
          config["etcd_address"].template get_ref<const std::string&>(),
          config["s3-host"].template get_ref<const std::string&>(),
          config["indexes"])) {}

void Writer::Run() {
    InternalRun(service);
}