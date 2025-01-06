#pragma once

#include <optional>
#include <functional>

#include <boost/asio.hpp>

class ThreadPool {
public:
    explicit ThreadPool(size_t workers);

    ~ThreadPool();

    template<class Function, class ...Args>
    void post(Function func, Args... args) {
        boost::asio::post(pool.value(), std::bind(func, args...));
    }

    void join();


private:
    std::optional<boost::asio::thread_pool> pool;
};