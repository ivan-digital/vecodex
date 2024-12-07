#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <faiss/IndexFlat.h>
#include "DocumentMetadata.h"
#include "faiss/Index.h"
using IDType = std::string;

class VecodexSegment {
public:
    VecodexSegment(std::unique_ptr<faiss::Index> index);

    VecodexSegment(const VecodexSegment&) = delete;

    VecodexSegment& operator=(const VecodexSegment&) = delete;

    VecodexSegment(VecodexSegment&&);

    void addVector(const IDType& id, const std::vector<float>& vector);
    
    // Mark search as const
    std::unordered_map<std::string, float> search(const std::vector<float>& query, int k) const;

    void mergeSegment(const VecodexSegment& other);

    size_t size() const {
        return ids_.size();
    }

private:
    std::unique_ptr<faiss::Index> index_;
    std::vector<IDType> ids_;
};

struct SearchResult {
    SearchResult(IDType id_value, float dist_value) : dist(dist_value), id(id_value) {}
    bool operator==(const SearchResult& other) const;
    bool operator<(const SearchResult& other) const;
    bool operator>(const SearchResult& other) const;
    float dist;
    IDType id;
};
