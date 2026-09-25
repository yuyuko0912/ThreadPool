#include "ThreadPool.h"

#include <chrono>


void runBenchmark(std::size_t threadCount, int taskCount)
{
    ThreadPool pool(threadCount);

    auto start = std::chrono::steady_clock::now();

    std::vector<std::future<void>> futures;
    futures.reserve(taskCount);

    for (int i = 0; i < taskCount; ++i)
    {
        futures.push_back(
            pool.submit([] {
                volatile int result = 0;

                for (int i = 0; i < 1000; ++i)
                {
                    result += i;
                }
            })
        );
    }

    for (auto& future : futures)
    {
        future.get();
    }

    auto end = std::chrono::steady_clock::now();

    auto duration =
        std::chrono::duration_cast<
            std::chrono::milliseconds
        >(end - start);

    std::cout
        << "threads = " << threadCount
        << ", tasks = " << taskCount
        << ", time = " << duration.count()
        << " ms"
        << std::endl;
}

int main()
{
    constexpr int taskCount = 10000;

    std::cout << "ThreadPool Benchmark\n";
    std::cout << "====================\n";

    runBenchmark(1, taskCount);
    runBenchmark(2, taskCount);
    runBenchmark(4, taskCount);
    runBenchmark(8, taskCount);

    return 0;
}