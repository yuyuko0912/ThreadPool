#include "ThreadPool.h"

#include <cassert>
#include <future>
#include <iostream>
#include <mutex>
#include <vector>

int main()
{
    ThreadPool pool(1); // 单个线程，防止任务被分散到各个线程无法判断是否被优先取出。

    std::vector<int> executionOrder;
    std::mutex mutex;

    std::promise<void> startPromise;
    std::shared_future<void> startFuture =
        startPromise.get_future().share();

    // 阻塞任务
    auto blocker = pool.submit([startFuture] {
        startFuture.wait();
    });

    // 此时工作线程被 blocker 占住，
    // 下面三个任务都会留在 priority_queue 中。

    auto future1 = pool.submitWithPriority(
        1,
        [&executionOrder, &mutex] {
            std::lock_guard<std::mutex> lock(mutex);
            executionOrder.push_back(1);
        }
    );

    auto future2 = pool.submitWithPriority(
        10,
        [&executionOrder, &mutex] {
            std::lock_guard<std::mutex> lock(mutex);
            executionOrder.push_back(10);
        }
    );

    auto future3 = pool.submitWithPriority(
        5,
        [&executionOrder, &mutex] {
            std::lock_guard<std::mutex> lock(mutex);
            executionOrder.push_back(5);
        }
    );

    // 释放工作线程
    startPromise.set_value();

    blocker.get();
    future1.get();
    future2.get();
    future3.get();

    assert(executionOrder.size() == 3);

    assert(executionOrder[0] == 10);
    assert(executionOrder[1] == 5);
    assert(executionOrder[2] == 1);
    // 被执行时机是否与优先级大小一致。

    std::cout << "test_priority passed." << std::endl;

    return 0;
}