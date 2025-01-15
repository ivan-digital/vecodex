#include "IndexConfig.h"
#include "json/json.h"
#include <fstream>
IndexConfig::IndexConfig(std::istream& in) {
    readConfig(in);
}

IndexConfig::IndexConfig(std::string_view config_file) {
    std::ifstream config_stream(config_file.data());
    readConfig(config_stream);
}

void IndexConfig::readConfig(std::istream& in) {
    Json::Value config;
    in >> config;
    if (!config.isMember("dim")) {
        throw Json::Exception("dim not provided in config");
    }
    if (!config.isMember("metric")) {
        throw Json::Exception("metric not provided in config");
    }
    std::string type_name = config["index"].asString();
    if (type_name == "Flat") {
        type = IndexType::Flat;
    } else if (type_name == "HNSW") {
        type = IndexType::HNSW;
    }

    dim = config["dim"].asInt();
    std::string metric_name = config["metric"].asString();
    if (metric_name == "L1") {
        metric = faiss::MetricType::METRIC_L1;
    } else if (metric_name == "L2") {
        metric = faiss::MetricType::METRIC_L2;
    } else {
        metric = faiss::MetricType::METRIC_L2;
    }
    if (config.isMember("M")) {
        M = config["M"].asInt(); 
    }
}
