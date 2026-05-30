#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "tools/time_process.h"

#include "model/transport_type.h"
#include "data/transport_data.h"
namespace fs = std::filesystem;
namespace file_io {


    // 写入文件
    void saveToFile(const std::vector<City>& cities, const std::vector<Trip>& trips, const std::string& path = "");

    // 从文件读取
    [[nodiscard]] TransportData loadFromFile(const std::string& dirPath = "");

    // TODO: 未来需要兼容 HH:MM 时间格式的班次输入输出，用于对接人工录入或非标准化数据源，目前仅支持绝对分钟数的输入输出
}  // namespace file_io

// TODO: 未来需限制权限，仅允许管理员写入文件，普通用户只能读取文件，或通过接口进行数据修改（接口层会进行权限检查和数据验证），以增强系统安全性和数据完整性
// TODO(optimize): 大文件（>10MB）可考虑内存映射（mmap）或 64KB 缓冲 IO 以减少系统调用次数