#pragma once

#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>
namespace vecodex {
template <class IndexType>
class Segment : public IndexType {
   public:
	using IDType = typename IndexType::ID;
	template <typename... Args>
	Segment(Args... args) : IndexType(args...) {}

	Segment(IndexType&& index) : IndexType(std::move(index)) {
		seg_id_ = rand();
	}

	Segment(FILE* fd) : IndexType(fd) {
		std::fread(&seg_id_, sizeof(seg_id_), 1, fd);
	}

	Segment(const Segment&) = default;

	Segment& operator=(const Segment&) = default;

	Segment(Segment&& other) = default;

	void addVectorBatch(size_t n, const IDType* ids, const float* vectors) {
		IndexType::add_batch(n, vectors, ids);
	}

	void deleteBatch(size_t n, const IDType* ids) {
		IndexType::erase_batch(n, ids);
	}
	// Mark search as const
	std::unordered_map<IDType, float> search_query(
		const std::vector<float>& query, int k) const {
		std::vector<IDType> indices(k);
		std::vector<float> distances(k);
		size_t ans_k = IndexType::single_search(
			k, query.data(), distances.data(), indices.data());
		std::unordered_map<IDType, float> result;
		result.reserve(ans_k);
		for (size_t i = 0; i < ans_k; ++i) {
			result[indices[i]] = distances[i];
		}
		return result;
	}

	void mergeSegment(const std::shared_ptr<Segment>& other) {
		IndexType::merge_from_other(std::move(other->getIndex()));
	}
	IndexType& getIndex() { return *static_cast<IndexType*>(this); }
	const IndexType& getIndex() const {
		return *static_cast<const IndexType*>(this);
	}

	void serialize(const std::string& filename) const {
		FILE* fd = std::fopen(filename.c_str(), "w");
		IndexType::serialize(fd);
		std::fwrite(&seg_id_, sizeof(seg_id_), 1, fd);
		std::fclose(fd);
	}

	size_t size() const { return IndexType::size(); }
	size_t getID() const { return seg_id_; }

   private:
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
