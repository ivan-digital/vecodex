#pragma once
#include <string>
#include <vector>
#include <functional>
#include "ISegment.h"
namespace vecodex {
template <typename IDType>
class IIndex {
   public:
	using UpdateCallback = std::function<void(
		std::vector<size_t>&&,
		std::vector<std::shared_ptr<const ISegment<IDType>>>&&)>;
	virtual void add(size_t n, const IDType* ids, const float* vectors) = 0;

	virtual std::vector<IDType> search(const std::vector<float>& query,
									   int k) const = 0;

	virtual void erase(size_t n, const IDType* ids) = 0;

	virtual void mergeSegments(size_t amount) = 0;

	virtual void pushSegment(
		const std::shared_ptr<ISegment<IDType>>& segment) = 0;

	virtual bool eraseSegment(size_t id) = 0;

	virtual void setUpdateCallback(UpdateCallback&& callback) = 0;
};
}  // namespace vecodex