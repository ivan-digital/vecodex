#pragma once
#include "IndexConfig.h"
#include "Segment.h"
#include "faiss/MetricType.h"
namespace vecodex {
template <class IndexType, class IDType, typename... ArgTypes>
class SegmentFactory {
   public:
	SegmentFactory(std::tuple<ArgTypes...>&& args) : args_(std::move(args)) {}
	Segment<IndexType, IDType> create(IndexConfig<IndexType> config) const {
		std::unique_ptr<IndexType> ptr =
			std::apply(create_ptr<IndexType>, args_);
		return vecodex::Segment<IndexType, IDType>(config, std::move(ptr));
	}

   private:
	template <class T>
	static std::unique_ptr<T> create_ptr(ArgTypes... args) {
		return std::make_unique<T>(args...);
	}
	std::tuple<ArgTypes...> args_;
};
}  // namespace vecodex