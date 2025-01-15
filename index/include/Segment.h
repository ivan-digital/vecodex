#pragma once

#include <IndexConfig.h>
#include <faiss/IndexFlat.h>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
using IDType = std::string;
namespace vecodex {
template <class IndexType>
class Segment {
   public:
	Segment(const IndexConfig<IndexType>& config,
			std::unique_ptr<IndexType>&& index)
		: config_(config), index_(std::move(index)) {}

	Segment(const Segment&) = delete;

	Segment& operator=(const Segment&) = delete;

	Segment(Segment&& other)
		: config_(std::move(other.config_)),
		  index_(std::move(other.index_)),
		  ids_(std::move(other.ids_)) {}

	void addVectorBatch(size_t n, const IDType* ids, const float* vectors) {
		std::copy_n(ids, n, std::back_inserter(ids_));
		config_.getAdd()(index_, n, vectors);
	}
	// Mark search as const
	std::unordered_map<std::string, float> search(
		const std::vector<float>& query, int k) const {
		std::vector<faiss::idx_t> indices(k);
		std::vector<float> distances(k);
		config_.getSearch()(index_, k, query.data(), distances.data(),
							(size_t*)indices.data());
		std::unordered_map<IDType, float> result;
		result.reserve(k);
		for (size_t i = 0; i < indices.size(); ++i) {
			if (indices[i] == -1) {
				continue;
			}
			result[ids_[indices[i]]] = distances[i];
		}
		return result;
	}

	void mergeSegment(Segment&& other) {
		std::copy_n(other.ids_.begin(), other.ids_.size(),
					std::back_inserter(ids_));
		config_.getMerge()(index_, std::move(other.index_));
	}

	const IDType* getIDs() const { return ids_.data(); }

	const std::unique_ptr<IndexType>& getIndex() const { return index_; }

	size_t size() const { return ids_.size(); }

   private:
	IndexConfig<IndexType> config_;
	std::unique_ptr<IndexType> index_;
	std::vector<IDType> ids_;
};

struct SearchResult {
	SearchResult(IDType id_value, float dist_value)
		: dist(dist_value), id(id_value) {}
	bool operator==(const SearchResult& other) const;
	bool operator<(const SearchResult& other) const;
	bool operator>(const SearchResult& other) const;
	float dist;
	IDType id;
};
}  // namespace vecodex