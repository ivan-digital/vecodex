#include "EtcdClient.h"
#include "etcd/Response.hpp"
#include <aws/common/date_time.h>
#include <boost/algorithm/string/split.hpp>
#include <cassert>
#include <optional>
#include <stdexcept>
#include <vector>
#include <set>
#include <boost/algorithm/string.hpp>

/*

Structure:

index_id/
-- searchers/
----- shard_id/
-------- hosts/
---------- searcher_addr_1
---------- searcher_addr_2
---------- ...
-------- stats/
---------- average_response_time
---------- errors_cnt
---------- ...

*/

EtcdClient::EtcdClient(const string& etcd_addr)
    : client_(etcd::Client(etcd_addr)) {}

std::vector<std::string> EtcdClient::ListSearcherHosts(const std::string& index_id, const std::optional<std::string>& shard_id) {
    std::string key = index_id + "/searchers";
    std::vector<std::string> hosts;
    if (shard_id.has_value()) {
        key += "/" + shard_id.value() + "/hosts";
    }
    return GetAllValues(key);
}

etcd::Response EtcdClient::AddSearcherHost(const std::string& index_id, const std::string& shard_id, const std::string& searcher_addr) {
    return AddElement(index_id + "/searchers/" + shard_id + "/hosts/" + searcher_addr, searcher_addr);
}

etcd::Response EtcdClient::RemoveSearcherHost(const std::string& index_id, const std::string& shard_id, const std::string& searcher_addr) {
    return RemoveElement(index_id + "/searchers/" + shard_id + "/hosts/" + searcher_addr);
}

std::vector<std::string> EtcdClient::ListShardIds(const std::string& index_id) {
    return ListAllKeys(index_id + "/searchers", 2);
}

double EtcdClient::GetSearcherAverageResponseTime(const std::string& index_id, const std::string& shard_id) {
    auto result = GetAllValues(index_id + "/searchers/" + shard_id + "/stats/average_response_time");
    assert(result.size() == 1);
    return std::stod(result[0]);
}

etcd::Response EtcdClient::UpdateAverageResponseTime(const std::string& index_id, const std::string& shard_id, double updated_value) {
    return AddElement(index_id + "/searchers/" + shard_id + "/stats/average_response_time", std::to_string(updated_value));
}


std::vector<std::string> EtcdClient::GetAllValues(const std::string& path) {
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

std::vector<std::string> EtcdClient::ListAllKeys(const std::string& path, size_t nesting_level) {
    std::set<std::string> result;
    etcd::Response response;
    try {
        response = client_.keys(path).get();
        if (!response.is_ok()) {
            throw std::runtime_error("Error while listing keys. etcd response: " + response.error_message());
        }
        for (size_t i = 0; i < response.keys().size(); i++) {
            std::vector<std::string> splited;
            boost::algorithm::split(splited, response.keys()[i], [](char c)->bool {
                return c == '/';
            });
            if (nesting_level >= splited.size()) {
                throw runtime_error("Nesting level provided exceeded path length");
            }
            result.insert(splited[nesting_level]);
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        throw e;
    }
    return std::vector<std::string>(result.begin(), result.end());
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

void EtcdClient::DebugPrintEverything() {
    etcd::Response response;
    try {
        response = client_.keys("").get();
        if (!response.is_ok()) {
            throw std::runtime_error("Error while printing etcd debug info. etcd response: " + response.error_message());
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        throw e;
    }
    std::cout << "Printing etcd keys:\n";
    for (const auto& elem : response.keys()) {
        std::cout << elem << std::endl;
    }
}

