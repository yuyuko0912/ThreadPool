#include "ThreadPool.h"

#include <atomic>
#include <cassert>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

// 多线程并发测试
int main()
{
    ThreadPool pool(4);

    constexpr int producerCount = 8;
    constexpr int tasksPerProducer = 100;
    constexpr int totalTasks =
        producerCount * tasksPerProducer;

    std::atomic<int> counter{0};

    std::vector<std::thread> producers;
    std::vector<std::vector<std::future<void>>> futures(
        producerCount
    );

    producers.reserve(producerCount);

    for (int i = 0; i < producerCount; ++i)
    {
        producers.emplace_back(
            [&pool, &counter, &futures, i] {
                for (int j = 0; j < tasksPerProducer; ++j)
                {
                    futures[i].push_back(
                        pool.submit([&counter] {
                            ++counter;
                        })
                    );
                }
            }
        );
    }

    for (auto& producer : producers)
    {
        producer.join();
    }

    for (auto& producerFutures : futures)
    {
        for (auto& future : producerFutures)
        {
            future.get();
        }
    }

    assert(counter.load() == totalTasks);

    std::cout << "test_concurrent passed." << std::endl;

    return 0;
}