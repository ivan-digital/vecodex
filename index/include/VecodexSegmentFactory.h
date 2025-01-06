#pragma once
#include "VecodexSegment.h"
#include "faiss/MetricType.h"
#include "IndexConfig.h"
template<class Index, typename... ArgTypes>
class VecodexSegmentFactory {
public:
    VecodexSegmentFactory(std::tuple<ArgTypes...>&& args) : args_(std::move(args)) {}
    VecodexSegment<Index> create(IndexConfig<Index> config) const {
        std::unique_ptr<Index> ptr = std::apply(create_ptr<Index>, args_);
        return VecodexSegment<Index>(config, std::move(ptr));
    }
private:
    template<class T>
    static std::unique_ptr<T> create_ptr(ArgTypes... args) {
        return std::make_unique<T>(args...);
    }
    std::tuple<ArgTypes...> args_;
};
