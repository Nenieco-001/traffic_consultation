cmake_minimum_required(VERSION 3.25) # 指定 CMake 最低版本

# project() 必须在 add_executable/add_library 之前调用
# 它会初始化编译器检测、设置语言标准等
project(Traffic_Information LANGUAGES CXX) # 项目名 Traffic_Information，仅启用 C++

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20) # 使用 C++20 标准
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 强制要求编译器支持 C++20，若不支持则报错

# 注入项目根目录作为编译宏（替代运行时 fs::current_path()，消除路径依赖）
add_compile_definitions("PROJECT_ROOT=\"${CMAKE_SOURCE_DIR}\"")

# 设置可执行文件输出目录（必须放在 add_executable 之前才生效）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
# 设置头文件搜索路径
include_directories(${CMAKE_SOURCE_DIR}/inc)
# 添加源文件并生成可执行文件
# TODO: 后续迭代需在此添加 algo/ controller/ ui/ 等子目录的源文件
add_executable(Traffic_Information
    src/main.cpp
    src/tools/time_process.cpp
    src/data/transport_data.cpp
    src/io/file_io.cpp
)
# 添加局部功能测试文件
add_executable(Time_process_test
    tests/time_process_test.cpp
    src/tools/time_process.cpp
)

add_executable(Transport_type_test
    tests/transport_type_test.cpp
)

add_executable(Query_request_test
    tests/query_request_test.cpp
)

add_executable(File_io_test
    tests/file_io_test.cpp
    src/tools/time_process.cpp
    src/data/transport_data.cpp
    src/io/file_io.cpp
)

add_executable(data_layer_test
    tests/data_layer_test.cpp
    src/data/transport_data.cpp
    src/io/file_io.cpp
)

add_executable(algo_layer_test
    tests/algo_layer_test.cpp
    src/data/transport_data.cpp
    src/io/file_io.cpp
    src/algo/dijkstra.cpp
)
