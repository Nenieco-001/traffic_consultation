#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

#include "domain/data/transport_data.h"
#include "domain/model/domain_types.h"
namespace fs = std::filesystem;

namespace file_io {


    // 写入文件
    void saveToFile(const std::vector<City>& cities, const std::vector<Trip>& trips, const std::string& path = "");

    // 从文件读取
    [[nodiscard]] TransportData loadFromFile(const std::string& dirPath = "");

    // TODO(significant): 细化文件接口，提供更细粒度的文件操作接口，如读取指定文件、写入指定文件、删除指定文件等，以支持更灵活的文件管理和数据更新，当前示例先实现整体读写功能
    // TODO: 未来需要兼容 HH:MM 时间格式的班次输入输出，用于对接人工录入或非标准化数据源，目前仅支持绝对分钟数的输入输出
}  // namespace file_io

// TODO:
// 未来需限制权限，仅允许管理员写入文件，普通用户只能读取文件，或通过接口进行数据修改（接口层会进行权限检查和数据验证），以增强系统安全性和数据完整性
// TODO(optimize): 大文件（>10MB）可考虑内存映射（mmap）或 64KB 缓冲 IO 以减少系统调用次数