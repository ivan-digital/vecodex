#pragma once
#include "Segment.h"
#include "faiss/MetricType.h"
namespace vecodex {
template <class IndexType, typename... ArgTypes>
class SegmentFactory {
   public:
	SegmentFactory(std::tuple<ArgTypes...>&& args) : args_(std::move(args)) {}
	inline void push_segment(
		std::deque<std::shared_ptr<Segment<IndexType>>>& data) const {
		auto create = [&](ArgTypes... args) {
			data.push_back(std::make_shared<Segment<IndexType>>(args...));
		};
		std::apply(create, args_);
	}

   private:
	std::tuple<ArgTypes...> args_;
};
}  // namespace vecodex