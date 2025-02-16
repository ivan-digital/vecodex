#pragma once

#include <deque>
#include <iostream>
#include <set>
#include <vector>
#include "Segment.h"
#include "SegmentFactory.h"
namespace vecodex {
template <class IndexType, class IDType, typename... ArgTypes>
class Index {
   public:
	Index(int dim, int segmentThreshold, std::tuple<ArgTypes...> args)
		: d_(dim),
		  segmentThreshold_(segmentThreshold),
		  factory_(std::move(args)) {
		factory_.push_segment(segments_);
	}

	void add(size_t n, const IDType* ids, const float* vectors) {
		while (n) {
			size_t batch =
				std::min(n, segmentThreshold_ - segments_.back().size());
			if (!batch) {
				throw std::runtime_error("Empty batch in adding");
			}
			segments_.back().addVectorBatch(batch, ids, vectors);
			ids += batch;
			vectors += (batch * d_);
			n -= batch;
			if (segments_.back().size() == segmentThreshold_) {
				factory_.push_segment(segments_);
			}
		}
	}

	std::vector<IDType> search(const std::vector<float>& query, int k) const {
		std::set<SearchResult<IDType>> kth_stat;
		for (const auto& segment : segments_) {
			auto segmentResults = segment.search_query(query, k);
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

	void erase(size_t n, const IDType* ids) {
		for (size_t i = 0; i < segments_.size(); ++i) {
			segments_[i].deleteBatch(n, ids);
		}
	}

	std::vector<size_t> mergeSegments(size_t amount) {
		if (amount > segments_.size()) {
			amount = segments_.size();
		}
		std::vector<size_t> erased_segments_;
		factory_.push_segment(segments_);
		for (size_t i = 0; i < amount; ++i) {
			erased_segments_.push_back(segments_.front().getID());
			segments_.back().mergeSegment(segments_.front());
			segments_.pop_front();
		}
		return erased_segments_;
	}
	size_t size() const { return segments_.size(); }

   private:
	int d_;
	size_t segmentThreshold_;
	SegmentFactory<IndexType, IDType, ArgTypes...> factory_;
	std::deque<Segment<IndexType, IDType>> segments_;
};
}  // namespace vecodex