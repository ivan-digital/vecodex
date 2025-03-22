#pragma once

#include <deque>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <vector>
#include "IBaseIndex.h"
#include "Segment.h"
#include "SegmentFactory.h"

namespace vecodex {
template <class IndexType>
class Index final : public IBaseIndex<typename IndexType::ID> {
   public:
	using UpdateCallback = std::function<void(
		std::vector<size_t>&&,
		std::vector<std::shared_ptr<const Segment<IndexType>>>&&)>;

	using IDType = typename IndexType::ID;
	template <typename... ArgTypes>
	Index(int dim, int segmentThreshold,
		  std::optional<UpdateCallback> update_callback, ArgTypes... args)
		: d_(dim),
		  segmentThreshold_(segmentThreshold),
		  factory_(std::make_shared<SegmentFactory<IndexType, ArgTypes...>>(
			  std::forward_as_tuple(args...))),
		  update_callback_(update_callback) {
		segments_.push_back(std::move(factory_->create()));
	}

	Index(int dim, int segmentThreshold,
		  std::optional<UpdateCallback> update_callback,
		  const std::shared_ptr<SegmentFactoryBase<IndexType>>& factory)
		: d_(dim),
		  segmentThreshold_(segmentThreshold),
		  factory_(factory),
		  update_callback_(update_callback) {
		segments_.push_back(std::move(factory_->create()));
	}

	void add(size_t n, const IDType* ids, const float* vectors) override {
		std::vector<std::shared_ptr<const Segment<IndexType>>> inserted;
		inserted.reserve((n / segmentThreshold_) + 1);
		size_t last = segments_.size() > 0 ? segments_.size() - 1 : 0;
		while (n) {
			size_t batch =
				std::min(n, segmentThreshold_ - segments_.back()->size());
			if (!batch) {
				throw std::runtime_error("Empty batch in adding");
			}
			segments_.back()->addVectorBatch(batch, ids, vectors);
			ids += batch;
			vectors += (batch * d_);
			n -= batch;
			if (segments_.back()->size() == segmentThreshold_) {
				segments_.push_back(std::move(factory_->create()));
			}
		}
		for (size_t i = last; i < segments_.size(); ++i) {
			if (segments_[i]->size() == segmentThreshold_) {
				inserted.push_back(segments_[i]);
			}
		}
		if (update_callback_) {
			update_callback_.value()({}, std::move(inserted));
		}
	}

	std::vector<IDType> search(const std::vector<float>& query,
							   int k) const override {
		std::set<SearchResult<IDType>> kth_stat;
		for (const auto& segment : segments_) {
			auto segmentResults = segment->search_query(query, k);
			for (const auto& p : segmentResults) {
				kth_stat.emplace(p.first, p.second);
				if (kth_stat.size() > k) {
					kth_stat.erase(*kth_stat.rbegin());
				}
			}
		}
		// Further logic to select top-k from combined results
		std::vector<IDType> results;
		for (const auto& vec : kth_stat) {
			results.push_back(vec.id);
		}
		return results;
	}

	void erase(size_t n, const IDType* ids) override {
		for (size_t i = 0; i < segments_.size(); ++i) {
			segments_[i]->deleteBatch(n, ids);
		}
	}

	void mergeSegments(size_t amount) override {
		if (amount > segments_.size()) {
			amount = segments_.size();
		}
		std::vector<std::shared_ptr<const Segment<IndexType>>> inserted;
		std::vector<size_t> erased;
		segments_.push_back(std::move(factory_->create()));
		for (size_t i = 0; i < amount; ++i) {
			erased.push_back(segments_.front()->getID());
			segments_.back()->mergeSegment(segments_.front());
			segments_.pop_front();
		}
		inserted.push_back(segments_.back());
		if (update_callback_.has_value()) {
			update_callback_.value()(std::move(erased), std::move(inserted));
		}
	}

	void push_segment(const std::shared_ptr<Segment<IndexType>>& segment) {
		segments_.push_back(segment);
		if (update_callback_.has_value()) {
			update_callback_.value()({}, {segment});
		}
	}
	size_t size() const { return segments_.size(); }

   private:
	int d_;
	size_t segmentThreshold_;
	std::shared_ptr<SegmentFactoryBase<IndexType>> factory_;
	std::deque<std::shared_ptr<Segment<IndexType>>> segments_;
	std::optional<UpdateCallback> update_callback_ = std::nullopt;
};
}  // namespace vecodex