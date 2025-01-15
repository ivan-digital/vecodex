#pragma once
#include <unordered_map>
#include <string>

#include "faiss/MetricType.h"

enum IndexType {
    Flat,
    HNSW
};

class IndexConfig {
public:
    IndexConfig(std::istream& in);

    IndexConfig(std::string_view config_file);

    IndexType type;
    int dim;
    faiss::MetricType metric;
    int M = 32;

private:
    void readConfig(std::istream& in);
    
};