#pragma once

#include <deque>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <vector>
#include "IIndex.h"
#include "Segment.h"
#include "SegmentFactory.h"

namespace vecodex {
template <class IndexType>
class Index final : public IIndex<typename IndexType::ID> {
   public:
	using IDType = typename IndexType::ID;
	using UpdateCallback =
	std::function<void(std::vector<size_t>&&,
					   std::vector<std::shared_ptr<ISegment<IDType>>>&&)>;

	template <typename... ArgTypes>
	Index(int dim, int segmentThreshold, ArgTypes... args)
		: IIndex<IDType>(),
		  d_(dim),
		  segmentThreshold_(segmentThreshold),
		  factory_(std::make_shared<SegmentFactory<IndexType, ArgTypes...>>(
			  std::forward_as_tuple(args...))) {
		this->segments_.push_back(std::move(factory_->create()));
	}

	Index(int dim, int segmentThreshold,
		  const std::shared_ptr<SegmentFactoryBase<IndexType>>& factory)
		: IIndex<IDType>(),
		  d_(dim),
		  segmentThreshold_(segmentThreshold),
		  factory_(factory) {
		this->segments_.push_back(std::move(factory_->create()));
	}

	void add(size_t n, const IDType* ids, const float* vectors) override {
		std::lock_guard lock(this->segments_m_);
		std::vector<std::shared_ptr<ISegment<IDType>>> inserted;
		inserted.reserve((n / segmentThreshold_) + 1);
		size_t last =
			this->segments_.size() > 0 ? this->segments_.size() - 1 : 0;
		while (n) {
			size_t batch =
				std::min(n, segmentThreshold_ - this->segments_.back()->size());
			if (!batch) {
				throw std::runtime_error("Empty batch in adding");
			}
			this->segments_.back()->addVectorBatch(batch, ids, vectors);
			ids += batch;
			vectors += (batch * d_);
			n -= batch;
			if (this->segments_.back()->size() == segmentThreshold_) {
				this->segments_.push_back(std::move(factory_->create()));
			}
		}
		for (size_t i = last; i < this->segments_.size(); ++i) {
			if (this->segments_[i]->size() == segmentThreshold_) {
				inserted.push_back(this->segments_[i]);
				this->storage_.add(this->segments_[i]);
			}
		}
		if (this->callback_ && !inserted.empty()) {
			this->callback_.value()({}, std::move(inserted));
		}
	}

	std::vector<IDType> search(const std::vector<float>& query,
							   int k) const override {
		std::shared_lock lock(this->segments_m_);
		std::set<SearchResult<IDType>> kth_stat;
		for (const auto& segment : this->segments_) {
			auto segmentResults = segment->searchQuery(query, k);
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
		std::lock_guard lock(this->segments_m_);
		for (size_t i = 0; i < this->segments_.size(); ++i) {
			this->segments_[i]->deleteBatch(n, ids);
		}
	}

	/*
	void mergeSegments(size_t amount) override {
		if (amount > this->segments_.size()) {
			amount = this->segments_.size();
		}
		std::vector<std::shared_ptr<ISegment<IDType>>> inserted;
		std::vector<size_t> erased;
		this->segments_.push_back(std::move(factory_->create()));
		for (size_t i = 0; i < amount; ++i) {
			erased.push_back(this->segments_.front()->getID());
			this->segments_.back()->mergeSegment(this->segments_.front());
			this->segments_.pop_front();
		}
		inserted.push_back(this->segments_.back());
		if (this->callback_.has_value()) {
			this->callback_.value()(std::move(erased), std::move(inserted));
		}
	}
	*/

	void pushSegment(
		const std::shared_ptr<ISegment<IDType>>& segment) override {
		std::lock_guard lock(this->segments_m_);
		this->storage_.add(segment);
		this->segments_.push_back(segment);
		if (this->callback_.has_value()) {
			this->callback_.value()({}, {segment});
		}
	}

	bool eraseSegment(size_t id) override {
		std::lock_guard lock(this->segments_m_);
		auto erase_it = std::find_if(
			this->segments_.begin(), this->segments_.end(),
			[&](const std::shared_ptr<const ISegment<IDType>>& segment)
				-> bool { return (segment->getID() == id); });
		if (erase_it == this->segments_.end()) {
			return false;
		}
		return true;
	}
	size_t size() const { return this->segments_.size(); }

	void setUpdateCallback(UpdateCallback&& callback) override {
		this->callback_ = std::move(callback);
	}

   private:
	int d_;
	size_t segmentThreshold_;
	std::shared_ptr<SegmentFactoryBase<IndexType>> factory_;
};
}  // namespace vecodex