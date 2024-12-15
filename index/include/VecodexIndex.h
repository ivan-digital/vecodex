#pragma once

#include "VecodexSegment.h"
#include "VecodexSegmentFactory.h"
#include <vector>

class VecodexIndex {
public:
    VecodexIndex(int segmentThreshold, const IndexConfig& config);

    void addVector(const std::string& id, const std::vector<float>& vector);
    std::vector<std::string> search(const std::vector<float>& query, int k);

    void updateVector(const std::string& id, const std::vector<float>& vector);
    void mergeSegments();

private:
    size_t segmentThreshold_;
    VecodexSegmentFactory factory_;
    std::vector<VecodexSegment> segments_;
};
