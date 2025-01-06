#pragma once
#include <unordered_map>
#include <string>
#include <functional>
#include <memory>

#include "faiss/MetricType.h"


template<class Index> 
class IndexConfig {
public:
    using AddFuncType = std::function<void(const std::unique_ptr<Index>&, size_t, const float*)>;
    using SearchFuncType = std::function<void(const std::unique_ptr<Index>&, size_t, const float*, float*, size_t*)>;
    using MergeFuncType = std::function<void(const std::unique_ptr<Index>&, std::unique_ptr<Index>&&)>;
    IndexConfig(AddFuncType add, SearchFuncType search, MergeFuncType merge): add_(add), search_(search), merge_(merge) {}
    AddFuncType getAdd() const {
        return add_;
    }

    SearchFuncType getSearch() const {
        return search_;
    }

    MergeFuncType getMerge() const {
        return merge_;
    }
private:
    AddFuncType add_;
    SearchFuncType search_;
    MergeFuncType merge_;
};