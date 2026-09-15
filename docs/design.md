# 线程池

## 核心思想

提前创建一批工作线程，让这些线程反复从任务队列中获取任务并执行。

流程：调用者 -> 把任务放进队列 -> 唤醒一个工作线程 -> 工作线程取出任务 -> 执行任务 -> 再次等待

## 核心组件

1. 工作线程
2. 任务队列
3. 同步机制

## 1. 工作线程

假设创建四个线程

```c++
std::vector<std::thread> workers;
for (int i = 0; i < 4; ++i)
{
    workers.emplace_back(...);
}
```

这四个线程创建后不会退出，而是不断的等待任务 -> 拿任务 -> 执行 -> 等待任务 ...

## 2. 任务队列

使用

```c++
std::queue<std::function<void()>> tasks;
```

来保存任务。

线程池负责调度任务，不关心任务具体做什么。因此使用`std::function<void()>`。

## 3. 同步机制

需要加锁 `mutex` 来防止多个线程同时修改一个任务。

没有任务时不应该 `while (tasks.empty()) {}` ，这是**忙等待**，CPU会一直空转。应当让线程在没有任务时睡眠： `std::condition_variable` 。

二者一同使用，利用RALL自动管理锁。

```c++
std::unique_lock<std::mutex> lock(queueMutex);

condition.wait(lock, [this] {
    return !tasks.empty() || stop;
});
```

进入： `lock()` ，退出作用域： `unlock()` 。

## 基本逻辑

```
① 创建线程池
       ↓
② 创建 N 个 worker
       ↓
③ worker 等待
       ↓
④ 用户 enqueue(task)
       ↓
⑤ task 放入 queue
       ↓
⑥ notify_one()
       ↓
⑦ worker 唤醒
       ↓
⑧ worker 从 queue 取 task
       ↓
⑨ 执行 task
       ↓
⑩ 回到等待
```

## 文件目录

```c++
ThreadPool/
├── CMakeLists.txt
├── README.md
├── LICENSE           ← 可选
├── .gitignore
│
├── include/
│   └── ThreadPool.h
│
├── src/
│   └── ThreadPool.cpp
│
├── example/
│   └── main.cpp
│
├── tests/
│   ├── test_basic.cpp
│   ├── test_future.cpp
│   ├── test_concurrent.cpp
│   └── test_shutdown.cpp
│
├── benchmark/
│   └── benchmark.cpp
│
├── docs/
│   └── design.md
│
├── build/              ← CMake构建产生的东西
│   ├── CMakeCache.txt
│   ├── CMakeFiles/
│   └── ...
│
└── bin/                ← 最终可执行文件
    ├── ThreadPool_example
    ├── test_basic
    ├── test_future
    ├── test_concurrent
    ├── test_shutdown
    └── ThreadPool_benchmark
```

## 第一版核心实现

```
固定 Worker 数量
线程安全任务队列
condition_variable 阻塞/唤醒
泛型任务提交
任意参数
future 返回值
异常传播
RAII
优雅关闭
多线程测试
CMake
Benchmark
Linux
```

### 阶段拆分

```
第一阶段
│
├── Worker线程
├── 任务队列
├── mutex
├── condition_variable
└── 基本关闭机制

第二阶段
│
├── 泛型submit()
├── 任意参数
└── future

第三阶段
│
├── 异常传播
└── RAII

第四阶段
│
├── 多线程测试
├── shutdown测试
└── benchmark

第五阶段
│
└── README + design.md
```

