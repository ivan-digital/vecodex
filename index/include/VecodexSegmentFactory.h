#pragma once
#include "VecodexSegment.h"
#include "faiss/MetricType.h"
#include "IndexConfig.h"

class VecodexSegmentFactory {
public:
    VecodexSegmentFactory(IndexConfig config);
    VecodexSegment createIndexSegment() const;
private:
    static VecodexSegment createFlat(int dim, faiss::MetricType metric = faiss::MetricType::METRIC_L2);
    static VecodexSegment createHNSW(int dim, int M = 32 /*IDK*/, faiss::MetricType metric = faiss::MetricType::METRIC_L2);
    IndexConfig config_;
};
