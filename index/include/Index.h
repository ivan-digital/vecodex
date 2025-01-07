#pragma once

#include <iostream>
#include <set>
#include <vector>
#include "Segment.h"
#include "SegmentFactory.h"
namespace vecodex {
template <class IndexType, typename... ArgTypes>
class Index {
   public:
	Index(int dim, int segmentThreshold, IndexConfig<IndexType> config,
		  std::tuple<ArgTypes...> args)
		: d_(dim),
		  segmentThreshold_(segmentThreshold),
		  config_(config),
		  factory_(std::move(args)) {
		segments_.push_back(factory_.create(config_));
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
				segments_.push_back(factory_.create(config_));
			}
		}
	}

	std::vector<IDType> search(const std::vector<float>& query, int k) {
		std::set<SearchResult> kth_stat;
		for (const auto& segment : segments_) {
			auto segmentResults = segment.search(query, k);
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

	void updateVector(const IDType& id, const std::vector<float>& vector);
	void mergeSegments() {
		if (segments_.size() == 1) {
			return;
		}
		vecodex::Segment mergedSegment(factory_.create(config_));
		for (auto&& segment : segments_) {
			mergedSegment.mergeSegment(std::move(segment));
		}
		segments_.clear();
		segments_.push_back(std::move(mergedSegment));
	}

   private:
	int d_;
	size_t segmentThreshold_;
	IndexConfig<IndexType> config_;
	SegmentFactory<IndexType, ArgTypes...> factory_;
	std::vector<Segment<IndexType>> segments_;
};
}  // namespace vecodex