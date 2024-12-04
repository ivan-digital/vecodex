#include "VecodexSegment.h"
#include "VecodexSegmentFactory.h"
#include "faiss/IndexFlat.h"
#include "faiss/IndexHNSW.h"
#include <memory>

VecodexSegmentFactory::VecodexSegmentFactory(IndexConfig config) : config_(config) {
}

VecodexSegment VecodexSegmentFactory::createIndexSegment() const {

    switch (config_.type) {
        case (IndexType::Flat):
        {
            return createFlat(config_.dim, config_.metric);
        }
        case (IndexType::HNSW):
        {
            return createHNSW(config_.dim, config_.M, config_.metric);
        }
        default: {
            throw std::runtime_error("Couldn't recognize vector index type");
        }
    }
}
VecodexSegment VecodexSegmentFactory::createFlat(int dim, faiss::MetricType metric) {
    auto index_ptr = std::make_unique<faiss::IndexFlat>(dim, metric);
    return VecodexSegment(std::move(index_ptr));
}
VecodexSegment VecodexSegmentFactory::createHNSW(int dim, int M, faiss::MetricType metric) {
    auto index_ptr = std::make_unique<faiss::IndexHNSW>(dim, M, metric);
    return VecodexSegment(std::move(index_ptr));
}