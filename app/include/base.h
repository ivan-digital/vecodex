#pragma once

#include <service.grpc.pb.h>
#include <string>

using service::BaseService;

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