#include "Writer.h"
#include "SearcherClient.h"

#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include <iostream>

WriterImpl::WriterImpl(const std::string& host, const std::string& port, const std::string& etcd_addr, const std::string& s3_host, const json& indexes_config)
    : host(host), port(port), etcd_client(etcd_addr), storage_client(s3_host), callback_runner(1) {
    std::lock_guard guard(runner_lock);
    storage_client.logIn("user", "password"); // todo

    for (const auto& item: indexes_config) {
        std::string index_id = item["id"].get<std::string>();
        indexes_create_agrs[index_id] = item;
    }

    updateIndexesState();
}

grpc::Status WriterImpl::ProcessWriteRequest(grpc::ServerContext* context, const WriteRequest* request, WriteResponse* response) {
    std::lock_guard guard(runner_lock);

    prom_exposer.IncrementCounter("requests_count", 1);

    updateIndexesState();

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
    if (shards.find(index_id) == shards.end()) {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Wrong index id");
    }

    std::string shard_id = chooseShard(index_id);

    std::map<std::string, std::string> labels {{"index_id", index_id}, {"shard_id", shard_id}};
    auto vec_cnt_opt = prom_exposer.GetGaugeValue<size_t>("vectors_count", labels);
    size_t vec_cnt = 0;
    if (vec_cnt_opt.has_value()) {
        vec_cnt = vec_cnt_opt.value();
    }

    if (attributes.find("delete") == attributes.end()) {
        shards[index_id][shard_id]->add(1, &vec_id, vec.data());
        prom_exposer.SetGauge("vectors_count", vec_cnt + 1, labels);
    }
    else {
        shards[index_id][index_id]->erase(1, &vec_id);
        prom_exposer.SetGauge("vectors_count", vec_cnt - 1, labels);
    }
    return grpc::Status::OK;
}

WriterImpl::~WriterImpl() {
    callback_runner.join();
}

void WriterImpl::indexUpdateCallback(std::vector<size_t>&& ids, std::vector<VecodexSegment>&& segs, const std::string& index_id, const std::string& shard_id) {
    auto task = [this, ids = std::move(ids), segs = std::move(segs), index_id = index_id, shard_id = shard_id] {
        std::lock_guard guard(runner_lock);

        std::map<std::string, std::string> labels {{"index_id", index_id}, {"shard_id", shard_id}};
        prom_exposer.IncrementCounter("callback_calls_count", 1, labels);

        std::cout << "Run callback" << std::endl;
        std::cout << "shard: [" << index_id << ", " << shard_id << "]" << std::endl;
        std::cout << "added:" << std::endl;
        for (const auto& added_seg_ptr : segs) {
            std::cout << added_seg_ptr->getID() << " ";
        }
        std::cout << std::endl;
        std::cout << "deleted:" << std::endl;
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

        auto hosts = etcd_client.ListSearcherHosts(index_id, shard_id);
        std::cout << "searcher hosts:" << std::endl;
        for (const auto& host : hosts) {
            std::cout << host << std::endl;
        }
        UpdateRequest request;
        request.mutable_added()->Assign(added.begin(), added.end());
        request.mutable_deleted()->Assign(deleted.begin(), deleted.end());
        request.set_index_id(index_id);
        request.set_shard_id(shard_id);
        for (auto& host : hosts) {
            std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(host, grpc::InsecureChannelCredentials());
            SearcherClient client(channel, host);
            UpdateResponse response = client.updateIndex(request);
        }
    };

    boost::asio::post(callback_runner, task);
}

void WriterImpl::updateIndexesState() {
    for (const auto& p : indexes_create_agrs) {
        std::string index_id = p.first;
        json index_config = p.second;

        std::vector<std::string> shard_ids_vec = etcd_client.ListShardIds(index_id);

        std::cout << "Shards for index_id \"" << index_id << "\" from etcd:" << std::endl;
        for (const auto& shard_id : shard_ids_vec) {
            std::cout << shard_id << " ";
        }
        std::cout << std::endl;

        std::set<std::string> shard_ids(shard_ids_vec.begin(), shard_ids_vec.end());
        if (shard_ids_vec.empty()) {
            shards.erase(index_id);
            continue;
        }

        if (shards.find(index_id) == shards.end()) {
            bool ok = storage_client.createBucket(index_id);
            if (!ok) {
                // todo
            }
            shards[index_id] = std::unordered_map<std::string, VecodexIndex>{};
        }
        std::vector<std::string> to_add;
        std::vector<std::string> to_del;
        for (const auto& shard : shards[index_id]) {
            if (shard_ids.find(shard.first) == shard_ids.end()) {
                to_del.push_back(shard.first);
            }
        }
        for (const auto& shard_id : shard_ids) {
            if (shards[index_id].find(shard_id) == shards[index_id].end()) {
                to_add.push_back(shard_id);
            }
        }

        for (const auto& shard_id_del : to_del) {
            shards[index_id].erase(shard_id_del);
            std::cout << "Deleted shard [" << index_id << ", " << shard_id_del << "]" << std::endl;
        }
        for (const auto& shard_id_add : to_add) {
            shards[index_id][shard_id_add] = vecodex::CreateIndex<std::string>(indexes_create_agrs[index_id]);
            auto callback = [this, index_id, shard_id_add](auto&& PH1, auto&& PH2) {
                indexUpdateCallback(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), index_id, shard_id_add);
            };
            shards[index_id][shard_id_add]->setUpdateCallback(callback);
            std::cout << "Created shard [" << index_id << ", " << shard_id_add << "]" << std::endl;
        }
    }
}

std::string WriterImpl::chooseShard(const std::string& index_id) {
    bool found = false;
    std::string shard_id;
    double min_time = 0;
    for (const auto& shard : shards[index_id]) {
        double shard_time = etcd_client.GetSearcherAverageResponseTime(index_id, shard.first);
        if (!found) {
            found = true;
            shard_id = shard.first;
            min_time = shard_time;
        }
        else {
            if (shard_time < min_time) {
                shard_id = shard.first;
                min_time = shard_time;
            }
        }
    }
    std::cout << "Chosen shard [" << index_id << ", " << shard_id << "] with response time " << min_time << std::endl;
    return shard_id;
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