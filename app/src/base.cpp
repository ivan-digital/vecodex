#include "base.h"

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>

using service::BaseService;


BaseServer::BaseServer(const std::string& host, const std::string& port)
    : host(host), port(port) {}

void BaseServer::InternalRun(BaseService::Service& service) {
    grpc::ServerBuilder builder;

    std::string fullAddress = host + ":" + port; 

    builder.AddListeningPort(fullAddress, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << fullAddress << std::endl;
    server->Wait();
}