#pragma once

#include <service.grpc.pb.h>
#include <string>

using service::BaseService;
using service::SearchResponse;
using service::SearchRequest;

class BaseClient {
public:
    BaseClient(const std::shared_ptr<grpc::ChannelInterface> channel);

    SearchResponse getProcessedDocuments(const SearchRequest& request) const;
protected:
    std::unique_ptr<BaseService::Stub> stub_;
};