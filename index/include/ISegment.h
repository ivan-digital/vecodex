#pragma once
#include <string>
#include <unordered_map>
#include <vector>
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

	virtual void serialize(const std::string& filename) const = 0;
};
}  // namespace vecodex