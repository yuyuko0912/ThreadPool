#pragma once

// 多线程
#include <iostream>
#include <thread> // 工作线程
#include <mutex> // 锁
#include <condition_variable> // 线程等待/唤醒

#include <queue> // 存放待执行任务
#include <vector> // 存放多个线程
#include <functional> // 包装任务
#include <cstddef> // size_t 等基础类型

class ThreadPool{
public:
    // 含参构造函数
    explicit ThreadPool(std::size_t threadCount); // 创建含有threadCount个线程的线程池
    // 析构函数
    ~ThreadPool();

    // 禁用任何形式的拷贝构造函数
    ThreadPool(const ThreadPool&) = delete; 
    ThreadPool& operator=(const ThreadPool&) = delete; // 禁用 =  的重载。

private:
// 成员函数
    void worker(); // 线程拿取任务进行工作。

// 成员变量
    std::vector<std::thread> workers_; // 存储线程。
    std::queue<std::function<void()>> tasks_; // 存储任务，类型擦除。
    // 加锁
    std::mutex mutex_;
    // 无任务时休眠
    std::condition_variable condition_;

    bool stopping_; // 线程池是否关闭
};

