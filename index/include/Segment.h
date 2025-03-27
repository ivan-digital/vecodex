#pragma once

#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>
#include "IBaseIndex.h"
#include "ISegment.h"
namespace vecodex {
template <class IndexType>
class Segment final : public vecodex::ISegment<typename IndexType::ID> {
   public:
	using IDType = typename IndexType::ID;
	template <typename... Args>
	Segment(Args... args) : index_(std::make_shared<IndexType>(args...)) {}

	Segment(IndexType&& index) : index_(std::move(index.index_)) {
		seg_id_ = rand();
	}

	Segment(FILE* fd) : index_(std::make_shared<IndexType>(fd)) {
		std::fread(&seg_id_, sizeof(seg_id_), 1, fd);
	}

	Segment(const Segment&) = default;

	Segment& operator=(const Segment&) = default;

	Segment(Segment&& other) = default;

	void addVectorBatch(size_t n, const IDType* ids,
						const float* vectors) override {
		index_->add_batch(n, vectors, ids);
	}

	void deleteBatch(size_t n, const IDType* ids) override {
		index_->erase_batch(n, ids);
	}
	// Mark search as const
	std::unordered_map<IDType, float> searchQuery(
		const std::vector<float>& query, int k) const override {
		std::vector<IDType> indices(k);
		std::vector<float> distances(k);
		size_t ans_k = index_->single_search(k, query.data(), distances.data(),
											 indices.data());
		std::unordered_map<IDType, float> result;
		result.reserve(ans_k);
		for (size_t i = 0; i < ans_k; ++i) {
			result[indices[i]] = distances[i];
		}
		return result;
	}

	void mergeSegment(const std::shared_ptr<Segment>& other) {
		index_->merge_from_other(std::move(other->getIndex()));
	}
	std::shared_ptr<IBaseIndex<IDType>> getIndex() { return index_; }

	void serialize(const std::string& filename) const override {
		FILE* fd = std::fopen(filename.c_str(), "w");
		index_->serialize_index(fd);
		std::fwrite(&seg_id_, sizeof(seg_id_), 1, fd);
		std::fclose(fd);
	}

	size_t size() const { return index_->size(); }
	size_t getID() const override { return seg_id_; }

   private:
	size_t seg_id_;
	std::shared_ptr<IBaseIndex<IDType>> index_;
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
