#include "ThreadPool.h"

#include <atomic>
#include <cassert>


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
                            counter.fetch_add(1);
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