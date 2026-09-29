#pragma once

// 多线程
#include <iostream>
#include <thread> // 工作线程
#include <mutex> // 锁
#include <condition_variable> // 线程等待/唤醒

#include <queue> // 存放待执行任务
#include <vector> // 存放多个线程
#include <cstddef> // size_t 等基础类型

#include <functional> // 包装任务
#include <utility> // move()- 转移任务所有权给运行的线程
#include <future> // 配合任务包装与多线程实现异步编程
#include <type_traits> // 自动推导返回类型等：用于包装任务

#include <stdexcept> // 异常类，报告“逻辑错误”与“运行时错误”。

#include <memory>


class ThreadPool{
public:
    // 含参构造函数
    explicit ThreadPool(std::size_t threadCount); // 创建含有threadCount个线程的线程池
    // 析构函数
    ~ThreadPool();

    // 禁用任何形式的拷贝构造函数
    ThreadPool(const ThreadPool&) = delete; 
    ThreadPool& operator=(const ThreadPool&) = delete; // 禁用 =  的重载。

    // 主动停止线程池
    void shutdown();
    
    // 对外接口，获取并包装任务
    template <class F,class... Args> 
    auto submit(F&& f,Args&&... args)
        -> std::future<std::invoke_result_t<F,Args...>>; // 尾置返回类型，返回值类型根据参数推导出。

    template <class F,class... Args>
    auto submitWithPriority(int priority,F&&f,Args&&... args)
        -> std::future<std::invoke_result_t<F,Args...>>;

private:
// 成员函数
    void worker(); // 线程拿取任务进行工作。 

// 成员变量
    std::vector<std::thread> workers_; // 存储线程。
    struct Task{
        int priority;
        std::function<void()> function;
        
        bool operator<(const Task& other) const{
            return priority < other.priority;  // 比较规则
        }
    };
    std::priority_queue<Task> tasks_; // 扩展为包含任务优先级版本的任务队列
    // std::queue<std::function<void()>> tasks_; 1.0 存储任务，类型擦除。
    // 加锁
    std::mutex mutex_;
    // 无任务时休眠
    std::condition_variable condition_;

    bool stopping_; // 线程池是否关闭
};

// submit函数体实现 -> 函数模板的实现放头文件比较好
template <class F,class... Args> 
auto ThreadPool::submit(F&& f,Args&&... args)
    -> std::future<std::invoke_result_t<F,Args...>>
    {   
        // 包装函数+函数参数
        using ReturnType = std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(
                std::forward<F>(f), // 完美转发。
                std::forward<Args>(args)...
            ) // 将参数与函数绑定，之后调用task()即可执行函数f，并传入参数args。
        ); // 共享对象shared_ptr，保持task的生命周期，直到任务执行完毕。

        std::future<ReturnType> result = task->get_future(); // 将任务返回值与result绑定，可用get获取。

        {
            std::lock_guard<std::mutex> lock(mutex_);

            if (stopping_)
            {
                throw std::runtime_error(
                    "submit on stopped ThreadPool"
                );
            } // 线程池关闭，停止接收任务

            tasks_.push(Task{0,  // 默认优先级为0，便于原先测试仍可使用。
                [task]() {
                    (*task)();
                }
            }); // (*task)() 指针解引用，调用task对象的operator()，执行任务。
        } // 访问队列时使用互斥锁

        condition_.notify_one(); // 有任务加入，唤醒一个线程。

        return result;        
    }

// submitWithPriority的具体实现
template <class F, class... Args>
auto ThreadPool::submitWithPriority(int priority,F&& f,Args&&... args) 
    -> std::future<std::invoke_result_t<F, Args...>>
    {   
        // 包装函数+函数参数
        using ReturnType = std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(
                std::forward<F>(f), // 完美转发。
                std::forward<Args>(args)...
            ) // 将参数与函数绑定，之后调用task()即可执行函数f，并传入参数args。
        ); // 共享对象shared_ptr，保持task的生命周期，直到任务执行完毕。

        std::future<ReturnType> result = task->get_future(); // 将任务返回值与result绑定，可用get获取。

        {
            std::lock_guard<std::mutex> lock(mutex_);

            if (stopping_)
            {
                throw std::runtime_error(
                    "submit on stopped ThreadPool"
                );
            } // 线程池关闭，停止接收任务

            tasks_.push(Task{priority,  // 与submit唯一不同之处！！！
                [task]() {
                    (*task)();
                }
            }); // (*task)() 指针解引用，调用task对象的operator()，执行任务。
        } // 访问队列时使用互斥锁

        condition_.notify_one(); // 有任务加入，唤醒一个线程。

        return result;     
    }