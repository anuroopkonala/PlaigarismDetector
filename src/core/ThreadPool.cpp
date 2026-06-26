#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t numThreads)
{
    if (numThreads == 0)
        numThreads = 1;

    workers_.reserve(numThreads);

    for (size_t i = 0; i < numThreads; ++i)
    {
        workers_.emplace_back([this]
        {
            for (;;)
            {
                std::function<void()> task;

                {
                    std::unique_lock lock(mutex_);
                    cv_.wait(lock, [this]
                    {
                        return stop_ || !queue_.empty();
                    });

                    if (stop_ && queue_.empty())
                        return;

                    task = std::move(queue_.front());
                    queue_.pop();
                }

                task();
            }
        });
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock lock(mutex_);
        stop_ = true;
    }
    cv_.notify_all();

    for (auto& w : workers_)
        w.join();
}
