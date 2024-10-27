// src/VecodexSegment.cpp
#include "VecodexSegment.h"

void VecodexSegment::addVector(const std::string& id, const std::vector<float>& vector, const std::unordered_map<std::string, std::string>& attributes) {
    index->add(1, vector.data());
    metadata.emplace_back(id, attributes);
}

std::vector<std::string> VecodexSegment::search(const std::vector<float>& query, int k) const {
    std::vector<int64_t> indices(k);  // Change the type here
    std::vector<float> distances(k);

    index->search(1, query.data(), k, distances.data(), indices.data());

    std::vector<std::string> resultIds;
    for (const auto& idx : indices) {
        if (idx < metadata.size()) {
            resultIds.push_back(metadata[idx].getId());
        }
    }
    return resultIds;
}

void VecodexSegment::mergeSegment(const VecodexSegment& other) {
    index->merge_from(*other.index, 0);
    metadata.insert(metadata.end(), other.metadata.begin(), other.metadata.end());
}
