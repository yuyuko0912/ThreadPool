#include "ThreadPool.h"

#include <chrono>
#include <future>
#include <iostream>
#include <vector>

void runBenchmark(std::size_t threadCount, std::size_t taskCount)
{
    ThreadPool pool(threadCount);

    auto start = std::chrono::steady_clock::now();

    std::vector<std::future<int>> futures;
    futures.reserve(taskCount);

    for (std::size_t i = 0; i < taskCount; ++i)
    {
        futures.push_back(pool.submit([] {
            int result = 0;

            for (int k = 0; k < 1000000; ++k)
            {
                result += k;
            }

            return result;
        }));
    }

    long long sum = 0;

    for (auto& future : futures)
    {
        sum += future.get();
    }

    auto end = std::chrono::steady_clock::now();

    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            end - start
        );

    std::cout
        << "threads = " << threadCount
        << ", tasks = " << taskCount
        << ", time = " << duration.count() << " ms"
        << ", checksum = " << sum
        << std::endl;
}

int main()
{
    constexpr std::size_t taskCount = 10000;

    std::cout << "ThreadPool Benchmark\n";
    std::cout << "====================\n";

    // warm-up
    std::cout << "Warm-up\n";
    runBenchmark(1, 1000);
    std::cout << "====================\n";

    // benchmark
    runBenchmark(1, taskCount);
    runBenchmark(2, taskCount);
    runBenchmark(4, taskCount);
    runBenchmark(8, taskCount);
    runBenchmark(8, 1000);
    runBenchmark(8, 100);

    return 0;
}