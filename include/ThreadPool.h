#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>



class ThreadPool
{
public:
    explicit ThreadPool(size_t numThreads);
    ~ThreadPool();

    
    template <typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>
    {
        using R = std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<R()>>(
            [f = std::forward<F>(f),
             args = std::make_tuple(std::forward<Args>(args)...)]() mutable
            {
                return std::apply(std::move(f), std::move(args));
            });

        std::future<R> fut = task->get_future();

        {
            std::unique_lock lock(mutex_);
            if (stop_)
                throw std::runtime_error("ThreadPool is stopped");
            queue_.emplace([task]{ (*task)(); });
        }

        cv_.notify_one();
        return fut;
    }

    size_t size() const { return workers_.size(); }

private:
    std::vector<std::thread>          workers_;
    std::queue<std::function<void()>> queue_;
    std::mutex                        mutex_;
    std::condition_variable           cv_;
    bool                              stop_ = false;
};
