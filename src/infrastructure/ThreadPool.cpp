#include "infrastructure/ThreadPool.h"
#include <iostream>

namespace Infrastructure
{

    ThreadPool::ThreadPool(size_t threads)
    {
        if (threads == 0)
            threads = 1;
        for (size_t i = 0; i < threads; ++i)
        {
            workers.emplace_back([this]
                                 {
            for (;;) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock, [this] {
                        return this->stop || !this->tasks.empty();
                    });
                    if (this->stop && this->tasks.empty())
                        return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                try {
                    task();
                } catch (...) {
                    // swallow exceptions to keep worker alive; logging can be added
                }
            } });
        }
    }

    ThreadPool::~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers)
        {
            if (worker.joinable())
                worker.join();
        }
    }

}