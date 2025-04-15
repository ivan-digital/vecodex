#pragma once
#include <stdio.h>
#include <map>
#include <memory>
#include <numeric>
#include <algorithm>
#include "IBaseIndex.h"
#include "SegmentFactory.h"
#include "faiss/IndexFlat.h"
#include "faiss/IndexHNSW.h"
#include "faiss/index_io.h"
#include "io.h"
namespace baseline {
template <class BaseIndex, typename IDType>
class FaissIndex final : public vecodex::IBaseIndex<IDType> {
   public:
	static int get_segment_type() {
		if constexpr (std::is_same<BaseIndex, faiss::IndexFlat>::value) {
			return vecodex::SegmentType::kFaissFlat;
		}
		return vecodex::SegmentType::kFaissHNSW;
	}
	using ID = IDType;
	template <typename... Args>
	FaissIndex(Args... args) : index_(std::make_shared<BaseIndex>(args...)) {}

	FaissIndex(FILE* fd) : index_((BaseIndex*)faiss::read_index(fd)) {
		readBinary(fd, next_id);
		size_t ids_sz = 0;
		readBinary(fd, ids_sz);
		for (size_t i = 0; i < ids_sz; ++i) {
			IDType k;
			faiss::idx_t v;
			readBinary(fd, k);
			readBinary(fd, v);
			ids_[k] = v;
		}
		size_t inv_ids_sz = 0;
		readBinary(fd, inv_ids_sz);
		for (size_t i = 0; i < inv_ids_sz; ++i) {
			faiss::idx_t k;
			IDType v;
			readBinary(fd, k);
			readBinary(fd, v);
			inv_ids_[k] = v;
		}
		size_t erased_sz = 0;
		readBinary(fd, erased_sz);
		for (size_t i = 0; i < erased_sz; ++i) {
			faiss::idx_t erased_id;
			readBinary(fd, erased_id);
			erased_.insert(erased_id);
		}
	}

	FaissIndex(const std::string& filename)
		: index_(faiss::read_index(filename.c_str())) {}

	void add_batch(size_t n, const float* vectors, const IDType* ids) override {
		index_->add(n, vectors);
		for (size_t i = 0; i < n; ++i) {
			ids_[ids[i]] = next_id;
			inv_ids_[next_id] = ids[i];
			next_id++;
		}
	}

	size_t single_search(size_t k, const float* q, float* dist,
						 IDType* ids) const override {
		std::vector<float> res_dist(k + erased_.size());
		std::vector<faiss::idx_t> res(k + erased_.size());
		std::cout << "single_search: " << res.size() << " " << erased_.size() << "\n";
		index_->search(1, q, res.size(), res_dist.data(), res.data());
		std::vector<std::pair<float, faiss::idx_t> > sorted_res;
		for (size_t i = 0; i < res.size(); ++i) {
			auto it = inv_ids_.find(res[i]);
			if (res[i] == -1 || it == inv_ids_.end() ||
				erased_.count(it->first)) {
				res[i] = -1;
				continue;
			}
			sorted_res.push_back({res_dist[i], res[i]});
		}
		std::sort(sorted_res.begin(), sorted_res.end());
		for (size_t i = 0; i < std::min(k, sorted_res.size()); ++i) {
			auto [key, value] = sorted_res[i];
			ids[i] = inv_ids_.at(value);
			dist[i] = key;
		}
		return std::min(k, sorted_res.size());
	}

	void merge_from_other(
		std::shared_ptr<vecodex::IBaseIndex<IDType>> other) override {
		auto other_index = std::dynamic_pointer_cast<FaissIndex>(other);
		if (other_index == nullptr) {
			throw std::runtime_error("Couldn't cast index into FaissIndex");
		}
		std::vector<faiss::idx_t> idxs;
		std::vector<IDType> ids;
		idxs.reserve(other_index->inv_ids_.size());
		for (const auto& [key, value] : other_index->inv_ids_) {
			if (other_index->erased_.count(key)) {
				continue;
			}
			ids.push_back(value);
			idxs.push_back(key);
		}
		std::vector<float> vectors(idxs.size() * index_->d);
		assert(ids.size() == idxs.size());

		other_index->index_->reconstruct_batch(idxs.size(), idxs.data(),
											   vectors.data());
		add_batch(ids.size(), vectors.data(), ids.data());
	}

	void erase_batch(size_t n, const IDType* ids) override {
		for (size_t i = 0; i < n; ++i) {
			if (!ids_.count(ids[i])) {
				continue;
			}
			erased_.insert(ids_[ids[i]]);
		}
	}

	void serialize_index(const std::string& filename) const override {
		FILE* fd = std::fopen(filename.c_str(), "w");
		serialize_index(fd);
		std::fclose(fd);
	}

	void serialize_index(FILE* fd) const override {
		faiss::write_index(index_.get(), fd);
		writeBinary(fd, next_id);
		writeBinary(fd, ids_.size());
		for (auto [k, v] : ids_) {
			writeBinary(fd, k);
			writeBinary(fd, v);
		}

		writeBinary(fd, inv_ids_.size());
		for (auto [k, v] : inv_ids_) {
			writeBinary(fd, k);
			writeBinary(fd, v);
		}

		writeBinary(fd, erased_.size());
		for (auto id : erased_) {
			writeBinary(fd, id);
		}
	}

	size_t size() const override { return ids_.size() - erased_.size(); }
	IDType getID(faiss::idx_t idx) const { return inv_ids_.at(idx); }

   private:
	std::shared_ptr<BaseIndex> index_;
	faiss::idx_t next_id = 0;
	std::unordered_map<IDType, faiss::idx_t> ids_;
	std::unordered_map<faiss::idx_t, IDType> inv_ids_;
	std::unordered_set<faiss::idx_t> erased_;
};

}  // namespace baseline
