#pragma once

#include "service.pb.h"
#include "service.grpc.pb.h"

using service::BaseService;
using service::WriteRequest;
using service::WriteResponse;

class WriterClient {
public:
    WriterClient(const std::shared_ptr<grpc::ChannelInterface>& channel);

    WriteResponse writeDocument(const WriteRequest& request);
private:
    std::unique_ptr<BaseService::Stub> stub_;
};