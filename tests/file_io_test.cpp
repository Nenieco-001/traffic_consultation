// IO 层测试
// 只读用例读取 tests_data/file_io_test/.dat（独立测试数据）
// 写入用例使用 tests_data/tmp/（gitignored）

#include "infrastructure/file_io.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <string>

// =============================================
// 辅助函数
// =============================================

static TransportData loadTestData() {
    std::string dir = std::string(PROJECT_ROOT) + "/tests_data/file_io_test";
    return file_io::loadFromFile(dir);
}

static std::string getWriteDir() {
    return std::string(PROJECT_ROOT) + "/tests_data/tmp";
}

static int countType(const std::vector<Trip>& trips, TransportType type) {
    return static_cast<int>(std::count_if(trips.begin(), trips.end(),
                                          [type](const Trip& t) { return t.type_ == type; }));
}

// =============================================
// 测试 1：加载城市 — 只读，使用独立数据
// =============================================

static void test_load_cities() {
    std::cout << "--- 加载城市 ---" << std::endl;

    auto data = loadTestData();
    auto cities = data.getAllCities();

    assert(cities.size() == 2);
    assert(cities[0].id_ == 1 && cities[0].name_ == "CityA");
    assert(cities[1].id_ == 2 && cities[1].name_ == "CityB");

    std::cout << "  城市数量: " << cities.size() << "（期望 2）" << std::endl;
    std::cout << "  CityA: id=" << cities[0].id_ << " name=" << cities[0].name_ << std::endl;
    std::cout << "  CityB: id=" << cities[1].id_ << " name=" << cities[1].name_ << std::endl;
    std::cout << "[PASS] 加载城市测试通过\n" << std::endl;
}

// =============================================
// 测试 2：加载 TRAIN 班次 — 只读
// =============================================

static void test_load_trips_train() {
    std::cout << "--- 加载 TRAIN 班次 ---" << std::endl;

    auto data = loadTestData();
    auto trips = data.getAllTrips();

    // 按类型查找 TRAIN 班次（不依赖硬编码下标）
    auto it = std::find_if(trips.begin(), trips.end(),
                           [](const Trip& t) { return t.type_ == TransportType::TRAIN; });
    assert(it != trips.end());

    assert(it->id_ == 1);
    assert(it->type_ == TransportType::TRAIN);
    assert(it->from_city_id_ == 1);
    assert(it->to_city_id_ == 2);
    assert(it->departure_time_ == 480);
    assert(it->arrival_time_ == 600);
    assert(it->price_ == 100);
    assert(it->trip_number_ == "T123");

    std::cout << "  TRAIN: id=" << it->id_ << " " << it->trip_number_
              << " " << it->from_city_id_ << "→" << it->to_city_id_
              << " " << it->departure_time_ << "→" << it->arrival_time_
              << " " << it->price_ << "元" << std::endl;
    std::cout << "[PASS] 加载 TRAIN 班次测试通过\n" << std::endl;
}

// =============================================
// 测试 3：加载 PLANE 班次 — 只读
// =============================================

static void test_load_trips_plane() {
    std::cout << "--- 加载 PLANE 班次 ---" << std::endl;

    auto data = loadTestData();
    auto trips = data.getAllTrips();

    auto it = std::find_if(trips.begin(), trips.end(),
                           [](const Trip& t) { return t.type_ == TransportType::PLANE; });
    assert(it != trips.end());

    assert(it->id_ == 2);
    assert(it->type_ == TransportType::PLANE);
    assert(it->from_city_id_ == 1);
    assert(it->to_city_id_ == 2);
    assert(it->departure_time_ == 540);
    assert(it->arrival_time_ == 660);
    assert(it->price_ == 300);
    assert(it->trip_number_ == "P456");

    std::cout << "  PLANE: id=" << it->id_ << " " << it->trip_number_
              << " " << it->from_city_id_ << "→" << it->to_city_id_
              << " " << it->departure_time_ << "→" << it->arrival_time_
              << " " << it->price_ << "元" << std::endl;
    std::cout << "[PASS] 加载 PLANE 班次测试通过\n" << std::endl;
}

// =============================================
// 测试 4：加载后汇总验证 — 聚合断言
// =============================================

static void test_load_all_trips() {
    std::cout << "--- 加载后汇总验证 ---" << std::endl;

    auto data = loadTestData();
    auto trips = data.getAllTrips();

    assert(trips.size() == 2);
    assert(countType(trips, TransportType::TRAIN) == 1);
    assert(countType(trips, TransportType::PLANE) == 1);

    std::cout << "  班次总数: " << trips.size() << "（期望 2）" << std::endl;
    std::cout << "  TRAIN: " << countType(trips, TransportType::TRAIN) << "（期望 1）" << std::endl;
    std::cout << "  PLANE: " << countType(trips, TransportType::PLANE) << "（期望 1）" << std::endl;
    std::cout << "[PASS] 汇总验证测试通过\n" << std::endl;
}

// =============================================
// 测试 5：保存并重新加载（往返一致性）
// =============================================

static void test_save_and_reload_roundtrip() {
    std::cout << "--- 保存并重新加载（往返一致性） ---" << std::endl;

    std::vector<City> cities = {{1, "CityA"}, {2, "CityB"}};
    std::vector<Trip> trips = {
        {1, TransportType::TRAIN, 1, 2, 480, 600, 100, "T123"},
        {2, TransportType::PLANE, 1, 2, 540, 660, 300, "P456"},
    };

    std::string dir = getWriteDir();
    file_io::saveToFile(cities, trips, dir);

    auto data = file_io::loadFromFile(dir);
    auto loadedCities = data.getAllCities();
    auto loadedTrips = data.getAllTrips();

    assert(loadedCities.size() == 2);
    assert(loadedCities[0].id_ == 1 && loadedCities[0].name_ == "CityA");
    assert(loadedCities[1].id_ == 2 && loadedCities[1].name_ == "CityB");

    assert(loadedTrips.size() == 2);
    assert(countType(loadedTrips, TransportType::TRAIN) == 1);
    assert(countType(loadedTrips, TransportType::PLANE) == 1);

    auto trainIt = std::find_if(loadedTrips.begin(), loadedTrips.end(),
                                [](const Trip& t) { return t.type_ == TransportType::TRAIN; });
    assert(trainIt != loadedTrips.end());
    assert(trainIt->trip_number_ == "T123");
    assert(trainIt->price_ == 100);

    auto planeIt = std::find_if(loadedTrips.begin(), loadedTrips.end(),
                                [](const Trip& t) { return t.type_ == TransportType::PLANE; });
    assert(planeIt != loadedTrips.end());
    assert(planeIt->trip_number_ == "P456");
    assert(planeIt->price_ == 300);

    std::cout << "  写入 " << dir << " → 重新读取，数据一致" << std::endl;
    std::cout << "[PASS] 往返一致性测试通过\n" << std::endl;
}

// =============================================
// 测试 6：保存空数据
// =============================================

static void test_save_empty_roundtrip() {
    std::cout << "--- 保存空数据 ---" << std::endl;

    std::vector<City> emptyCities;
    std::vector<Trip> emptyTrips;

    std::string dir = getWriteDir() + "/empty_test";
    file_io::saveToFile(emptyCities, emptyTrips, dir);

    auto data = file_io::loadFromFile(dir);
    assert(data.getAllCities().empty());
    assert(data.getAllTrips().empty());

    std::cout << "  空城市: " << data.getAllCities().size() << "（期望 0）" << std::endl;
    std::cout << "  空班次: " << data.getAllTrips().size() << "（期望 0）" << std::endl;
    std::cout << "[PASS] 保存空数据测试通过\n" << std::endl;
}

// =============================================
// 测试 7：默认路径加载（不传路径 = PROJECT_ROOT/data/）
// =============================================

static void test_load_default_path() {
    std::cout << "--- 默认路径加载 ---" << std::endl;

    // loadFromFile("") 使用 PROJECT_ROOT/data/ 下的默认文件
    // 只要不抛异常且有数据就说明路径解析正确
    auto data = file_io::loadFromFile("");
    assert(data.getAllCities().size() > 0);
    assert(data.getAllTrips().size() > 0);

    std::cout << "  默认路径城市: " << data.getAllCities().size() << "（期望 > 0）" << std::endl;
    std::cout << "  默认路径班次: " << data.getAllTrips().size() << "（期望 > 0）" << std::endl;
    std::cout << "[PASS] 默认路径加载测试通过\n" << std::endl;
}

// =============================================
// 测试 8：文件不存在 → 抛异常
// =============================================

static void test_file_not_found_throws() {
    std::cout << "--- 文件不存在错误处理 ---" << std::endl;

    std::string badDir = std::string(PROJECT_ROOT) + "/tests_data/non_existent_dir_xyzzy";
    bool threw = false;

    try {
        auto _ = file_io::loadFromFile(badDir);
        (void)_;
    } catch (const std::runtime_error& e) {
        threw = true;
        std::cout << "  捕获异常: " << e.what() << std::endl;
    }

    assert(threw);

    std::cout << "[PASS] 文件不存在抛出异常测试通过\n" << std::endl;
}

// =============================================
// 主函数
// =============================================

int main() {
    std::cout << "========================================\n";
    std::cout << "  文件 IO 层测试\n";
    std::cout << "========================================\n" << std::endl;

    test_load_cities();
    test_load_trips_train();
    test_load_trips_plane();
    test_load_all_trips();
    test_save_and_reload_roundtrip();
    test_save_empty_roundtrip();
    test_load_default_path();
    test_file_not_found_throws();

    std::cout << "========================================\n";
    std::cout << "  全部文件 IO 层测试通过!\n";
    std::cout << "========================================\n";
    return 0;
}
