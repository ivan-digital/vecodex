#include "VecodexSegment.h"
#include "VecodexSegmentFactory.h"
#include "faiss/IndexFlat.h"
#include "faiss/IndexHNSW.h"
#include <memory>

VecodexSegment VecodexSegmentFactory::createIndexSegment(int dim, faiss::MetricType metric) {
    switch (type_) {
        case (IndexType::Flat):
        {
            return createFlat(dim, metric);
        }
        case (IndexType::HNSW):
        {
            return createHNSW(dim, 32, metric);
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