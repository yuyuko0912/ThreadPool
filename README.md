# ThreadPool

一个基于 C++11/14/17/20 的高性能线程池实现。

## 特性

- **固定线程数**：支持自定义工作线程数量
- **任务队列**：安全的阻塞任务队列
- **Future 支持**：支持返回 std::future 获取任务结果
- **优先级队列**：支持优先级任务调度
- **优雅关闭**：支持线程池安全关闭，等待任务完成

## 快速开始

### 构建项目

```bash
mkdir build
cd build
cmake ..
make
```

### 基本用法

```cpp
#include "ThreadPool.h"
#include <iostream>

int main() {
    // 创建包含 4 个工作线程的线程池
    ThreadPool pool(4);

    // 提交任务
    auto result = pool.enqueue([](int a, int b) {
        return a + b;
    }, 1, 2);

    // 获取结果
    std::cout << "Result: " << result.get() << std::endl;

    return 0;
}
```

### 带优先级的任务

```cpp
// 提交优先级任务 (priority 值越大优先级越高)
pool.enqueuePriority(1, [](int a, int b) {
    return a + b;
}, 1, 2);
```

## 项目结构

```
.
├── include/          # 头文件
│   └── ThreadPool.h
├── src/              # 源代码
│   └── ThreadPool.cpp
├── example/          # 示例代码
│   └── main.cpp
├── tests/            # 单元测试
│   ├── test_basic.cpp
│   ├── test_concurrent.cpp
│   ├── test_future.cpp
│   ├── test_priority.cpp
│   └── test_shutdown.cpp
├── benchmark/        # 性能测试
│   └── benchmark.cpp
├── docs/             # 设计文档
│   └── design.md
└── CMakeLists.txt
```

## 运行测试

```bash
# 运行所有测试
./build/tests/test_basic
./build/tests/test_concurrent
./build/tests/test_future
./build/tests/test_priority
./build/tests/test_shutdown

# 运行性能测试
./build/benchmark/benchmark
```

## API 参考

### ThreadPool

```cpp
// 构造函数
ThreadPool(std::size_t threadCount);

// 提交任务 (返回 std::future)
template<typename F, typename... Args>
auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

// 提交优先级任务
template<typename F, typename... Args>
auto enqueuePriority(int priority, F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

// 关闭线程池 (等待任务完成)
void shutdown();

// 析构函数 (自动调用 shutdown)
~ThreadPool();
```

## 设计文档

详细的设计思路和实现细节请参阅 [docs/design.md](docs/design.md)。

## 许可证

MIT License