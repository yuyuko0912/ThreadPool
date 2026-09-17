#include "ThreadPool.h"
#include <iostream>
#include <thread>

using namespace std;
int main() {
    ThreadPool T(4);
    cout << "ThreadPool started.\n";
    this_thread::sleep_for(chrono::seconds(1)); // 当前线程睡眠一秒。
    cout << "ThreadPool finished.\n";
    return 0;
}