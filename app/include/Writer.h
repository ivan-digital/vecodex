#ifndef VECODEX_WRITER_H
#define VECODEX_WRITER_H

#include "base.h"

#include <vector>
#include <string>
#include <unordered_map>

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/S3Client.h>

using namespace Aws;
using namespace Aws::Auth;

namespace {
    struct Event {
        Event() = default;

        Event(const std::string& id, const std::vector<float>& vector,
              const std::unordered_map <std::string, std::string>& attributes)
              : id(id), vector(vector), attributes(attributes) {};

        std::string id;
        std::vector<float> vector;
        std::unordered_map <std::string, std::string> attributes;
    };
}

class Writer : public BaseServer {
public:
    Writer(const std::string& host, const std::string& port, const std::string& s3_host);

    ~Writer();

    void Run();

private:
    void receiveUpdate(const std::string& id, const std::vector<float>& vector, const std::unordered_map <std::string, std::string>& attributes);

    void pushUpdates();

    bool putObjectBuffer(const Aws::String &bucketName, const Aws::String &objectName, const std::string &objectContent, const Aws::S3::S3ClientConfiguration &clientConfig, const Aws::Auth::AWSCredentials &credentials);

    std::vector<Event> updates;
    std::string s3_host;
    Aws::SDKOptions options;
};

#endif //VECODEX_WRITER_H
