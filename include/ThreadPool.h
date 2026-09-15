#pragma once

// 多线程
#include <thread> // 工作线程
#include <mutex> // 锁
#include <condition_variable> // 线程等待/唤醒

#include <queue> // 存放待执行任务
#include <vector> // 存放多个线程
#include <functional> // 包装任务
#include <cstddef> // size_t 等基础类型

