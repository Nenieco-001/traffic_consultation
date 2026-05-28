cmake_minimum_required(VERSION 3.25)          # 指定 CMake 最低版本

# project() 必须在 add_executable/add_library 之前调用
# 它会初始化编译器检测、设置语言标准等
project(Traffic_Infomation LANGUAGES CXX)             # 项目名 Traffic_Infomation，仅启用 C++

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)                    # 使用 C++20 标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)           # 强制要求编译器支持 C++20，若不支持则报错

# 设置可执行文件输出目录（必须放在 add_executable 之前才生效）
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
# 添加可执行文件，指定源文件
