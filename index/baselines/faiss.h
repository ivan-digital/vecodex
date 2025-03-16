#pragma once
#include <stdio.h>
#include <map>
#include <memory>
#include <numeric>
#include "SegmentFactory.h"
#include "faiss/IndexFlat.h"
#include "faiss/IndexHNSW.h"
#include "faiss/index_io.h"
namespace baseline {
template <class BaseIndex, typename IDType>
class FaissIndex {
   public:
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

	void add_batch(size_t n, const float* vectors, const IDType* ids) {
		index_->add(n, vectors);
		for (size_t i = 0; i < n; ++i) {
			ids_[ids[i]] = next_id;
			inv_ids_[next_id] = ids[i];
			next_id++;
		}
	}

	size_t single_search(size_t k, const float* q, float* dist,
						 IDType* ids) const {
		std::vector<float> res_dist(k + erased_.size());
		std::vector<faiss::idx_t> res(k + erased_.size());
		index_->search(1, q, res.size(), res_dist.data(), res.data());
		std::map<float, faiss::idx_t> sorted_res;
		for (size_t i = 0; i < res.size(); ++i) {
			auto it = inv_ids_.find(res[i]);
			if (res[i] == -1 || it == inv_ids_.end() ||
				erased_.count(it->first)) {
				res[i] = -1;
				continue;
			}
			sorted_res[res_dist[i]] = res[i];
			if (sorted_res.size() > k) {
				sorted_res.erase(sorted_res.rbegin()->first);
			}
		}
		size_t i = 0;
		for (const auto& [key, value] : sorted_res) {
			ids[i] = inv_ids_.at(value);
			dist[i] = key;
			i++;
		}
		return i;
	}

	void merge_from_other(FaissIndex&& other) {
		std::vector<faiss::idx_t> idxs;
		std::vector<IDType> ids;
		idxs.reserve(other.inv_ids_.size());
		for (const auto& [key, value] : other.inv_ids_) {
			if (other.erased_.count(key)) {
				continue;
			}
			ids.push_back(value);
			idxs.push_back(key);
		}
		std::vector<float> vectors(idxs.size());
		other.index_->reconstruct_batch(idxs.size(), idxs.data(),
										vectors.data());
		add_batch(ids.size(), vectors.data(), ids.data());
	}

	void erase_batch(size_t n, const IDType* ids) {
		for (size_t i = 0; i < n; ++i) {
			if (!ids_.count(ids[i])) {
				continue;
			}
			erased_.insert(ids_[ids[i]]);
		}
	}

	void serialize(const std::string& filename) const {
		FILE* fd = std::fopen(filename.c_str(), "w");
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
		std::fclose(fd);
	}

	size_t size() const { return ids_.size() - erased_.size(); }
	IDType getID(faiss::idx_t idx) const { return inv_ids_.at(idx); }

   private:
	template <typename T>
	static void writeBinary(FILE* fd, T value) {
		if constexpr (std::is_same_v<T, std::string>) {
			size_t sz = value.size();
			std::fwrite(&sz, sizeof(sz), 1, fd);
			std::fwrite(value.data(), sizeof(value[0]), sz, fd);
			return;
		}
		std::fwrite(&value, sizeof(value), 1, fd);
	}
	template <typename T>
	static void readBinary(FILE* fd, T& value) {
		if constexpr (std::is_same_v<T, std::string>) {
			size_t sz = 0;
			std::fread(&sz, sizeof(sz), 1, fd);
			value.resize(sz);
			std::fread(value.data(), sizeof(value[0]), sz, fd);
			return;
		}
		std::fread(&value, sizeof(value), 1, fd);
	}
	std::shared_ptr<BaseIndex> index_;
	faiss::idx_t next_id = 0;
	std::unordered_map<IDType, faiss::idx_t> ids_;
	std::unordered_map<faiss::idx_t, IDType> inv_ids_;
	std::unordered_set<faiss::idx_t> erased_;
};

}  // namespace baseline
