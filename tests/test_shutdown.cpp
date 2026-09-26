#include "ThreadPool.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>
using namespace std::chrono_literals; // 100ms


int main()
{
    ThreadPool pool(4);

    // test1：shutdown 前的任务正常执行
    auto future = pool.submit([] {
        std::this_thread::sleep_for(100ms);  // 线程睡眠100毫秒，制造任务运行中的时间窗口，认为拉长执行时间，让竞争窗口变宽，更容易触发可能存在的bug。
        return 42;
    });

    // test2：主动关闭线程池
    pool.shutdown();   // 按照刚刚设置的sleep，此时任务还在运行期间，判断是否运行完毕。。
    assert(future.get() == 42);

    // test3：shutdown 后拒绝新任务
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