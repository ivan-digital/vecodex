#include "etcd_client.h"
#include <vector>

EtcdClient::EtcdClient(const string& etcd_addr)
    : client_(etcd::Client(etcd_addr)) {}

std::vector<std::string> EtcdClient::ListSearcherHostsByIndexId(const std::string& index_id) {
    std::string key = index_id + "/searchers";
    std::vector<std::string> hosts;
    etcd::Response response;

    try {
        response = client_.ls(key).get();
        if (!response.is_ok()) {
            throw std::runtime_error("Error while listing keys. etcd response: " + response.error_message());
        }
        for (size_t i = 0; i < response.keys().size(); i++) {
            hosts.emplace_back(response.values().at(i).key());
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        throw e;
    }
    return hosts;
}

etcd::Response EtcdClient::AddSearcherHostByIndexId(const std::string& index_id, const std::string& searcher_id) {
    std::string key = index_id + "/searchers/" + "/" + searcher_id;
    std::string value = "1";
    try {
        auto response = client_.set(key, value).get();
        if (!response.is_ok()) {
            throw std::runtime_error("Error while adding searcher by index_id. etcd response: " + response.error_message());
        }
        return response;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        throw e;
    }
}

etcd::Response EtcdClient::RemoveSearcherHostByIndexId(const std::string& index_id, const std::string& searcher_id) {
    std::string key = index_id + "/searchers/" + "/" + searcher_id;
    try {
        auto response = client_.rm(key).get();
        if (!response.is_ok()) {
            throw std::runtime_error("Error while removing searcher by index_id. etcd response: " + response.error_message());
        }
        return response;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        throw e;
    }
}

etcd::Response EtcdClient::AddSearcherHost(const std::string& searcher_id) {
    std::string key = "searchers/" + searcher_id;
    std::string value = "1";
    try {
        auto response = client_.set(key, value).get();
        if (!response.is_ok()) {
            throw std::runtime_error("Error while adding searcher. etcd response: " + response.error_message());
        }
        return response;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        throw e;
    }
}

etcd::Response EtcdClient::RemoveSearcherHost(const std::string& searcher_id) {
    std::string key = "searchers/" + searcher_id;

    try {
        auto response = client_.rm(key).get();
        if (!response.is_ok()) {
            throw std::runtime_error("Error while removing searcher. etcd response: " + response.error_message());
        }
        return response;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        throw e;
    }
}

