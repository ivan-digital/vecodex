#pragma once

#include "base.h"
#include "VecodexIndex.h"
#include "StorageClient.h"

#include <vector>
#include <string>
#include <unordered_map>

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/S3Client.h>

using namespace Aws;
using namespace Aws::Auth;


class Writer : public BaseServer {
public:
    Writer(const std::string& host, const std::string& port, const std::string& s3_host, size_t threshold, const std::string& config_filename);

    ~Writer();

    void Run();

private:
    void receiveUpdate(const std::string& id, const std::vector<float>& vector, const std::unordered_map <std::string, std::string>& attributes);

    void pushUpdate(const VecodexSegment& segment);

    std::string s3_host;
    std::optional<VecodexIndex> index;
    std::optional<StorageClient> storage_client;
};

