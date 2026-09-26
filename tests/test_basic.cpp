#include "ThreadPool.h"

#include <cassert> // assert()
#include <iostream>

int main(){
    ThreadPool pool(4);

    // test1，基本任务执行
    bool executed = false;
    auto future1 = pool.submit([&executed] {
        executed = true;
    });
    future1.get();
    assert(executed); // 判断任务是否被执行，若成功，继续运行。

    // test2，任务返回值
    auto future2 = pool.submit([] {
        return 42;
    });

    assert(future2.get() == 42);
    
    // test3，含参任务
    auto future3 = pool.submit(
        [](int a, int b) {
            return a + b;
        },
        10,
        20
    );
    assert(future3.get() == 30);

    
    std::cout << "test_basic passed." << std::endl;
    return 0;
}