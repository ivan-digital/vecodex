#include "VecodexSegmentFactory.h"
#include "IndexConfig.h"
#include "faiss/IndexFlat.h"
#include "faiss/IndexHNSW.h"
namespace baseline {
void IndexFlatAdd(const std::unique_ptr<faiss::IndexFlat>& index, size_t n, const float* vectors);

void IndexFlatSearch(const std::unique_ptr<faiss::IndexFlat>& index, size_t k, const float* q, float* dist, size_t *ids);

void IndexFlatMerge(const std::unique_ptr<faiss::IndexFlat>& index, std::unique_ptr<faiss::IndexFlat>&& other);

void IndexHNSWAdd(const std::unique_ptr<faiss::IndexHNSWFlat>& index, size_t n, const float* vectors);

void IndexHNSWSearch(const std::unique_ptr<faiss::IndexHNSWFlat>& index, size_t k, const float* q, float* dist, size_t *ids);

void IndexHNSWMerge(const std::unique_ptr<faiss::IndexHNSWFlat>& index, std::unique_ptr<faiss::IndexHNSWFlat>&& other);
}
