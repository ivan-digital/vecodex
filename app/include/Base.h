#pragma once

#include <service.grpc.pb.h>
#include <string>
#include "EtcdClient.h"
#include <nlohmann/json.hpp>

using service::BaseService;
using service::SearchResponse;
using service::SearchRequest;
using json = nlohmann::json;


class BaseServer {
public:
    BaseServer(const json& config);

    ~BaseServer() {}

    virtual void Run() = 0;
protected:
    std::string host;
    std::string port;

    void InternalRun(BaseService::Service& service);
};