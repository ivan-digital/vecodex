#pragma once

#include "BaseClient.h"

using service::UpdateRequest;
using service::UpdateResponse;

class SearcherClient final : public BaseClient {
    using BaseClient::BaseClient;

public:
    UpdateResponse updateIndex(const UpdateRequest& request) const;
};