#pragma once
#include <deque>
#include <list>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include "IBaseIndex.h"
namespace vecodex {
enum SegmentType { kFaissFlat = 0, kFaissHNSW };
template <typename IDType>
class ISegment {
   public:
	ISegment(int seg_type_) : seg_type(seg_type_) {}

	virtual void addVectorBatch(size_t n, const IDType* ids,
								const float* vectors) = 0;

	virtual std::unordered_map<IDType, float> searchQuery(
		const std::vector<float>& query, int k) const = 0;

	virtual void deleteBatch(size_t n, const IDType* ids) = 0;

	virtual size_t getID() const { return seg_id; };

	virtual std::shared_ptr<IBaseIndex<IDType>> getIndex() = 0;

	virtual void mergeSegment(std::shared_ptr<ISegment<IDType>> other) = 0;

	virtual void serialize(const std::string& filename) const = 0;

	virtual void serialize(FILE* fd) const = 0;

	virtual size_t size() const = 0;

	void lock() const { m_.lock(); }
	void unlock() const { m_.unlock(); }

	void readerLock() const { m_.lock_shared(); }
	void readerUnlock() const { m_.unlock_shared(); }

	void writerLock() const { m_.lock(); }
	void writerUnlock() const { m_.unlock(); }

	inline std::lock_guard<std::shared_mutex> aquireWrite() const {
		return std::lock_guard(m_);
	}
	inline std::shared_lock<std::shared_mutex> aquireRead() const {
		return std::shared_lock(m_);
	}

	size_t seg_id;
	int seg_type = 0;

   private:
	mutable std::shared_mutex m_;
};
}  // namespace vecodex