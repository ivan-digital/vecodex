// include/VecodexIndex.h
#ifndef VECODEX_INDEX_H
#define VECODEX_INDEX_H

#include "VecodexSegment.h"
#include "VecodexSegmentFactory.h"
#include <vector>

class VecodexIndex {
public:
    VecodexIndex(int dimension, int segmentThreshold, IndexType type);

    void addVector(const std::string& id, const std::vector<float>& vector, const std::unordered_map<std::string, std::string>& attributes);
    std::vector<std::string> search(const std::vector<float>& query, int k);

    void updateVector(const std::string& id, const std::vector<float>& vector, const std::unordered_map<std::string, std::string>& attributes);
    void mergeSegments();

private:
    int dimension_;
    int segmentThreshold_;
    VecodexSegmentFactory factory_;
    std::vector<VecodexSegment> segments_;
};

#endif // VECODEX_INDEX_H

