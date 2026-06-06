#include "infrastructure/file_io.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string_view>

#include "file_config.h"
namespace fs = std::filesystem;


namespace file_io {

    // 将字符串解析为 TransportType 枚举值 (nodiscard 表示该函数的返回值不应被忽略,编译器会发出警告)
    [[nodiscard]] static TransportType parseTransportType(std::string_view sv) {
        if (sv == "TRAIN")
            return TransportType::TRAIN;
        if (sv == "PLANE")
            return TransportType::PLANE;
        throw std::runtime_error(std::string("Unknown transport type: ") + std::string(sv));
    }

    // 写入文件
    void saveToFile(const std::vector<City>& cities, const std::vector<Trip>& trips, const std::string& path) {
        // 根据传入的路径参数构造实际的文件路径，如果 path 为空则通过 FileConfig 使用默认路径
        auto& cfg = FileConfig::instance();
        auto cityPath = path.empty() ? cfg.cityDataPath() : (fs::path(path) / "city.dat").string();
        auto trainPath = path.empty() ? cfg.trainDataPath() : (fs::path(path) / "train_schedules.dat").string();
        auto flightPath = path.empty() ? cfg.flightDataPath() : (fs::path(path) / "flight_schedules.dat").string();

        // 确保目录存在，如果不存在则创建 (所有 dat 父目录相同，因此只需创建一次)
        fs::create_directories(fs::path(cityPath).parent_path());

        // --- 写入城市 ---
        // 使用 lambda 函数封装写入逻辑，避免代码重复
        auto writeCities = [&](const std::string& filePath) {
            std::ofstream file(filePath);
            if (!file.is_open())
                throw std::runtime_error("无法打开文件进行写入: " + filePath);
            for (const auto& city : cities)
                file << "CITY " << city.id_ << " " << city.name_ << '\n';
        };

        // --- 写入班次 ---
        auto writeTrips = [&](const std::string& filePath, TransportType type) {
            std::ofstream file(filePath);
            if (!file.is_open())
                throw std::runtime_error("无法打开文件进行写入: " + filePath);
            for (const auto& trip : trips) {
                if (trip.type_ == type) {
                    file << "TRIP " << trip.id_ << " " << trip.type_ << " " << trip.from_city_id_ << " "
                         << trip.to_city_id_ << " " << trip.departure_time_ << " " << trip.arrival_time_ << " "
                         << trip.price_ << " " << trip.trip_number_ << '\n';
                }
            }
        };

        writeCities(cityPath);
        writeTrips(trainPath, TransportType::TRAIN);
        writeTrips(flightPath, TransportType::PLANE);
    }

    // 从文件加载
    TransportData loadFromFile(const std::string& dirPath) {
        auto& cfg = FileConfig::instance();
        auto cityPath = dirPath.empty() ? cfg.cityDataPath() : (fs::path(dirPath) / "city.dat").string();
        auto trainPath = dirPath.empty() ? cfg.trainDataPath() : (fs::path(dirPath) / "train_schedules.dat").string();
        auto flightPath =
            dirPath.empty() ? cfg.flightDataPath() : (fs::path(dirPath) / "flight_schedules.dat").string();
        TransportData data;

        // --- 加载城市 ---
        {
            std::ifstream file(cityPath);
            if (!file.is_open())
                throw std::runtime_error("无法打开文件进行读取: " + cityPath);
            std::string line;
            while (std::getline(file, line)) {
                // TODO(optimize): 用 std::from_chars 替代 std::istringstream 解析数值字段，可避免每次构造 stream 的开销
                if (!line.starts_with("CITY "))  // starts_with ( C++20 ) 用于检查字符串是否以指定前缀开头
                    continue;
                City city;
                std::istringstream iss(line);
                std::string token;  // 用于读取 "CITY" 前缀
                iss >> token >> city.id_ >> city.name_;
                data.addCity(city);
            }
        }

        // --- 加载班次 ---
        auto loadTrips = [&](const std::string& filePath) {
            std::ifstream file(filePath);
            if (!file.is_open())
                throw std::runtime_error("无法打开文件进行读取: " + filePath);
            std::string line;
            while (std::getline(file, line)) {
                // TODO(optimize): 用 std::string_view 按空格分割行内容，避免构造
                // std::istringstream。若预知数据量还可提前 reserve 减少内存分配次数
                if (!line.starts_with("TRIP "))
                    continue;
                Trip trip;
                std::istringstream iss(line);
                std::string token, type_str;
                iss >> token >> trip.id_ >> type_str;
                trip.type_ = parseTransportType(type_str);
                iss >> trip.from_city_id_ >> trip.to_city_id_ >> trip.departure_time_ >> trip.arrival_time_ >>
                    trip.price_ >> trip.trip_number_;
                data.addTrip(trip);
            }
        };

        loadTrips(trainPath);
        loadTrips(flightPath);

        return data;
    }

    void saveUsers(const std::vector<User>& users, const std::string& path) {
        auto& cfg = FileConfig::instance();
        auto userPath = path.empty() ? cfg.userDataPath() : (fs::path(path) / "user.dat").string();
        fs::create_directories(fs::path(userPath).parent_path());

        std::ofstream file(userPath);
        if (!file.is_open())
            throw std::runtime_error("无法打开文件进行写入: " + userPath);
        for (const auto& u : users)
            file << "USER " << u.id_ << " " << u.username_ << " " << u.password_hash_ << " "
                 << userRoleToString(u.role_) << '\n';
    }

    std::vector<User> loadUsers(const std::string& dirPath) {
        auto& cfg = FileConfig::instance();
        auto userPath = dirPath.empty() ? cfg.userDataPath() : (fs::path(dirPath) / "user.dat").string();
        std::vector<User> users;

        std::ifstream file(userPath);
        if (!file.is_open())
            return users;  // 文件不存在时返回空列表而非抛出异常（首次运行场景）

        std::string line;
        while (std::getline(file, line)) {
            if (!line.starts_with("USER "))
                continue;
            User u;
            std::string role_str;
            std::istringstream iss(line);
            std::string token;
            iss >> token >> u.id_ >> u.username_ >> u.password_hash_ >> role_str;
            u.role_ = stringToUserRole(role_str);
            users.push_back(u);
        }
        return users;
    }

}  // namespace file_io
