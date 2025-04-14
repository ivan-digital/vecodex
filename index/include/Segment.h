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
class Segment final : public ISegment<typename IndexType::ID> {
   public:
	using IDType = typename IndexType::ID;
	template <typename... Args>
	Segment(Args... args)
		: ISegment<IDType>(IndexType::get_segment_type()),
		  index_(std::make_shared<IndexType>(args...)) {
		this->seg_id = rand();
	}

	Segment(IndexType&& index)
		: ISegment<IDType>(IndexType::get_segment_type()),
		  index_(std::move(index.index_)) {
		this->seg_id = rand();
	}

	Segment(FILE* fd)
		: ISegment<IDType>(IndexType::get_segment_type()),
		  index_(std::make_shared<IndexType>(fd)) {
		std::fread(&this->seg_id, sizeof(this->seg_id), 1, fd);
	}

	Segment(const Segment&) = default;

	Segment& operator=(const Segment&) = default;

	Segment(Segment&& other) = default;

	void addVectorBatch(size_t n, const IDType* ids,
						const float* vectors) override {
		index_->add_batch(n, vectors, ids);
	}

	void deleteBatch(size_t n, const IDType* ids) override {
		auto lock = this->aquireWrite();
		index_->erase_batch(n, ids);
	}
	// Mark search as const
	std::unordered_map<IDType, float> searchQuery(
		const std::vector<float>& query, int k) const override {
		auto lock = this->aquireRead();
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

	void mergeSegment(std::shared_ptr<ISegment<IDType>> other) override {
		auto other_ptr = std::dynamic_pointer_cast<Segment>(other);
		assert(other_ptr);
		auto other_lock = other->aquireWrite();
		auto current_lock = this->aquireWrite();
		index_->merge_from_other(other->getIndex());
	}
	std::shared_ptr<IBaseIndex<IDType>> getIndex() override { return index_; }

	void serialize(const std::string& filename) const override {
		auto lock = ISegment<typename IndexType::ID>::aquireRead();
		FILE* fd = std::fopen(filename.c_str(), "w");
		index_->serialize_index(fd);
		std::fwrite(&this->seg_id, sizeof(this->seg_id), 1, fd);
		std::fclose(fd);
	}

	void serialize(FILE* fd) const override {
		auto lock = ISegment<typename IndexType::ID>::aquireRead();
		index_->serialize_index(fd);
		std::fwrite(&this->seg_id, sizeof(this->seg_id), 1, fd);
	}

	size_t size() const override { return index_->size(); }
	size_t getID() const override { return this->seg_id; }

   private:
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
