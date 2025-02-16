#include <map>
#include <numeric>
#include "SegmentFactory.h"
#include "faiss/IndexFlat.h"
#include "faiss/IndexHNSW.h"
namespace baseline {
template <class BaseIndex, typename IDType>
class FaissIndex : public BaseIndex {
   public:
	using ID = IDType;
	template <typename... Args>
	FaissIndex(Args... args) : BaseIndex(args...) {}

	void add_batch(size_t n, const float* vectors, const IDType* ids) {
		BaseIndex::add(n, vectors);
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
		BaseIndex::search(1, q, res.size(), res_dist.data(), res.data());
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
		other.getBase().reconstruct_batch(idxs.size(), idxs.data(),
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

	BaseIndex& getBase() { return *static_cast<BaseIndex*>(this); }

	size_t size() const { return ids_.size() - erased_.size(); }
	IDType getID(faiss::idx_t idx) const { return inv_ids_.at(idx); }

   private:
	faiss::idx_t next_id = 0;
	std::unordered_set<faiss::idx_t> erased_;
	std::unordered_map<IDType, faiss::idx_t> ids_;
	std::unordered_map<faiss::idx_t, IDType> inv_ids_;
};

}  // namespace baseline
