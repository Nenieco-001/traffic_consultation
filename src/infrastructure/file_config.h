#pragma once

#include <string>

// 文件路径配置（单例）- 为系统拥有者提供接口设置数据文件路径，支持默认（PROJECT_ROOT/data/）和自定义两种模式
// 封装数据文件路径解析，支持默认（PROJECT_ROOT）和自定义两种模式
class FileConfig {
public:
    static FileConfig& instance();  // 获取单例实例，线程安全的局部静态变量实现

    std::string cityDataPath() const;
    std::string trainDataPath() const;
    std::string flightDataPath() const;

    void setDataDir(const std::string& dir);
    void resetToDefault();

private:
    FileConfig() = default;
    std::string resolve(const std::string& filename) const; // 内部路径解析函数

    std::string data_dir_;  // 空 = 使用 PROJECT_ROOT/data/
};
