#include "StorageClient.h"

#include <iostream>

#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>

StorageClient::StorageClient(const std::string& host) {
    Aws::InitAPI(options);
    config.emplace();
    config->scheme = Aws::Http::Scheme::HTTP;
    config->endpointOverride = Aws::String(host);
}

StorageClient::~StorageClient() {
    s3Client.reset();
    config.reset();
    Aws::ShutdownAPI(options);
}

void StorageClient::logIn(const std::string& login, const std::string& password) {
    Aws::Auth::AWSCredentials credentials;
    credentials.SetAWSAccessKeyId(login);
    credentials.SetAWSSecretKey(password);
    s3Client.emplace(credentials, config.value(), Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, true);
}

bool StorageClient::putObject(const std::string& bucket_name, const std::string& filename) {
    Aws::S3::Model::PutObjectRequest request;
    request.SetBucket(bucket_name);
    request.SetKey(filename);

    std::shared_ptr<Aws::IOStream> input_data = Aws::MakeShared<Aws::FStream>("SampleAllocationTag", filename.c_str(), std::ios_base::in | std::ios_base::binary);

    if (!*input_data) {
        std::cerr << "Error unable to read file " << filename << std::endl;
        return false;
    }

    request.SetBody(input_data);
    Aws::S3::Model::PutObjectOutcome outcome = s3Client->PutObject(request);

    if (!outcome.IsSuccess()) {
        std::cerr << "Error: putObject: " << outcome.GetError().GetMessage() << std::endl;
    }
    else {
        std::cout << "Added object '" << filename << "' to bucket '" << bucket_name << "'.";
    }

    return outcome.IsSuccess();
}

bool StorageClient::getObject(const std::string& bucket_name, const std::string& filename) {
    Aws::S3::Model::GetObjectRequest request;
    request.SetBucket(bucket_name);
    request.SetKey(filename);

    Aws::S3::Model::GetObjectOutcome outcome = s3Client->GetObject(request);
    outcome.GetResult();

    if (!outcome.IsSuccess()) {
        const Aws::S3::S3Error &err = outcome.GetError();
        std::cerr << "Error: getObject: " << err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
    }
    else {
        std::cout << "Successfully retrieved '" << filename << "' from '" << bucket_name << "'." << std::endl;
        std::ofstream output(filename, std::ios_base::out | std::ios_base::binary);
        output << outcome.GetResult().GetBody().rdbuf();
        output.close();
    }

    return outcome.IsSuccess();
}

bool StorageClient::delObject(const std::string& bucket_name, const std::string& filename) {
    Aws::S3::Model::DeleteObjectRequest request;
    request.WithKey(filename).WithBucket(bucket_name);

    Aws::S3::Model::DeleteObjectOutcome outcome = s3Client->DeleteObject(request);

    if (!outcome.IsSuccess()) {
        auto err = outcome.GetError();
        std::cerr << "Error: deleteObject: " << err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
    }
    else {
        std::cout << "Successfully deleted the object." << std::endl;
    }

    return outcome.IsSuccess();
}