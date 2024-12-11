#include "Writer.h"

Writer::Writer(const std::string& host, const std::string& port, const std::string& s3_host): BaseServer(host, port), s3_host(s3_host) {}

Writer::~Writer() {
    Aws::ShutdownAPI(options);
}

void Writer::Run() {
    Aws::InitAPI(options);
    // todo
}

void Writer::receiveUpdate(const std::string &id, const std::vector<float> &vector,
                           const std::unordered_map <std::string, std::string> &attributes) {
    updates.emplace_back(id, vector, attributes);
}

void Writer::pushUpdates() {
    Aws::Client::ClientConfiguration config;
    config.scheme = Aws::Http::Scheme::HTTP;
    config.endpointOverride = Aws::String(s3_host);
    Aws::Auth::AWSCredentials credentials; // todo: credentials?

    // todo
}

bool Writer::putObjectBuffer(const Aws::String &bucketName,
                             const Aws::String &objectName,
                             const std::string &objectContent,
                             const Aws::S3::S3ClientConfiguration &clientConfig,
                             const Aws::Auth::AWSCredentials &credentials) {
    Aws::S3::S3Client s3Client(credentials, clientConfig, Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, true);

    Aws::S3::Model::PutObjectRequest request;
    request.SetBucket(bucketName);
    request.SetKey(objectName);

    const std::shared_ptr<Aws::IOStream> inputData = Aws::MakeShared<Aws::StringStream>("");
    *inputData << objectContent.c_str();

    request.SetBody(inputData);

    Aws::S3::Model::PutObjectOutcome outcome = s3Client.PutObject(request);

    if (!outcome.IsSuccess()) {
        std::cerr << "Error: putObjectBuffer: " << outcome.GetError().GetMessage() << std::endl;
    }
    return outcome.IsSuccess();
}


