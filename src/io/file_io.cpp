#include "io/file_io.h"



namespace file_io {

    // 默认数据文件路径（基于 PROJECT_ROOT 编译宏，与运行目录无关）
    static const std::string defaultCityDataPath = std::string(PROJECT_ROOT) + "/data/city.dat";
    static const std::string defaultTrainDataPath = std::string(PROJECT_ROOT) + "/data/train_schedules.dat";
    static const std::string defaultFlightDataPath = std::string(PROJECT_ROOT) + "/data/flight_schedules.dat";

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
        // 根据传入的路径参数构造实际的文件路径，如果 path 为空则使用默认路径
        auto cityPath = path.empty() ? defaultCityDataPath : (fs::path(path) / "city.dat").string();
        auto trainPath = path.empty() ? defaultTrainDataPath : (fs::path(path) / "train_schedules.dat").string();
        auto flightPath = path.empty() ? defaultFlightDataPath : (fs::path(path) / "flight_schedules.dat").string();

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
        auto cityPath = dirPath.empty() ? defaultCityDataPath : (fs::path(dirPath) / "city.dat").string();
        auto trainPath = dirPath.empty() ? defaultTrainDataPath : (fs::path(dirPath) / "train_schedules.dat").string();
        auto flightPath =
            dirPath.empty() ? defaultFlightDataPath : (fs::path(dirPath) / "flight_schedules.dat").string();
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
        // TODO(optimize): 用 std::string_view 按空格分割行内容，避免构造 std::istringstream。若预知数据量还可提前 reserve 减少 reallocation
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

}  // namespace file_io
