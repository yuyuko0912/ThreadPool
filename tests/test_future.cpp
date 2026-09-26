#include "ThreadPool.h"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

int main()
{
    ThreadPool pool(4);

    // test1：返回值
    auto future1 = pool.submit([] {
        return 42;
    });

    assert(future1.get() == 42);

    // test2：带参数和返回值
    auto future2 = pool.submit(
        [](int a, int b) {
            return a * b;
        },
        6,7
    );

    assert(future2.get() == 42);

    // test3：void返回值
    bool executed = false;

    auto future3 = pool.submit([&executed] {
        executed = true;
    });

    future3.get();

    assert(executed);

    // test4：异常传播
    auto future4 = pool.submit([] {
        throw std::runtime_error("test exception");
    });

    bool exceptionCaught = false;

    try
    {
        future4.get();
    }
    catch (const std::runtime_error& e)
    {
        exceptionCaught = true;
        assert(std::string(e.what()) == "test exception");
    }

    assert(exceptionCaught);

    std::cout << "test_future passed." << std::endl;

    return 0;
}