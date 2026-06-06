#include "infrastructure/file_config.h"

FileConfig& FileConfig::instance() {
    static FileConfig config;  // 局部静态变量，C++11及以上标准保证线程安全
    return config;
}

static std::string defaultDataDir() {
    return std::string(PROJECT_ROOT) + "/data";
}

void FileConfig::setDataDir(const std::string& dir) {
    data_dir_ = dir;
}

void FileConfig::resetToDefault() {
    data_dir_.clear();
}

std::string FileConfig::resolve(const std::string& filename) const {
    auto base = data_dir_.empty() ? defaultDataDir() : data_dir_;
    return base + "/" + filename;
}

std::string FileConfig::cityDataPath() const {
    return resolve("city.dat");
}
std::string FileConfig::trainDataPath() const {
    return resolve("train_schedules.dat");
}
std::string FileConfig::flightDataPath() const {
    return resolve("flight_schedules.dat");
}
std::string FileConfig::userDataPath() const {
    return resolve("user.dat");
}
