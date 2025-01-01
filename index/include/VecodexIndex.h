#pragma once

#include "VecodexSegment.h"
#include "VecodexSegmentFactory.h"
#include <vector>

class VecodexIndex {
public:
    VecodexIndex(int segmentThreshold, const IndexConfig& config);
    
    void add(size_t n, const IDType* ids, const float* vectors);

    void addVector(const IDType& id, const std::vector<float>& vector);
    std::vector<IDType> search(const std::vector<float>& query, int k);

    void updateVector(const IDType& id, const std::vector<float>& vector);
    void mergeSegments();

private:
    size_t segmentThreshold_;
    VecodexSegmentFactory factory_;
    std::vector<VecodexSegment> segments_;
};
