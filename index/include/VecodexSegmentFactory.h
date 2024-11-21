#ifndef VECODEX_SEGMENT_FACTORY_H
#define VECODEX_SEGMENT_FACTORY_H

#include "VecodexSegment.h"
#include "faiss/MetricType.h"

enum IndexType {
    Flat,
    HNSW
};
class VecodexSegmentFactory {
public:
    VecodexSegmentFactory(IndexType type) : type_(type) {}
    VecodexSegment createIndexSegment(int dim, faiss::MetricType metric = faiss::MetricType::METRIC_L2);
private:
    static VecodexSegment createFlat(int dim, faiss::MetricType metric = faiss::MetricType::METRIC_L2);
    static VecodexSegment createHNSW(int dim, int M = 32 /*IDK*/, faiss::MetricType metric = faiss::MetricType::METRIC_L2);
    IndexType type_;
};
#endif // VECODEX_SEGMENT_FACTORY_H