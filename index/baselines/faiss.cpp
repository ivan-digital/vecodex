#include "faiss.h"
void baseline::IndexFlatAdd(const std::unique_ptr<faiss::IndexFlat>& index, size_t n, const float* vectors) {
    index->add(n, vectors);
}

void baseline::IndexFlatSearch(const std::unique_ptr<faiss::IndexFlat>& index, size_t k, const float* q, float* dist, size_t *ids) {
    index->search(1, q, k, dist, (faiss::idx_t*)ids);
}

void baseline::IndexFlatMerge(const std::unique_ptr<faiss::IndexFlat>& index, std::unique_ptr<faiss::IndexFlat>&& other) {
    index->merge_from(*other);
}

void baseline::IndexHNSWAdd(const std::unique_ptr<faiss::IndexHNSWFlat>& index, size_t n, const float* vectors) {
    index->add(n, vectors);
}

void baseline::IndexHNSWSearch(const std::unique_ptr<faiss::IndexHNSWFlat>& index, size_t k, const float* q, float* dist, size_t *ids) {
    index->search(1, q, k, dist, (faiss::idx_t*)ids);
}

void baseline::IndexHNSWMerge(const std::unique_ptr<faiss::IndexHNSWFlat>& index, std::unique_ptr<faiss::IndexHNSWFlat>&& other) {
    std::vector<float> vectors(other->ntotal * other->d);
    other->reconstruct_n(0, other->ntotal, vectors.data());
    index->add(other->ntotal, vectors.data());
}