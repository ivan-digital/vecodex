#pragma once
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include "faiss/MetricType.h"
namespace vecodex {
template <class IndexType>
class IndexConfig {
   public:
	using AddFuncType = std::function<void(const std::unique_ptr<IndexType>&,
										   size_t, const float*)>;
	using SearchFuncType =
		std::function<void(const std::unique_ptr<IndexType>&, size_t,
						   const float*, float*, size_t*)>;
	using MergeFuncType = std::function<void(const std::unique_ptr<IndexType>&,
											 std::unique_ptr<IndexType>&&)>;
	using DeleteFuncType = std::function<void(const std::unique_ptr<IndexType>&,
											  size_t, const size_t*)>;
	IndexConfig(AddFuncType add_func, SearchFuncType search_func,
				MergeFuncType merge_func,
				std::optional<DeleteFuncType> delete_func = std::nullopt)
		: add_(add_func),
		  search_(search_func),
		  merge_(merge_func),
		  delete_(delete_func) {}
	AddFuncType getAdd() const { return add_; }

	SearchFuncType getSearch() const { return search_; }

	MergeFuncType getMerge() const { return merge_; }

	bool hasDelete() const { return delete_.has_value(); }

	std::optional<DeleteFuncType> getDelete() const { return delete_; }

   private:
	AddFuncType add_;
	SearchFuncType search_;
	MergeFuncType merge_;
	std::optional<DeleteFuncType> delete_;
};
}  // namespace vecodex