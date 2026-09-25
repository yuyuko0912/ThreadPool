#include "ThreadPool.h"

#include <cassert>
#include <chrono>
#include <stdexcept>


int main()
{
    ThreadPool pool(4);

    // 测试1：shutdown 前的任务正常执行
    auto future = pool.submit([] {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(100)
        );

        return 42;
    });

    assert(future.get() == 42);

    // 测试2：主动关闭线程池
    pool.shutdown();

    // 测试3：shutdown 后拒绝新任务
    bool exceptionCaught = false;

    try
    {
        pool.submit([] {
            return 100;
        });
    }
    catch (const std::runtime_error&)
    {
        exceptionCaught = true;
    }

    assert(exceptionCaught);

    std::cout << "test_shutdown passed." << std::endl;

    return 0;
}