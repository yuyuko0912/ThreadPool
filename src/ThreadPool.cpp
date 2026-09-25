#include "ThreadPool.h"



// 含参构造函数：创建时线程池状态为开启
ThreadPool::ThreadPool(std::size_t threadCount):stopping_(false){
    if(threadCount == 0){
        throw std::invalid_argument("threadCount must be greater than 0!"); // 参数非法异常
    }

    workers_.reserve(threadCount); // 预分配空间：需要的线程数量

    for(std::size_t i = 0;i < threadCount;++i){
        workers_.emplace_back(&ThreadPool::worker,this); // 每个线程启动后都调用当前线程池对象中的worker()成员函数。
    }

}

// 每个空闲线程轮流对任务队列上锁，等待并从中取出任务，执行后排队等待任务队列解锁，重复。
void ThreadPool::worker(){
    while(true){
        std::function<void()> task;
        // RALL作用域块，锁住访问任务队列的时间。
        {
            std::unique_lock<std::mutex> lock(mutex_); // 上锁

            condition_.wait(lock,[this]{
                return stopping_ || !tasks_.empty();

            }); // 等待，线程池关闭或任务队列不为空时启动。
            
            if(stopping_ && tasks_.empty()){
                return;
            } // 如果线程池关闭且所有任务执行完毕，关闭当前线程。
            
            // 线程池开启且有任务时，拿取，并让任务出队。
            task = std::move(tasks_.front()); // move() 是将当前队列头部任务的所有权转移给局部变量task。
            tasks_.pop();
        }
        task(); // 在当前线程运行该任务。

    }

}

// 关闭线程池（与析构时一个流程）
void ThreadPool::shutdown(){
    {
    std::lock_guard<std::mutex> lock(mutex_);
    stopping_ = true;
    }

    condition_.notify_all(); // 唤醒全部线程

    for(std::thread& worker : workers_){ // 这里使用引用来避免拷贝（thread不可拷贝）
        if(worker.joinable()){ // 判断线程池是否可以join。
            worker.join(); // 阻塞等待该线程执行完毕，之后回收资源。
        }
    }
}

// 析构函数，优雅退出
ThreadPool::~ThreadPool(){
    shutdown();
}