#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t workers) {
    pool.emplace(workers);
}

ThreadPool::~ThreadPool() {
    pool.reset();
}

void ThreadPool::join() {
    pool->join();
}