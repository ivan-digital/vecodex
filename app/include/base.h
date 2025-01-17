#pragma once

#include <service.grpc.pb.h>
#include <string>

using service::BaseService;
using service::SearchResponse;
using service::SearchRequest;


class BaseServer {
public:
    BaseServer(const std::string& host, const std::string& port);

    ~BaseServer() {}

    virtual void Run() = 0;
protected:
    std::string host;
    std::string port;

    void InternalRun(BaseService::Service& service);
};


class BaseClient {
public:
    BaseClient(const std::shared_ptr<grpc::ChannelInterface> channel);

    SearchResponse getProcessedDocuments(const SearchRequest& request) const;
private:
    std::unique_ptr<BaseService::Stub> stub_;
};