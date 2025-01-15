#include "VecodexSegment.h"
#include <algorithm>
#include <iostream>
bool SearchResult::operator==(const SearchResult& other) const {
    return id == other.id;
}

bool SearchResult::operator<(const SearchResult& other) const {
    return dist < other.dist || id < other.id;
}

bool SearchResult::operator>(const SearchResult& other) const {
    return dist > other.dist || id > other.id;
}


VecodexSegment::VecodexSegment(std::unique_ptr<faiss::Index> index) : index_(std::move(index)) {}

VecodexSegment::VecodexSegment(VecodexSegment&& other) : index_(std::move(other.index_)), ids_(std::move(other.ids_)) {}

void VecodexSegment::addVector(const IDType& id, const std::vector<float>& vector) {
    index_->add(1, vector.data());
    ids_.push_back(id);
}

std::unordered_map<IDType, float> VecodexSegment::search(const std::vector<float>& query, int k) const {
    std::vector<faiss::idx_t> indices(k);
    std::vector<float> distances(k);

    index_->search(1, query.data(), k, distances.data(), indices.data());
    std::unordered_map<IDType, float> result;
    result.reserve(k);
    for (size_t i = 0; i < indices.size(); ++i) {
        result[ids_[indices[i]]] = distances[i];
    }
    return result;
}

void VecodexSegment::mergeSegment(VecodexSegment& other) {
    index_->merge_from(*other.index_);
    std::copy_n(other.ids_.begin(), other.ids_.size(), std::back_inserter(ids_));
}
