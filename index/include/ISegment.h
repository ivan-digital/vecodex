#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "IBaseIndex.h"
namespace vecodex {
template <typename IDType>
class ISegment {
   public:
	virtual void addVectorBatch(size_t n, const IDType* ids,
								const float* vectors) = 0;

	virtual std::unordered_map<IDType, float> searchQuery(
		const std::vector<float>& query, int k) const = 0;

	virtual void deleteBatch(size_t n, const IDType* ids) = 0;

	virtual size_t getID() const = 0;

	virtual std::shared_ptr<IBaseIndex<IDType>> getIndex() = 0;

	virtual void mergeSegment(
		const std::shared_ptr<ISegment<IDType>>& other) = 0;

	virtual void serialize(const std::string& filename) const = 0;

	virtual size_t size() const = 0;
};
}  // namespace vecodex