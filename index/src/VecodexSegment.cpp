#include "VecodexSegment.h"
VecodexSegment::VecodexSegment(std::unique_ptr<faiss::Index> index) : index_(std::move(index)) {}

VecodexSegment::VecodexSegment(VecodexSegment&& other) : index_(std::move(other.index_)), metadata_(std::move(other.metadata_)) {}

void VecodexSegment::addVector(const std::string& id, const std::vector<float>& vector, const std::unordered_map<std::string, std::string>& attributes) {
    index_->add(1, vector.data());
    metadata_.emplace_back(id, attributes);
}

std::unordered_map<std::string, float> VecodexSegment::search(const std::vector<float>& query, int k) const {
    std::vector<int64_t> indices(k);  // Change the type here
    std::vector<float> distances(k);

    index_->search(1, query.data(), k, distances.data(), indices.data());
    std::unordered_map<std::string, float> result;
    for (size_t i = 0; i < indices.size(); ++i) {
        auto idx = indices[i];
        auto dist = distances[i];
        if (idx >= metadata_.size()) {
            continue;
        }
        result[metadata_[idx].getId()] = dist;

    }

    return result;
}

void VecodexSegment::mergeSegment(const VecodexSegment& other) {
    index_->merge_from(*other.index_, 0);
    metadata_.insert(metadata_.end(), other.metadata_.begin(), other.metadata_.end());
}
