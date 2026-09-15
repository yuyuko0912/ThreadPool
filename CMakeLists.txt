cmake_minimum_required(VERSION 3.16) # cmake最低版本

# 项目名称、版本、使用语言
project(ThreadPool
    VERSION 1.0
    LANGUAGES CXX
) 

# c++标准-强制要求-不依赖GCC私有扩展
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 打开编译器警告
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options( # 增加参数
        -Wall # 常见警告
        -Wextra # 额外警告
        -Wpedantic # 更严格筛查
    )
endif()

# 编译产物目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY 
    ${PROJECT_SOURCE_DIR}/bin
)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY 
    ${PROJECT_SOURCE_DIR}/bin
)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY 
    ${PROJECT_SOURCE_DIR}/bin
)

# 查找线程库，找到当前系统对应的线程支持，且线程支持必须存在
find_package(Threads REQUIRED)

# 创建一个Library-库目标，其余可执行文件均会用到这个库
add_library(ThreadPool STATIC  # 静态库
    src/ThreadPool.cpp
)
# 设置头文件搜索路径-基于目标，可精准控制哪些目标可以看到哪些头文件，避免头文件路径污染整个项目。
target_include_directories(ThreadPool
    PUBLIC # 自己用，且传递给依赖者（链接了这个Target的其他目标也自动获得这个包含路径）
        ${PROJECT_SOURCE_DIR}/include # 最近一次project()命令所在目录，拼接指向include/目录
)
# 将这个Target链接到系统线程库
target_link_libraries(ThreadPool
    PUBLIC
        Threads::Threads #先前的寻找部分找到的线程支持
)

# 创建Example，用于使用此线程池
add_executable(ThreadPool_example
    example/main.cpp
)
# 链接到ThreadPool库上
target_link_libraries(ThreadPool_example
    PRIVATE # 自己用，不传递
        ThreadPool
)

# 启动项目测试
enable_testing()
# 基础测试：基本任务提交与执行
add_executable(test_basic
    tests/test_basic.cpp
)
target_link_libraries(test_basic
    PRIVATE
        ThreadPool
)
# 将测试注册给CTest，便于今后执行ctest。返回0为测试成功
add_test(
    NAME basic
    COMMAND test_basic
)

#返回值、异常、future测试
add_executable(test_future
    tests/test_future.cpp
)
target_link_libraries(test_future
    PRIVATE
        ThreadPool
)
add_test(
    NAME future
    COMMAND test_future
)

#多线程并发提交测试
add_executable(test_concurrent
    tests/test_concurrent.cpp
)
target_link_libraries(test_concurrent
    PRIVATE
        ThreadPool
)
add_test(
    NAME concurrent
    COMMAND test_concurrent
)

#线程池关闭测试
add_executable(test_shutdown
    tests/test_shutdown.cpp
)
target_link_libraries(test_shutdown
    PRIVATE
        ThreadPool
)
add_test(
    NAME shutdown
    COMMAND test_shutdown
)

# Benchmark，程序性能测试
add_executable(ThreadPool_benchmark
    benchmark/benchmark.cpp
)
target_link_libraries(ThreadPool_benchmark
    PRIVATE
        ThreadPool
)