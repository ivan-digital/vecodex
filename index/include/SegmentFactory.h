#pragma once
#include "Segment.h"
#include "faiss/MetricType.h"
namespace vecodex {
template <typename IndexType>
class SegmentFactoryBase {
   public:
	virtual std::shared_ptr<Segment<IndexType>> create() const = 0;
};
template <class IndexType, typename... ArgTypes>
class SegmentFactory final : public SegmentFactoryBase<IndexType> {
   public:
	SegmentFactory(std::tuple<ArgTypes...>&& args) : args_(std::move(args)) {}
	std::shared_ptr<Segment<IndexType>> create() const override {
		auto create = [&](ArgTypes... args) {
			return std::make_shared<Segment<IndexType>>(args...);
		};
		return std::apply(create, args_);
	}

   private:
	std::tuple<ArgTypes...> args_;
};
}  // namespace vecodex