#include "../src/scheduler.hpp"

#include "doctest.h"
#include <iostream>
#include <atomic>

TEST_CASE("correct time") {
    time_t time1 = std::time(nullptr) + 1;
    time_t check_time1 = std::time(nullptr);

    time_t time2 = std::time(nullptr) + 5;
    time_t check_time2 = std::time(nullptr);

    time_t time3 = std::time(nullptr) + 10;
    time_t check_time3 = std::time(nullptr);

    {
        Scheduler scheduler(2);

        scheduler.add([&check_time1]() {
            check_time1 = std::time(nullptr);
        }, std::time(nullptr) + 1);

        scheduler.add([&check_time2]() {
            check_time2 = std::time(nullptr);
        }, std::time(nullptr) + 5);

        scheduler.add([&check_time3]() {
            check_time3 = std::time(nullptr);
        }, std::time(nullptr) + 10);
    }

    CHECK_LE(time1, check_time1);
    CHECK_LE(time2, check_time2);
    CHECK_LE(time3, check_time3);
}

TEST_CASE("correct order") {
    time_t check_time1 = std::time(nullptr);

    time_t check_time2 = std::time(nullptr);

    time_t check_time3 = std::time(nullptr);

    {
        Scheduler scheduler(3);

        scheduler.add([&check_time1]() {
            check_time1 = std::time(nullptr);
        }, std::time(nullptr) + 1);

        scheduler.add([&check_time2]() {
            check_time2 = std::time(nullptr);
        }, std::time(nullptr) + 2);

        scheduler.add([&check_time3]() {
            check_time3 = std::time(nullptr);
        }, std::time(nullptr) + 3);
    }

    CHECK_LE(check_time1, check_time2);
    CHECK_LE(check_time3, check_time3);
}

TEST_CASE("correct number of tasks") {
    std::atomic<int> taskCount(0);
    {
        Scheduler scheduler(2);

        auto task = [&taskCount]() {
            taskCount++;
        };

        std::vector<std::thread> threads;
        threads.reserve(10);
        for (int i = 0; i < 10; ++i) {
            threads.emplace_back([&scheduler, task]() {
                for (int j = 0; j < 10; ++j) {
                    scheduler.add(task, std::time(nullptr) + j);
                }
            });
        }

        for (auto &thread: threads) {
            thread.join();
        }
    }
    CHECK_EQ(taskCount.load(), 100);
}

TEST_CASE("data race") {
    std::atomic<int> sharedCounter(0);
    Scheduler scheduler(4);

    auto incrementTask = [&sharedCounter]() {
        for (int i = 0; i < 1000; ++i) {
            sharedCounter.fetch_add(1);
        }
    };

    for (int i = 0; i < 10; ++i) {
        scheduler.add(incrementTask, std::time(nullptr));
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));

    CHECK_EQ(sharedCounter.load(), 10000);
}

