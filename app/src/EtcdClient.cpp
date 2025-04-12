#include "EtcdClient.h"
#include "etcd/Response.hpp"
#include <optional>
#include <vector>

EtcdClient::EtcdClient(const string& etcd_addr)
    : client_(etcd::Client(etcd_addr)) {}

std::vector<std::string> EtcdClient::ListSearcherHosts(const std::string& index_id, const std::optional<std::string>& shard_id) {
    std::string key = index_id + "/searchers";
    std::vector<std::string> hosts;

    if (shard_id.has_value()) {
        key += "/" + shard_id.value();
        return ListAllElements(key);
    }
    const auto shards_ids = ListAllElements(key);
    for (const auto& shard_id : shards_ids) {
        auto found = ListAllElements(key + "/" + shard_id);
        hosts.insert(hosts.end(), found.begin(), found.end());
    }
    return hosts;
}

etcd::Response EtcdClient::AddSearcherHost(const std::string& index_id, const std::string& shard_id, const std::string& searcher_addr) {
    return AddElement(index_id + "/searchers/" + shard_id + "/" + searcher_addr, searcher_addr);
}

etcd::Response EtcdClient::RemoveSearcherHost(const std::string& index_id, const std::string& shard_id, const std::string& searcher_addr) {
    return RemoveElement(index_id + "/searchers/" + shard_id + "/" + searcher_addr);
}

std::vector<std::string> EtcdClient::ListShardIds(const std::string& index_id) {
    return ListAllElements(index_id + "/searchers");
}

std::vector<std::string> EtcdClient::ListAllElements(const std::string& path) {
    std::vector<std::string> result;
    etcd::Response response;
    try {
        response = client_.ls(path).get();
        if (!response.is_ok()) {
            throw std::runtime_error("Error while listing keys. etcd response: " + response.error_message());
        }
        for (size_t i = 0; i < response.keys().size(); i++) {
            result.emplace_back(response.values()[i].as_string());
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        throw e;
    }
    return result;
}

etcd::Response EtcdClient::AddElement(const std::string& path, const std::string& value) {
    try {
        auto response = client_.set(path, value).get();
        if (!response.is_ok()) {
            throw std::runtime_error("Error while adding searcher. etcd response: " + response.error_message());
        }
        return response;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        throw e;
    }
}

etcd::Response EtcdClient::RemoveElement(const std::string& path) {
    try {
        auto response = client_.rm(path).get();
        if (!response.is_ok()) {
            throw std::runtime_error("Error while removing searcher. etcd response: " + response.error_message());
        }
        return response;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        throw e;
    }
}

