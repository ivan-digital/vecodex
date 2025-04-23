#pragma once

#include "etcd/Response.hpp"
#include <string>
#include <optional>
#include <etcd/Client.hpp>
#include <vector>

class EtcdClient {
public:
    EtcdClient(const std::string& etcd_addr);

    std::vector<std::string> ListSearcherHosts(const std::string& index_id, const std::optional<std::string>& shard_id = std::nullopt);

    etcd::Response AddSearcherHost(const std::string& index_id, const std::string& shard_id, const std::string& searcher_addr);

    etcd::Response RemoveSearcherHost(const std::string& index_id, const std::string& shard_id, const std::string& searcher_addr);

    std::vector<std::string> ListShardIds(const std::string& index_id);

    double GetSearcherAverageResponseTime(const std::string& index_id, const std::string& shard_id);

    etcd::Response UpdateAverageResponseTime(const std::string& index_id, const std::string& shard_id, double updated_value);

    void DebugPrintEverything();

private:
    etcd::Client client_;

private:
    std::vector<std::string> ListAllKeys(const std::string& path, size_t nesting_level);
    
    std::vector<std::string> GetAllValues(const std::string& path);

    etcd::Response AddElement(const std::string& path, const std::string& value);

    etcd::Response RemoveElement(const std::string& path);
};