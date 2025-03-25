#pragma once

#include <fstream>
#include <optional>

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/core/auth/AWSCredentialsProvider.h>


class StorageClient {
public:
    explicit StorageClient(const std::string& host);

    ~StorageClient();

    bool logIn(const std::string& login, const std::string& password);

    bool putObject(const std::string& bucket_name, const std::string& filename);

    bool getObject(const std::string& bucket_name, const std::string& filename);

    bool delObject(const std::string& bucket_name, const std::string& filename);

    bool createBucket(const std::string& bucket_name);

private:
    Aws::SDKOptions options;
    std::optional<Aws::Client::ClientConfiguration> config;
    std::optional<Aws::S3::S3Client> s3Client;
};