#include "VecodexIndex.h"
#include "VecodexSegmentFactory.h"
#include <map>
#include <iostream>
VecodexIndex::VecodexIndex(int segmentThreshold, const IndexConfig& config)
    : segmentThreshold_(segmentThreshold), factory_(config) {
    segments_.push_back(factory_.createIndexSegment());
}

void VecodexIndex::addVector(const std::string& id, const std::vector<float>& vector, const std::unordered_map<std::string, std::string>& attributes) {
    if (segments_.back().size() >= segmentThreshold_) {
        segments_.push_back(factory_.createIndexSegment());
    }
    segments_.back().addVector(id, vector, attributes);
}

std::vector<std::string> VecodexIndex::search(const std::vector<float>& query, int k) {
    std::multimap<float, std::string> kth_stat;
    for (const auto& segment : segments_) {
        auto segmentResults = segment.search(query, k);
        for (const auto& vec : segmentResults) {
            kth_stat.insert({vec.second, vec.first});
            if (kth_stat.size() > k) {
                kth_stat.erase(kth_stat.rbegin()->first);
            }
        }
        // results.insert(results.end(), segmentResults.begin(), segmentResults.end());
    }
    // Further logic to select top-k from combined results
    std::vector<std::string> results;
    for (const auto& vec : kth_stat) {
        results.push_back(vec.second);
    }
    return results;
}

void VecodexIndex::updateVector(const std::string& id, const std::vector<float>& vector, const std::unordered_map<std::string, std::string>& attributes) {
    // Find and update the vector in the appropriate segment
    // This part requires handling vector identification and re-indexing as needed
}

void VecodexIndex::mergeSegments() {
    // Merge segments based on your policy (e.g., combining small segments, periodic merges)
    if (segments_.size() > 1) {
        VecodexSegment mergedSegment(factory_.createIndexSegment());
        for (const auto& segment : segments_) {
            mergedSegment.mergeSegment(segment);
        }
        segments_.clear();
        segments_.push_back(std::move(mergedSegment));
    }
}
