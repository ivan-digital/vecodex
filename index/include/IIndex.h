#pragma once
#include <string>
#include <vector>
namespace vecodex {
template <typename IDType>
class IIndex {
   public:
	virtual void add(size_t n, const IDType* ids, const float* vectors) = 0;

	virtual std::vector<IDType> search(const std::vector<float>& query,
									   int k) const = 0;

	virtual void erase(size_t n, const IDType* ids) = 0;

	virtual void mergeSegments(size_t amount) = 0;
};
}  // namespace vecodex