#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class Scheduler final {
private:
    class Task {
    private:
        std::function<void()> task_;
        std::time_t timestamp_;

    public:
        Task(std::function<void()> task, std::time_t timestamp) : task_(std::move(task)), timestamp_(timestamp) {}

        std::time_t getTimestamp() const {
            return timestamp_;
        }

        std::function<void()> getTask() const {
            return task_;
        }

        bool operator<(const Task &other) const {
            return timestamp_ > other.timestamp_; // use for priority_queue
        }
    };

    std::priority_queue<Task, std::vector<Task>> tasks_;
    std::condition_variable condvar_;
    std::vector<std::thread> threads_;
    std::mutex mutex_;
    bool stop_;

public:
    explicit Scheduler(size_t amountThread);

    ~Scheduler();

    Scheduler(const Scheduler &other) = delete;

    Scheduler(Scheduler &&other) noexcept = delete;

    Scheduler &operator=(const Scheduler &other) = delete;

    Scheduler &operator=(Scheduler &&other) noexcept = delete;

    void add(std::function<void()> task, std::time_t timestamp);

    void thread_worker();
};

#endif // SCHEDULER_HPP
