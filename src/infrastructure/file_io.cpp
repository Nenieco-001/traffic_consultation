#include "infrastructure/file_io.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string_view>

#include <charconv>
#include "file_config.h"
namespace fs = std::filesystem;


namespace file_io {

    // 将字符串解析为 TransportType 枚举值 (nodiscard 表示该函数的返回值不应被忽略,编译器会发出警告)
    [[nodiscard]] static TransportType parseTransportType(std::string_view sv) {
        if (sv == "TRAIN")
            return TransportType::TRAIN;
        if (sv == "PLANE")
            return TransportType::PLANE;
        throw std::runtime_error(std::string("未知交通类型: ") + std::string(sv));
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
            std::string line;   // 逐行读取文件内容
            while (std::getline(file, line)) {
                if (!line.starts_with("CITY "))
                    continue;
                City city;
                std::string_view sv(line);  // 使用 string_view 避免不必要的字符串复制
                sv.remove_prefix(5);  // 跳过 "CITY "
                auto result = std::from_chars(sv.data(), sv.data() + sv.size(), city.id_);
                if (result.ec != std::errc())
                    throw std::runtime_error("解析城市 ID 失败，行内容: " + line);
                sv.remove_prefix(result.ptr - sv.data());  // 跳过 ID
                if (!sv.empty() && sv.front() == ' ')
                    sv.remove_prefix(1);  // 跳过 ID 和名称之间的空格
                city.name_ = std::string(sv);
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
                if (!line.starts_with("TRIP "))
                    continue;
                Trip trip;
                std::string_view sv(line);
                sv.remove_prefix(5);  // 跳过 "TRIP "

                // 定义一个 lambda 函数来解析下一个整数值，并更新 string_view 的位置
                auto parseNext = [&](int& val) {
                    auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), val);
                    sv.remove_prefix(ptr - sv.data());
                    if (!sv.empty() && sv.front() == ' ')
                        sv.remove_prefix(1);
                };

                parseNext(trip.id_);

                // 解析交通类型
                {
                    auto space = sv.find(' ');  // 查找下一个空格分隔符
                    trip.type_ = parseTransportType(sv.substr(0, space));
                    sv.remove_prefix(space + 1);
                }

                parseNext(trip.from_city_id_);
                parseNext(trip.to_city_id_);
                parseNext(trip.departure_time_);
                parseNext(trip.arrival_time_);
                parseNext(trip.price_);

                // 解析班次号
                trip.trip_number_ = std::string(sv);
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
            file << "USER " << u.id_ << " " << u.username_ << " " << u.password_hash_ << " " << userRoleToString(u.role_)
                 << '\n';
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
