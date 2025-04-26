#pragma once

#include <service.grpc.pb.h>
#include <string>

using service::BaseService;
using service::SearchResponse;
using service::SearchRequest;

class BaseClient {
public:
    BaseClient(const std::shared_ptr<grpc::ChannelInterface> channel, std::string&& addr);
    
    BaseClient(const std::shared_ptr<grpc::ChannelInterface> channel, const std::string& addr);

    SearchResponse getProcessedDocuments(const SearchRequest& request) const;

    std::string GetAddress() const;
protected:
    std::unique_ptr<BaseService::Stub> stub_;
private:
    std::string address;
};