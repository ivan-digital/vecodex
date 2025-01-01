#include "VecodexIndex.h"
#include "VecodexSegmentFactory.h"
#include <map>
#include <iostream>
#include <set>
#include <algorithm>
VecodexIndex::VecodexIndex(int segmentThreshold, const IndexConfig& config)
    : segmentThreshold_(segmentThreshold), factory_(config) {
    segments_.push_back(factory_.createIndexSegment());
}

void VecodexIndex::addVector(const IDType& id, const std::vector<float>& vector) {
    if (segments_.back().size() >= segmentThreshold_) {
        segments_.push_back(factory_.createIndexSegment());
    }
    segments_.back().addVector(id, vector);
}

void VecodexIndex::add(size_t n, const IDType* ids, const float* vectors) {
    while (n) {
        size_t batch = std::min(n, segmentThreshold_ - segments_.back().size());
        if (!batch) {
            throw std::runtime_error("Empty batch in adding");
        }
        segments_.back().addVectorBatch(batch, ids, vectors);
        ids += batch;
        vectors += (batch * factory_.getConfig().dim);
        n -= batch;
        if (segments_.back().size() == segmentThreshold_) {
            segments_.push_back(factory_.createIndexSegment());
        }
    }
}

std::vector<IDType> VecodexIndex::search(const std::vector<float>& query, int k) {
    std::set<SearchResult> kth_stat;
    for (const auto& segment : segments_) {
        auto segmentResults = segment.search(query, k);
        for (const auto& p : segmentResults) {
            kth_stat.emplace(p.first, p.second);
            if (kth_stat.size() > k) {
                kth_stat.erase(*kth_stat.rbegin());
            }
        }
        // results.insert(results.end(), segmentResults.begin(), segmentResults.end());
    }
    // Further logic to select top-k from combined results
    std::vector<IDType> results;
    for (const auto& vec : kth_stat) {
        results.push_back(vec.id);
    }
    return results;
}

void VecodexIndex::updateVector(const std::string& id, const std::vector<float>& vector) {
    // Find and update the vector in the appropriate segment
    // This part requires handling vector identification and re-indexing as needed
}

void VecodexIndex::mergeSegments() {
    if (segments_.size() == 1) {
        return;
    }
    VecodexSegment mergedSegment(factory_.createIndexSegment());
    for (auto& segment : segments_) {
        mergedSegment.mergeSegment(segment);
    }
    segments_.clear();
    segments_.push_back(std::move(mergedSegment));
}
