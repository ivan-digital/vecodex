#ifndef VECODEX_SEGMENT_H
#define VECODEX_SEGMENT_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <faiss/IndexFlat.h>
#include "DocumentMetadata.h"

class VecodexSegment {
public:
    VecodexSegment(int dimension) : index(new faiss::IndexFlatL2(dimension)) {}

    void addVector(const std::string& id, const std::vector<float>& vector, const std::unordered_map<std::string, std::string>& attributes);
    
    // Mark search as const
    std::vector<std::string> search(const std::vector<float>& query, int k) const;

    void mergeSegment(const VecodexSegment& other);

    size_t size() const {
        return metadata.size();
    }

private:
    std::unique_ptr<faiss::IndexFlatL2> index;
    std::vector<DocumentMetadata> metadata;
};


#endif // VECODEX_SEGMENT_H
