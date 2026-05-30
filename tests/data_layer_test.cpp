#include "data/transport_data.h"
#include "io/file_io.h"

#include <algorithm>
#include <cassert>
#include <iostream>

// =============================================
// 辅助函数
// =============================================

static TransportData loadTestData() {
    std::string dir = std::string(PROJECT_ROOT) + "/tests_data";
    return file_io::loadFromFile(dir);
}

static int countType(const std::vector<Trip>& trips, TransportType type) {
    return static_cast<int>(std::count_if(trips.begin(), trips.end(),
                                          [type](const Trip& t) { return t.type_ == type; }));
}

// =============================================
// 测试 1：加载后数据完整性
// =============================================

void test_load_basic() {
    std::cout << "--- 测试加载后数据完整性 ---" << std::endl;

    auto data = loadTestData();

    auto cities = data.getAllCities();
    assert(cities.size() == 6);
    assert(cities[0].id_ == 1 && cities[0].name_ == "Beijing");
    assert(cities[1].id_ == 2 && cities[1].name_ == "Shanghai");
    assert(cities[2].id_ == 3 && cities[2].name_ == "Guangzhou");
    assert(cities[3].id_ == 4 && cities[3].name_ == "Shenzhen");
    assert(cities[4].id_ == 5 && cities[4].name_ == "Chengdu");
    assert(cities[5].id_ == 6 && cities[5].name_ == "Wuhan");

    auto trips = data.getAllTrips();
    assert(trips.size() == 32);
    assert(countType(trips, TransportType::TRAIN) == 18);
    assert(countType(trips, TransportType::PLANE) == 14);

    // 验证 G1
    assert(trips[0].id_ == 1 && trips[0].type_ == TransportType::TRAIN &&
           trips[0].from_city_id_ == 1 && trips[0].to_city_id_ == 2 &&
           trips[0].departure_time_ == 420 && trips[0].arrival_time_ == 706 &&
           trips[0].price_ == 553 && trips[0].trip_number_ == "G1");

    // 验证 CA1234
    assert(trips[18].id_ == 19 && trips[18].type_ == TransportType::PLANE &&
           trips[18].from_city_id_ == 1 && trips[18].to_city_id_ == 2 &&
           trips[18].departure_time_ == 420 && trips[18].arrival_time_ == 540 &&
           trips[18].price_ == 800 && trips[18].trip_number_ == "CA1234");

    // 验证 CZ5100（最后一条）
    assert(trips[31].id_ == 32 && trips[31].type_ == TransportType::PLANE &&
           trips[31].from_city_id_ == 4 && trips[31].to_city_id_ == 1 &&
           trips[31].departure_time_ == 960 && trips[31].arrival_time_ == 1140 &&
           trips[31].price_ == 1250 && trips[31].trip_number_ == "CZ5100");

    std::cout << "  城市数量: " << cities.size() << "（期望 6）" << std::endl;
    std::cout << "  班次数量: " << trips.size() << "（期望 32）" << std::endl;
    std::cout << "  火车班次: " << countType(trips, TransportType::TRAIN) << "（期望 18）" << std::endl;
    std::cout << "  航班班次: " << countType(trips, TransportType::PLANE) << "（期望 14）" << std::endl;
    std::cout << "[PASS] 加载后数据完整性测试通过\n" << std::endl;
}

// =============================================
// 测试 2：增删城市
// =============================================

void test_city_operations() {
    std::cout << "--- 测试增删城市 ---" << std::endl;

    auto data = loadTestData();
    assert(data.getAllCities().size() == 6);

    data.addCity({7, "Xi'an"});
    assert(data.getAllCities().size() == 7);

    data.addCity({8, "Nanjing"});
    assert(data.getAllCities().size() == 8);

    data.removeCity(7);
    assert(data.getAllCities().size() == 7);

    data.removeCity(8);
    assert(data.getAllCities().size() == 6);

    // 删除不存在的城市 → 无变化
    data.removeCity(999);
    assert(data.getAllCities().size() == 6);

    std::cout << "[PASS] 增删城市测试通过\n" << std::endl;
}

// =============================================
// 测试 3：增删班次
// =============================================

void test_trip_operations() {
    std::cout << "--- 测试增删班次 ---" << std::endl;

    auto data = loadTestData();
    assert(data.getAllTrips().size() == 32);

    data.addTrip({33, TransportType::TRAIN, 1, 2, 500, 700, 200, "G999"});
    assert(data.getAllTrips().size() == 33);

    data.removeTrip(33);
    assert(data.getAllTrips().size() == 32);

    // 删除不存在的班次 → 无变化
    data.removeTrip(999);
    assert(data.getAllTrips().size() == 32);

    // 删除 Trip1 后，北京出发火车减少 1 条
    auto trips_before = data.getDeparturesInWindow(1, 0, 1439, TransportType::TRAIN);
    size_t n_before = trips_before.size();
    data.removeTrip(1);
    auto trips_after = data.getDeparturesInWindow(1, 0, 1439, TransportType::TRAIN);
    assert(trips_after.size() == n_before - 1);

    std::cout << "[PASS] 增删班次测试通过\n" << std::endl;
}

// =============================================
// 测试 4：时间窗口过滤
// =============================================

void test_departures_in_window() {
    std::cout << "--- 测试时间窗口过滤 ---" << std::endl;

    auto data = loadTestData();

    // 北京 TRAIN [400,500] → G1(420), G67(450), G79(480) 共 3 条
    auto trips = data.getDeparturesInWindow(1, 400, 500, TransportType::TRAIN);
    assert(trips.size() == 3);
    assert(trips[0].departure_time_ == 420);
    assert(trips[1].departure_time_ == 450);
    assert(trips[2].departure_time_ == 480);
    std::cout << "  北京 TRAIN [400,500]: 3 条（dep=420/450/480）" << std::endl;

    // 北京 TRAIN [100,200] → 空
    trips = data.getDeparturesInWindow(1, 100, 200, TransportType::TRAIN);
    assert(trips.empty());
    std::cout << "  北京 TRAIN [100,200]: 0 条" << std::endl;

    // 北京 PLANE [400,600] → CA1234(420), CZ3000(480), CZ5000(540) 共 3 条
    trips = data.getDeparturesInWindow(1, 400, 600, TransportType::PLANE);
    assert(trips.size() == 3);
    assert(trips[0].departure_time_ == 420);
    assert(trips[2].departure_time_ == 540);
    std::cout << "  北京 PLANE [400,600]: 3 条（dep=420/480/540）" << std::endl;

    // 广州 TRAIN [400,500] → G9701(420), G86(420), G1102(480) 共 3 条
    trips = data.getDeparturesInWindow(3, 400, 500, TransportType::TRAIN);
    assert(trips.size() == 3);
    std::cout << "  广州 TRAIN [400,500]: 3 条" << std::endl;

    // 边界值：[420,420] → G1(420) 1 条
    trips = data.getDeparturesInWindow(1, 420, 420, TransportType::TRAIN);
    assert(trips.size() == 1);
    assert(trips[0].id_ == 1);
    std::cout << "  北京 TRAIN [420,420]: 1 条（边界 inclusive）" << std::endl;

    // 验证排序：武汉 TRAIN [500,900] → G68(840), G1101(840), G2184(840) 共 3 条
    trips = data.getDeparturesInWindow(6, 500, 900, TransportType::TRAIN);
    assert(trips.size() == 3);
    std::cout << "  武汉 TRAIN [500,900]: 3 条" << std::endl;

    std::cout << "[PASS] 时间窗口过滤测试通过\n" << std::endl;
}

// =============================================
// 测试 5：指定时间后过滤
// =============================================

void test_departures_after() {
    std::cout << "--- 测试指定时间后过滤 ---" << std::endl;

    auto data = loadTestData();

    // 北京 TRAIN after 400 → G1(420), G79(480), G67(450) 共 3 条（G89 dep=360 被排除）
    auto trips = data.getDeparturesAfter(1, 400, TransportType::TRAIN);
    assert(trips.size() == 3);
    std::cout << "  北京 TRAIN after 400: 3 条" << std::endl;

    // 北京 TRAIN after 500 → 空（最晚 G79 在 480）
    trips = data.getDeparturesAfter(1, 500, TransportType::TRAIN);
    assert(trips.empty());
    std::cout << "  北京 TRAIN after 500: 0 条" << std::endl;

    // 北京 PLANE after 400 → CA1234(420), CZ3000(480), CZ5000(540), CA8201(1020) 共 4 条
    trips = data.getDeparturesAfter(1, 400, TransportType::PLANE);
    assert(trips.size() == 4);
    std::cout << "  北京 PLANE after 400: 4 条" << std::endl;

    // 深圳 PLANE after 0 → CA4377(840), CZ5100(960) 共 2 条
    trips = data.getDeparturesAfter(4, 0, TransportType::PLANE);
    assert(trips.size() == 2);
    std::cout << "  深圳 PLANE after 0: 2 条" << std::endl;

    // 边界值 after 420 → G1(420), G79(480), G67(450) 共 3 条（420 inclusive）
    trips = data.getDeparturesAfter(1, 420, TransportType::TRAIN);
    assert(trips.size() == 3);
    std::cout << "  北京 TRAIN after 420: 3 条（边界 inclusive）" << std::endl;

    std::cout << "[PASS] 指定时间后过滤测试通过\n" << std::endl;
}

// =============================================
// 测试 6：修改后查询正确性
// =============================================

void test_after_modification() {
    std::cout << "--- 测试修改后查询正确性 ---" << std::endl;

    auto data = loadTestData();

    // 初始：北京 [400,500] TRAIN → 3 条
    auto trips = data.getDeparturesInWindow(1, 400, 500, TransportType::TRAIN);
    size_t n_before = trips.size();

    // 删除 G1 后 → 减 1
    data.removeTrip(1);
    trips = data.getDeparturesInWindow(1, 400, 500, TransportType::TRAIN);
    assert(trips.size() == n_before - 1);
    std::cout << "  删除 G1 后查询返回 " << (n_before - 1) << " 条" << std::endl;

    // 重新添加同班次 → 恢复
    data.addTrip({1, TransportType::TRAIN, 1, 2, 420, 706, 553, "G1"});
    trips = data.getDeparturesInWindow(1, 400, 500, TransportType::TRAIN);
    assert(trips.size() == n_before);
    std::cout << "  重新添加 G1 后查询返回 " << n_before << " 条" << std::endl;

    std::cout << "[PASS] 修改后查询正确性测试通过\n" << std::endl;
}

// =============================================
// 测试 7：交通工具类型过滤
// =============================================

void test_type_filtering() {
    std::cout << "--- 测试交通工具类型过滤 ---" << std::endl;

    auto data = loadTestData();

    // 北京 [0,1439] TRAIN → 4 条（G1, G79, G89, G67）
    auto trains = data.getDeparturesInWindow(1, 0, 1439, TransportType::TRAIN);
    assert(trains.size() == 4);
    assert(std::all_of(trains.begin(), trains.end(),
                       [](const Trip& t) { return t.type_ == TransportType::TRAIN; }));

    // 北京 [0,1439] PLANE → 4 条（CA1234, CZ3000, CZ5000, CA8201）
    auto planes = data.getDeparturesInWindow(1, 0, 1439, TransportType::PLANE);
    assert(planes.size() == 4);
    assert(std::all_of(planes.begin(), planes.end(),
                       [](const Trip& t) { return t.type_ == TransportType::PLANE; }));

    std::cout << "  北京 TRAIN: 4 条, PLANE: 4 条（类型不串）" << std::endl;

    // 广州 [0,1439] PLANE → 2 条（CZ4000, CZ6000）
    planes = data.getDeparturesInWindow(3, 0, 1439, TransportType::PLANE);
    assert(planes.size() == 2);

    // 广州 [0,1439] TRAIN → 4 条（G80, G86, G9701, G1102）
    trains = data.getDeparturesInWindow(3, 0, 1439, TransportType::TRAIN);
    assert(trains.size() == 4);
    std::cout << "  广州 TRAIN: 4 条, PLANE: 2 条" << std::endl;

    // 深圳 [0,1439] PLANE → 2 条（CA4377, CZ5100）
    planes = data.getDeparturesInWindow(4, 0, 1439, TransportType::PLANE);
    assert(planes.size() == 2);
    std::cout << "  深圳 PLANE: 2 条" << std::endl;

    std::cout << "[PASS] 交通工具类型过滤测试通过\n" << std::endl;
}

// =============================================
// 主函数
// =============================================

int main() {
    std::cout << "========================================\n";
    std::cout << "  数据层测试\n";
    std::cout << "========================================\n" << std::endl;

    test_load_basic();
    test_city_operations();
    test_trip_operations();
    test_departures_in_window();
    test_departures_after();
    test_after_modification();
    test_type_filtering();

    std::cout << "========================================\n";
    std::cout << "  全部数据层测试通过!\n";
    std::cout << "========================================\n";
    return 0;
}
