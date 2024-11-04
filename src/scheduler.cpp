#include "scheduler.hpp"

Scheduler::Scheduler(size_t amountThreads) : tasks_(), condvar_(), threads_(), mutex_(), stop_(false) {
    threads_.reserve(amountThreads);
    for (size_t i = 0; i < amountThreads; i++) {
        threads_.emplace_back(&Scheduler::thread_worker, this);
    }
}

Scheduler::~Scheduler() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_ = true;
    }

    condvar_.notify_all();
    for (auto &thread: threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void Scheduler::add(std::function<void()> task, std::time_t timestamp) {
    std::lock_guard<std::mutex> lock(mutex_);
    tasks_.emplace(std::move(task), timestamp);
    condvar_.notify_one();
}

void Scheduler::thread_worker() {
    while (true) {
        std::unique_lock<std::mutex> lock(mutex_);
        condvar_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });

        if (stop_ && tasks_.empty()) {
            return;
        }

        auto now = std::time(nullptr);
        while (!tasks_.empty() && tasks_.top().getTimestamp() <= now) {
            std::function<void()> task = tasks_.top().getTask();
            tasks_.pop();

            lock.unlock();
            task();
            lock.lock();
        }
    }
}
