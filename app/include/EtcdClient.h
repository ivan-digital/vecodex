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

private:
    etcd::Client client_;

private:
    std::vector<std::string> ListAllElements(const std::string& path);

    etcd::Response AddElement(const std::string& path, const std::string& value);

    etcd::Response RemoveElement(const std::string& path);
};