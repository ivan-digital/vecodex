#include "Writer.h"
#include "SearcherClient.h"

#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include <optional>
#include <functional>

WriterImpl::WriterImpl(const std::string& host, const std::string& port, const std::string& etcd_addr, const std::string& s3_host, const json& indexes_config)
    : host(host), port(port), etcd_client(etcd_addr), storage_client(s3_host) {
    storage_client.logIn("user", "password"); // todo

    for (const auto& item : indexes_config) {
      	std::string id = item["id"].get<std::string>();
        indexes[id] = std::make_shared<IndexHNSWType>(
            2, 3,
            std::bind(&WriterImpl::indexUpdateCallback, this, std::placeholders::_1, std::placeholders::_2, id),
            2, 2, faiss::MetricType::METRIC_L2
        );

        bool ok = storage_client.createBucket(id);
        if (!ok) {
        	// todo
        }
    }
}

grpc::Status WriterImpl::ProcessWriteRequest(grpc::ServerContext* context, const WriteRequest* request, WriteResponse* response) {

    std::vector<float> vec(request->data().vector_data().begin(), request->data().vector_data().end());
    std::unordered_map<std::string, std::string> attributes(request->data().attributes().begin(), request->data().attributes().end());

    // ----- Testing -----
    std::cout << "Document received\nid: " << request->data().id() << std::endl;
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
    	indexes[index_id]->add(1, &index_id, vec.data());
    }
    else {
    	indexes[index_id]->erase(1, &index_id);
    }
    return grpc::Status::OK;
}

WriterImpl::~WriterImpl() {
}

// todo: slow operation, consider using async
void WriterImpl::indexUpdateCallback(std::vector<size_t>&& ids, std::vector<std::shared_ptr<const SegmentHNSWType>>&& segs, const std::string& index_id) {
    std::vector<size_t> added;
    std::vector<size_t> deleted;
    added.reserve(ids.size());
    deleted.reserve(segs.size());
    for (const auto& added_seg_ptr : segs) {
      	size_t id = added_seg_ptr->getID();
      	std::string added_filename = std::to_string(id);
        added.push_back(id);
        added_seg_ptr->serialize(added_filename);
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

    auto hosts = etcd_client.ListSearcherHostsByIndexId(index_id);
    UpdateRequest request;
    request.mutable_added()->Assign(added.begin(), added.end());
    request.mutable_deleted()->Assign(deleted.begin(), deleted.end());
    for (auto& host : hosts) {
        std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(host, grpc::InsecureChannelCredentials());
        SearcherClient client(channel);
        UpdateResponse response = client.updateIndex(request);
    }
}

Writer::Writer(const json& config) :
    BaseServer(config), service(WriterImpl(host, port, config["etcd_address"].template get_ref<const std::string&>(),
                                                       config["s3-host"].template get_ref<const std::string&>(),
                                                       config["indexes"])) {}

void Writer::Run() {
    InternalRun(service);
}