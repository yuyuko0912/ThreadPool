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
#include <utility> // move()
#include <future> // 提供异步编程、实现线程间传递
#include <type_traits> // 自动推导返回类型等
#include <stdexcept> // 异常类，报告“逻辑错误”与“运行时错误”


class ThreadPool{
public:
    // 含参构造函数
    explicit ThreadPool(std::size_t threadCount); // 创建含有threadCount个线程的线程池
    // 析构函数
    ~ThreadPool();

    // 禁用任何形式的拷贝构造函数
    ThreadPool(const ThreadPool&) = delete; 
    ThreadPool& operator=(const ThreadPool&) = delete; // 禁用 =  的重载。

    // 对外接口，获取并包装任务
    template <class F,class... Args> 
    auto submit(F&& f,Args&&... args)
        -> std::future<std::invoke_result_t<F,Args...>>; // 尾置返回类型，返回值类型根据参数推导出。

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

template <class F,class... Args> 
auto ThreadPool::submit(F&& f,Args&&... args)
    -> std::future<std::invoke_result_t<F,Args...>>
    {
        using ReturnType = std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(
                std::forward<F>(f),
                std::forward<Args>(args)...
            )
        );

        std::future<ReturnType> result = task->get_future();

        {
            std::lock_guard<std::mutex> lock(mutex_);

            if (stopping_)
            {
                throw std::runtime_error(
                    "submit on stopped ThreadPool"
                );
            }

            tasks_.emplace([task]() {
                (*task)();
            });
        }

        condition_.notify_one();

        return result;        
    }