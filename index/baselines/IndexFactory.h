#pragma once
#include <nlohmann/json.hpp>
#include <algorithm>
#include "Index.h"
#include "faiss.h"

using Json = nlohmann::json;

namespace vecodex {
static const std::string faissFlat = "faissFlat";
static const std::string faissHNSWFlat = "faissHNSWFlat";
static const std::unordered_map<std::string, faiss::MetricType>
	faiss_metric_map = {
		{"L1", faiss::MetricType::METRIC_L1},
		{"L2", faiss::MetricType::METRIC_L2},
		{"Linf", faiss::MetricType::METRIC_Linf},
};
template <typename IDType>
std::shared_ptr<vecodex::IIndex<IDType>> CreateIndex(const Json& json) {
	std::string type = json["type"].get<std::string>();
	std::string lib = json["library"].get<std::string>();
	int dim = json["dim"].get<int>();
	int threshold = json["threshold"].get<int>();
	std::string metric = json["metric"].get<std::string>();
	if (lib == "faiss") {
		faiss::MetricType metric_type = faiss_metric_map.at(metric);
		if (type == faissFlat) {
			return std::make_shared<
				vecodex::Index<baseline::FaissIndex<faiss::IndexFlat, IDType>>>(
				dim, threshold, std::nullopt, dim, metric_type);
		} else if (type == faissHNSWFlat) {
			int M = json["M"].get<int>();
			return std::make_shared<vecodex::Index<
				baseline::FaissIndex<faiss::IndexHNSWFlat, IDType>>>(
				dim, threshold, std::nullopt, dim, M, metric_type);
		} else {
			throw std::runtime_error("Doesn't support index type " + type);
		}
	} else {
		throw std::runtime_error("Doesn't support such library as " + lib);
	}
}
}  // namespace vecodex