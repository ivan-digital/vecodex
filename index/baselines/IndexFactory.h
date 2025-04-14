#include <json/json.h>
#include <algorithm>
#include "ISegment.h"
#include "Index.h"
#include "faiss.h"
#include "io.h"
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
std::shared_ptr<vecodex::IIndex<IDType>> CreateIndex(const Json::Value& json) {
	std::string type = json["type"].asString();
	std::string lib = json["library"].asString();
	int dim = json["dim"].asInt();
	int threshold = json["threshold"].asInt();
	std::string metric = json["metric"].asString();
	if (lib == "faiss") {
		faiss::MetricType metric_type = faiss_metric_map.at(metric);
		if (type == faissFlat) {
			return std::make_shared<
				vecodex::Index<baseline::FaissIndex<faiss::IndexFlat, IDType>>>(
				dim, threshold, dim, metric_type);
		} else if (type == faissHNSWFlat) {
			int M = json["M"].asInt();
			return std::make_shared<vecodex::Index<
				baseline::FaissIndex<faiss::IndexHNSWFlat, IDType>>>(
				dim, threshold, dim, M, metric_type);
		} else {
			throw std::runtime_error("Doesn't support index type " + type);
		}
	} else {
		throw std::runtime_error("Doesn't support such library as " + lib);
	}
}

template <typename IDType>
void SerializeSegment(const std::string& filename,
					  std::shared_ptr<vecodex::ISegment<IDType>> segment) {
	FILE* fd = std::fopen(filename.c_str(), "w");
	SerializeSegment(fd, segment);
std:
	fclose(fd);
}

template <typename IDType>
void SerializeSegment(FILE* fd,
					  std::shared_ptr<vecodex::ISegment<IDType>> segment) {
	writeBinary(fd, segment->seg_type);
	segment->serialize(fd);
}

template <typename IDType>
std::shared_ptr<vecodex::ISegment<IDType>> DeserealizeSegment(
	const std::string& filename) {
	FILE* fd = std::fopen(filename.c_str(), "r");
	std::shared_ptr<vecodex::ISegment<IDType>> res =
		DeserealizeSegment<IDType>(fd);
	std::fclose(fd);
	return res;
}

template <typename IDType>
std::shared_ptr<vecodex::ISegment<IDType>> DeserealizeSegment(FILE* fd) {
	int seg_type = 0;
	readBinary(fd, seg_type);
	if (seg_type == vecodex::SegmentType::kFaissFlat) {
		return std::make_shared<
			vecodex::Segment<baseline::FaissIndex<faiss::IndexFlat, IDType>>>(
			fd);
	} else if (seg_type == vecodex::SegmentType::kFaissHNSW) {
		return std::make_shared<
			vecodex::Segment<baseline::FaissIndex<faiss::IndexHNSW, IDType>>>(
			fd);
	}
	return {};
}
}  // namespace vecodex