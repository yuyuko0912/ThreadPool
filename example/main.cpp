#include "ThreadPool.h"
#include <iostream>

using namespace std;

int add(int a, int b)
{
    return a + b;
}

int main() {
    ThreadPool pool(4);
    auto future1 = pool.submit([]{ return 42;});
    auto future2 = pool.submit(add, 10, 20);
    auto future3 = pool.submit(
        [](int x){
            return x*x;
        },
        5
    );

    cout << future1.get() << '\n';
    cout << future2.get() << '\n';
    cout << future3.get() << '\n';
    return 0;
}