#include "Base.h"

#include <grpc/grpc.h>
#include <grpcpp/server.h>
#include <grpcpp/server_builder.h>


BaseServer::BaseServer(const json& config)
    : host(config["hostname"].template get_ref<const std::string&>()), 
      port(config["port"].template get_ref<const std::string&>()) {}

void BaseServer::InternalRun(BaseService::Service& service) {
    grpc::ServerBuilder builder;

    // std::string fullAddress = host + ":" + port; 
    std::string fullAddress = "[::]:" + port; // host + port?

    builder.AddListeningPort(fullAddress, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << port << std::endl;
    server->Wait();
}