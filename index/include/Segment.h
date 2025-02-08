#pragma once

#include <IndexConfig.h>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>
namespace vecodex {
template <class IndexType, class IDType>
class Segment {
   public:
	Segment(const IndexConfig<IndexType>& config,
			std::unique_ptr<IndexType>&& index)
		: config_(config), index_(std::move(index)) {
		seg_id_ = rand();
	}

	Segment(size_t id, const IndexConfig<IndexType>& config,
			std::unique_ptr<IndexType>&& index)
		: config_(config), index_(std::move(index)), seg_id_(id) {}

	Segment(const Segment&) = delete;

	Segment& operator=(const Segment&) = delete;

	Segment(Segment&& other)
		: config_(std::move(other.config_)),
		  index_(std::move(other.index_)),
		  seg_id_(std::move(other.seg_id_)) {}

	void addVectorBatch(size_t n, const IDType* ids, const float* vectors) {
		config_.getAdd()(index_, n, vectors, ids);
	}

	void deleteBatch(size_t n, const IDType* ids) {
		if (!config_.hasDelete()) {
			throw std::runtime_error("No delete function for Index Segment");
		}
		config_.getDelete().value()(index_, n, ids);
	}
	// Mark search as const
	std::unordered_map<IDType, float> search(const std::vector<float>& query,
											 int k) const {
		std::vector<IDType> indices(k);
		std::vector<float> distances(k);
		size_t ans_k = config_.getSearch()(index_, k, query.data(),
										   distances.data(), indices.data());
		std::unordered_map<IDType, float> result;
		result.reserve(ans_k);
		for (size_t i = 0; i < ans_k; ++i) {
			result[indices[i]] = distances[i];
		}
		return result;
	}

	void mergeSegment(Segment&& other) {
		config_.getMerge()(index_, std::move(other.index_));
	}

	const std::unique_ptr<IndexType>& getIndex() const { return index_; }

	size_t size() const { return index_->size(); }
	size_t getID() const { return seg_id_; }

   private:
	size_t sz_ = 0;
	IndexConfig<IndexType> config_;
	std::unique_ptr<IndexType> index_;
	size_t seg_id_;
};
template <class IDType>
struct SearchResult {
	SearchResult(IDType id_value, float dist_value)
		: dist(dist_value), id(id_value) {}
	bool operator==(const SearchResult& other) const { return id == other.id; }
	bool operator<(const SearchResult& other) const {
		return dist < other.dist || (dist == other.dist && id < other.id);
	}
	bool operator>(const SearchResult& other) const {
		return dist > other.dist || (dist == other.dist && id > other.id);
	}
	float dist;
	IDType id;
};
}  // namespace vecodex
