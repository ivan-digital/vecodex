#pragma once
namespace vecodex {
template <typename IDType>
class IBaseIndex {
   public:
	virtual void add_batch(size_t n, const float* vectors,
						   const IDType* ids) = 0;

	virtual size_t single_search(size_t k, const float* q, float* dist,
								 IDType* ids) const = 0;

	virtual void erase_batch(size_t n, const IDType* ids) = 0;

	virtual void merge_from_other(std::shared_ptr<IBaseIndex> other) = 0;

	virtual void serialize_index(FILE* fd) const = 0;

	virtual void serialize_index(const std::string& filename) const = 0;

	virtual size_t size() const = 0;
};
}  // namespace vecodex