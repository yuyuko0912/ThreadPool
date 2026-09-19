# 线程池

## 核心思想

提前创建一批工作线程，让这些线程反复从任务队列中获取任务并执行。

流程：调用者 -> 把任务放进队列 -> 唤醒一个工作线程 -> 工作线程取出任务 -> 执行任务 -> 再次等待

**模型：** 生产者 - 消费者。
**警告：** 线程池本身应该禁止被拷贝。

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

最后要实现**优雅退出**。

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


## 线程池类
**禁用拷贝构造函数与=的重载！！线程池不可被任何形式的拷贝。因为线程池中含有不可拷贝参数（如thread、mutex等）**

### 公开
1.含参构造函数 
```explicit ThreadPool(std::size_t threadCount);```
必须显示构造函数，输入的参数为创建的线程数目。`size_t` 的作用是**拒绝负数**。
2.析构函数、
```
~ThreadPool();
```

### 私有

#### 成员函数
1.`void worker()；`
拿取任务时用 `move(tasks_.front())` ，意为把队列头部任务的所有权移给局部变量 `task` 。
- `tasks_.front()` 返回队首元素的引用，不是拷贝。
- `std::move` 把它转成右值引用，触发 `std::function` 的移动赋值。
- 结果：`task` 接管任务内容，队首元素变成"空壳"（`function` 移走后为空）。
使用 `move` 可以直接转移内部资源，无需拷贝，效率比较高。如果用`task = tasks_.front();` 则需要拷贝资源，开销较大。

2. 
```
template <class F,class... Args>
auto submit(F&& f,Args&&... args)
        -> std::future<std::invoke_result_t<F,Args...>>;
```
- auto ... -> T：尾置返回类型，因为返回类型中用到了参数F和Args，它们俩在参数列表里面才出现。
- `F` 是接住的任务本身的类型，`Args` 是任务需要的参数的类型们，是一个参数包，含有零个或多个类型。
- `submit` 中，`f` 是任务本身，`F` 是任务类型；`args` 是参数列表的参数，` Args` 是参数列表的参数类型。
- 用 `&&` 作转发引用，将传入的实参自动变成左值引用或右值引用，保留其本身的左右值属性。
- `std::invoke_result_t<F, Args...> `几乎等于（把 `args` 喂给 `f` 后，`f` 返回的类型）。
#### 成员变量

// 成员变量
1. 存储线程。
```
vector<thread> workers_;
``` 
- 可以工作的线程集合，具体取决于创建线程池时输入的线程个数。
2. 包装并存储任务
```
queue<function<void()>> tasks_;
``` 
- **先进先出：**用单端队列存储任务。
- **类型擦除：**各种类型的任务都被打包为无参无返回值的 `void` 类型的函数，存储在函数包装器`function` 中。，使进入线程池内部的所有任务统一变成 `tasks_` 。
*后续加入 `bind` 进一步将有参函数也打包。提升线程池的兼容性。也可以引入 `priority_queue` 来实现优先队列，对于任务按照优先级排列，使优先级最大的任务永远处于 `top` 。*
3. 冲突管理
生产者-消费者模型实现依赖互斥锁和条件变量
```
mutex mtex_;
condtion_variable condition_;
```
- 用 `mutex` 解决多个worker同时访问任务队列的问题。避免数据竞争。
- 用 `condition_variable` 解决当队列为空时，防止Worker一直死循环的问题。改为休眠。
```
无任务时->worker休眠->submit()加入任务->notify_one->worker被唤醒
```
4. 判断线程池状态
```
bool stopping_;
```
- 值为false时，线程池可以正常工作，可继续提交任务。
- 值为true时，线程池关闭，不再接收新的任务，worker逐渐退出。
是后续实现优雅关闭的核心。

## 优雅关闭
确认要关闭线程池时：
禁止新任务的进入->worker继续处理已有任务->直到队列为空->worker退出->等待全部线程执行完毕。