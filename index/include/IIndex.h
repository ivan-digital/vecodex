#pragma once
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>
#include <boost/thread/thread.hpp>
#include <boost/chrono.hpp>
#include <exception>
#include <functional>
#include <shared_mutex>
#include <string>
#include <vector>
#include "ISegment.h"
namespace vecodex {
template <typename IDType>
class IIndex {
   public:
	using UpdateCallback =
		std::function<void(std::vector<size_t>&&,
						   std::vector<std::shared_ptr<ISegment<IDType>>>&&)>;
	IIndex() {
		storage_.this_index = this;
		using namespace std::chrono_literals;
		merging_thread_ = std::make_shared<std::thread>([&]() {
			while (!storage_.stopped.load()) {
				storage_.merge(100);
				std::this_thread::sleep_for(1s);
			}
		});
	}
	~IIndex() {
		storage_.stopped.store(true);
		merging_thread_->join();
	}
	virtual void add(size_t n, const IDType* ids, const float* vectors) = 0;

	virtual std::vector<IDType> search(const std::vector<float>& query,
									   int k) const = 0;

	virtual void erase(size_t n, const IDType* ids) = 0;

	virtual void pushSegment(
		const std::shared_ptr<ISegment<IDType>>& segment) = 0;

	virtual bool eraseSegment(size_t id) = 0;

	virtual void setUpdateCallback(UpdateCallback&& callback) = 0;

	void StartExecutor(int thread_num) {
		execution_thread_ = std::make_shared<std::thread>([&]() {
			ctx_ = std::make_shared<boost::asio::io_context>(thread_num);
			ctx_->run();
		});
	}

	void StopExecution() {
		ctx_->stop();
		execution_thread_->join();
		execution_thread_.reset();
	}

	boost::unique_future<bool> async_add(size_t n, const IDType* ids,
										 const float* vectors) {
		if (!ctx_) {
			throw std::runtime_error("Execution was not started");
		}
		std::shared_ptr<boost::promise<bool>> promise_ptr;
		boost::unique_future<bool> f(promise_ptr->get_future());
		boost::asio::post(
			*ctx_, [n = n, ids = ids, vectors = vectors,
					promise_ptr = std::move(promise_ptr), index = this]() {
				index->add(n, ids, vectors);
				promise_ptr->set_value(true);
			});
		return f;
	}

	boost::unique_future<bool> async_erase(size_t n, const IDType* ids) {
		if (!ctx_) {
			throw std::runtime_error("Execution was not started");
		}
		std::shared_ptr<boost::promise<bool>> promise_ptr;
		boost::unique_future<bool> f(promise_ptr->get_future());
		boost::asio::post(
			*ctx_, [n = n, ids = ids, promise_ptr = std::move(promise_ptr),
					index = this]() {
				index->erase(n, ids);
				promise_ptr->set_value(true);
			});
		return f;
	}

	boost::unique_future<std::vector<IDType>> async_search(
		const std::vector<float>& query, int k) {
		if (!ctx_) {
			throw std::runtime_error("Execution was not started");
		}
		std::shared_ptr<boost::promise<std::vector<IDType>>> promise_ptr;
		auto f(promise_ptr->get_future());
		boost::asio::post(
			*ctx_, [query = std::move(query), k = k,
					promise_ptr = std::move(promise_ptr), index = this]() {
				auto out = index->search(query, k);
				promise_ptr->set_value(std::move(out));
			});
		return f;
	}

   protected:
	struct SegmentStorage {
		std::vector<std::vector<std::shared_ptr<ISegment<IDType>>>> levels;
		std::atomic_bool stopped = false;
		IIndex* this_index;
		void add(std::shared_ptr<ISegment<IDType>> segment) {
			size_t i = 0;
			for (; (1ll << i) < segment->size(); ++i) {
				if (i == levels.size()) {
					levels.push_back({});
				}
			}
			i = i == 0 ? 0 : i - 1;
			if (i == levels.size()) {
				levels.push_back({});
			}
			levels[i].push_back(segment);
		}
		void merge(size_t amount) {
			std::lock_guard lock(this_index->segments_m_);
			for (size_t i = 0; amount > 0 && i < levels.size(); ++i) {
				std::vector<size_t> erased;
				std::vector<std::shared_ptr<ISegment<IDType>>> inserted;
				for (size_t j = 0; j + 1 < levels[i].size(); ++j) {
					if (!levels[i][j] || !levels[i][j + 1] ||
						levels[i][j]->size() + levels[i][j + 1]->size() >
							amount) {
						continue;
					}
					amount -= levels[i][j]->size() + levels[i][j + 1]->size();
					erased.push_back(levels[i][j + 1]->getID());
					erased.push_back(levels[i][j]->getID());
					levels[i][j]->mergeSegment(levels[i][j + 1]);
					levels[i][j]->seg_id = rand();
					auto index = std::move(levels[i][j]);
					if (i + 1 == levels.size()) {
						levels.push_back({});
					}
					levels[i + 1].push_back(index);
					levels[i][j].reset();
					levels[i][j + 1].reset();
					inserted.push_back(index);
				}
				for (auto&& segment : inserted) {
					this_index->segments_.push_back(segment);
				}
				for (auto&& id : erased) {
					auto erase_it = std::find_if(
						this_index->segments_.begin(),
						this_index->segments_.end(),
						[&](const std::shared_ptr<const ISegment<IDType>>&
								segment) -> bool {
							return (segment->getID() == id);
						});
					assert(erase_it != this_index->segments_.end());
					this_index->segments_.erase(erase_it);
				}
				if (this_index->callback_.has_value()) {
					this_index->callback_.value()(std::move(erased),
												  std::move(inserted));
				}
				erased.clear();
				inserted.clear();
			}
			for (size_t i = 0; i < levels.size(); ++i) {
				if (levels[i].empty()) {
					continue;
				}
				levels[i].erase(
					std::remove_if(
						levels[i].begin(), levels[i].end(),
						[](const std::shared_ptr<ISegment<IDType>>& segment) {
							return !segment;
						}),
					levels[i].end());
			}
		}
	};
	SegmentStorage storage_;
	std::optional<UpdateCallback> callback_;
	std::shared_ptr<std::thread> execution_thread_;
	std::shared_ptr<std::thread> merging_thread_;
	std::shared_ptr<boost::asio::io_context> ctx_;
	std::vector<std::shared_ptr<ISegment<IDType>>> segments_;
	mutable std::shared_mutex segments_m_;
};
}  // namespace vecodex