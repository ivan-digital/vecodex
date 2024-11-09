// src/VecodexIndex.cpp
#include "VecodexIndex.h"

VecodexIndex::VecodexIndex(int dimension, int segmentThreshold)
    : dimension(dimension), segmentThreshold(segmentThreshold) {
    segments.emplace_back(dimension);
}

void VecodexIndex::addVector(const std::string& id, const std::vector<float>& vector, const std::unordered_map<std::string, std::string>& attributes) {
    if (segments.back().size() >= segmentThreshold) {
        segments.emplace_back(dimension);
    }
    segments.back().addVector(id, vector, attributes);
}

std::vector<std::string> VecodexIndex::search(const std::vector<float>& query, int k) {
    std::vector<std::string> results;
    for (const auto& segment : segments) {
        auto segmentResults = segment.search(query, k);
        results.insert(results.end(), segmentResults.begin(), segmentResults.end());
    }
    // Further logic to select top-k from combined results
    return results;
}

void VecodexIndex::updateVector(const std::string& id, const std::vector<float>& vector, const std::unordered_map<std::string, std::string>& attributes) {
    // Find and update the vector in the appropriate segment
    // This part requires handling vector identification and re-indexing as needed
}

void VecodexIndex::mergeSegments() {
    // Merge segments based on your policy (e.g., combining small segments, periodic merges)
    if (segments.size() > 1) {
        VecodexSegment mergedSegment(dimension);
        for (const auto& segment : segments) {
            mergedSegment.mergeSegment(segment);
        }
        segments.clear();
        segments.push_back(std::move(mergedSegment));
    }
}

