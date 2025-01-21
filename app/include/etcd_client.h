#include <string>
#include <etcd/Client.hpp>

class EtcdClient {
public:
    EtcdClient(const std::string& etcd_addr);

    std::vector<std::string> ListSearcherHostsByIndexId(const std::string& index_id);

    etcd::Response AddSearcherHostByIndexId(const std::string& index_id, const std::string& searcher_id);

    etcd::Response AddSearcherHost(const std::string& searcher_id);

    etcd::Response RemoveSearcherHost(const std::string& searcher_id);

    etcd::Response RemoveSearcherHostByIndexId(const std::string& index_id, const std::string& searcher_id);
private:
    etcd::Client client_;
};