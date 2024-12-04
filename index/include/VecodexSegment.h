#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <faiss/IndexFlat.h>
#include "DocumentMetadata.h"
#include "faiss/Index.h"

class VecodexSegment {
public:
    VecodexSegment(std::unique_ptr<faiss::Index> index);

    VecodexSegment(const VecodexSegment&) = delete;

    VecodexSegment& operator=(const VecodexSegment&) = delete;

    VecodexSegment(VecodexSegment&&);

    void addVector(const std::string& id, const std::vector<float>& vector, const std::unordered_map<std::string, std::string>& attributes);
    
    // Mark search as const
    std::unordered_map<std::string, float> search(const std::vector<float>& query, int k) const;

    void mergeSegment(const VecodexSegment& other);

    size_t size() const {
        return metadata_.size();
    }


private:
    std::unique_ptr<faiss::Index> index_;
    std::vector<DocumentMetadata> metadata_;
};
