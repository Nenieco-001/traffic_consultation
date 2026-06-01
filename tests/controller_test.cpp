// ConsultController 测试
// 覆盖：基本查询（三种策略）、无解处理、城市/班次增删管理
// 数据源：tests_data/algorithm_test/
//
// 城市: 1=北京 2=上海 3=广州 4=深圳 5=孤岛
// 班次: G1/G2/G3 北京→上海, G4/G5 上海→广州, G6 广州→深圳, G7 北京→深圳(直达), CZ8888 北京→广州(航班)

#include "application/consult_controller.h"

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <string>

// =============================================
// 辅助函数
// =============================================

static std::string testDataDir() {
    return std::string(PROJECT_ROOT) + "/tests_data/algorithm_test";
}

// 验证路径不变量
static void assertPathValid(const Path& path, int expected_segments) {
    assert(path.segments_.size() == static_cast<size_t>(expected_segments));
    assert(path.transfer_count_ == expected_segments - 1);
    assert(path.total_time_ > 0);
    assert(path.total_price_ > 0);
    for (const auto& seg : path.segments_) {
        assert(seg.wait_time_ >= 0);
    }
}

// 构建查询请求
static QueryRequest makeReq(int from, int to, Strategy s, TransportType t, int depart) {
    QueryRequest req;
    req.from_city_id_ = from;
    req.to_city_id_ = to;
    req.strategy_ = s;
    req.transport_ = t;
    req.depart_after_ = depart;
    return req;
}

// =============================================
// C1: 基本查询 FASTEST — 北京(1)→上海(2) TRAIN
// =============================================
// 期望：G1(420→660, 553元), total_time=260

static void test_C1_fastest_query() {
    std::cout << "--- C1: 基本查询 FASTEST 北京→上海 TRAIN ---" << std::endl;

    ConsultController ctrl;
    ctrl.loadData(testDataDir());

    auto req = makeReq(1, 2, Strategy::FASTEST, TransportType::TRAIN, 400);
    QueryResult result = ctrl.query(req);

    assert(result.has_solution());
    assert(result.error_msg.empty());
    assert(result.paths.size() == 1);

    const auto& path = result.paths[0];
    assertPathValid(path, 1);
    assert(path.segments_[0].trip_.id_ == 1);    // G1
    assert(path.segments_[0].trip_.trip_number_ == "G1");
    assert(path.segments_[0].wait_time_ == 20);
    assert(path.total_time_ == 260);
    assert(path.total_price_ == 553);

    std::cout << "  [PASS] G1: depart 420, arrive 660, total=260, price=553" << std::endl;
}

// =============================================
// C2: 基本查询 CHEAPEST — 北京(1)→上海(2) TRAIN
// =============================================
// 期望：G3(840→1020, 500元), total_price=500

static void test_C2_cheapest_query() {
    std::cout << "--- C2: 基本查询 CHEAPEST 北京→上海 TRAIN ---" << std::endl;

    ConsultController ctrl;
    ctrl.loadData(testDataDir());

    auto req = makeReq(1, 2, Strategy::CHEAPEST, TransportType::TRAIN, 400);
    QueryResult result = ctrl.query(req);

    assert(result.has_solution());
    assert(result.paths.size() == 1);

    const auto& path = result.paths[0];
    assertPathValid(path, 1);
    assert(path.segments_[0].trip_.id_ == 3);    // G3
    assert(path.segments_[0].trip_.trip_number_ == "G3");
    assert(path.total_price_ == 500);

    std::cout << "  [PASS] G3: depart 840, arrive 1020, total=620, price=500" << std::endl;
}

// =============================================
// C3: 基本查询 LEAST_TRANSFERS — 北京(1)→深圳(4) TRAIN
// =============================================
// 期望：G7(600→1080, 2000元) 直达, transfer=0

static void test_C3_least_transfer_query() {
    std::cout << "--- C3: 基本查询 LEAST_TRANSFERS 北京→深圳 TRAIN ---" << std::endl;

    ConsultController ctrl;
    ctrl.loadData(testDataDir());

    auto req = makeReq(1, 4, Strategy::LEAST_TRANSFERS, TransportType::TRAIN, 400);
    QueryResult result = ctrl.query(req);

    assert(result.has_solution());
    assert(result.paths.size() == 1);

    const auto& path = result.paths[0];
    assertPathValid(path, 1);
    assert(path.segments_[0].trip_.id_ == 7);    // G7
    assert(path.segments_[0].trip_.trip_number_ == "G7");
    assert(path.transfer_count_ == 0);
    assert(path.total_price_ == 2000);

    std::cout << "  [PASS] G7 直达: depart 600, arrive 1080, total=680, price=2000, transfer=0" << std::endl;
}

// =============================================
// C4: MIXED + FASTEST 跨类型查询 — 北京(1)→广州(3)
// =============================================
// 期望：CZ8888 航班(480→660)比任何火车快

static void test_C4_mixed_fastest() {
    std::cout << "--- C4: MIXED FASTEST 北京→广州 ---" << std::endl;

    ConsultController ctrl;
    ctrl.loadData(testDataDir());

    auto req = makeReq(1, 3, Strategy::FASTEST, TransportType::MIXED, 400);
    QueryResult result = ctrl.query(req);

    assert(result.has_solution());
    assert(result.paths.size() == 1);

    const auto& path = result.paths[0];
    assertPathValid(path, 1);
    assert(path.segments_[0].trip_.id_ == 8);    // CZ8888
    assert(path.segments_[0].trip_.trip_number_ == "CZ8888");
    assert(path.total_time_ == 260);
    assert(path.total_price_ == 1500);

    std::cout << "  [PASS] 航班 CZ8888: depart 480, arrive 660, total=260, price=1500" << std::endl;
}

// =============================================
// C5: 无解查询 — 北京(1)→孤岛(5)
// =============================================
// 期望：error_msg 非空, has_solution = false

static void test_C5_no_solution() {
    std::cout << "--- C5: 无解查询 北京→孤岛 ---" << std::endl;

    ConsultController ctrl;
    ctrl.loadData(testDataDir());

    auto req = makeReq(1, 5, Strategy::FASTEST, TransportType::TRAIN, 400);
    QueryResult result = ctrl.query(req);

    assert(!result.has_solution());
    assert(!result.error_msg.empty());
    assert(result.paths.empty());

    std::cout << "  [PASS] 无解提示: \"" << result.error_msg << "\"" << std::endl;
}

// =============================================
// C6: 添加城市 + 查询 — 添加新区, 确认数据层可见
// =============================================

static void test_C6_add_city() {
    std::cout << "--- C6: 添加城市 ---" << std::endl;

    ConsultController ctrl;
    ctrl.loadData(testDataDir());

    size_t old_count = ctrl.getData().getAllCities().size();
    ctrl.addCity(City{99, "NewCity"});
    size_t new_count = ctrl.getData().getAllCities().size();

    assert(new_count == old_count + 1);

    // 验证新增城市存在
    bool found = false;
    for (const auto& c : ctrl.getData().getAllCities()) {
        if (c.id_ == 99 && c.name_ == "NewCity") {
            found = true;
            break;
        }
    }
    assert(found);

    std::cout << "  [PASS] 城市数: " << old_count << " → " << new_count << ", 新增城市 id=99 已存在" << std::endl;
}

// =============================================
// C7: 删除城市 — 删除后数据层不可见
// =============================================

static void test_C7_remove_city() {
    std::cout << "--- C7: 删除城市 ---" << std::endl;

    ConsultController ctrl;
    ctrl.loadData(testDataDir());

    size_t old_count = ctrl.getData().getAllCities().size();
    ctrl.removeCity(5);  // 孤岛, 无关联班次
    size_t new_count = ctrl.getData().getAllCities().size();

    assert(new_count == old_count - 1);

    // 验证已删除
    for (const auto& c : ctrl.getData().getAllCities()) {
        assert(c.id_ != 5);
    }

    std::cout << "  [PASS] 城市数: " << old_count << " → " << new_count << ", id=5 已删除" << std::endl;
}

// =============================================
// C8: 添加班次 + 查询 — 添加新班次后查询验证
// =============================================
// 添加 孤岛(5)→北京(1) 班次, 然后查询应可达

static void test_C8_add_trip() {
    std::cout << "--- C8: 添加班次 + 查询 ---" << std::endl;

    ConsultController ctrl;
    ctrl.loadData(testDataDir());

    // 先添加孤岛到北京的班次
    Trip new_trip;
    new_trip.id_ = 100;
    new_trip.type_ = TransportType::TRAIN;
    new_trip.from_city_id_ = 5;   // 孤岛
    new_trip.to_city_id_ = 1;     // 北京
    new_trip.departure_time_ = 600;
    new_trip.arrival_time_ = 900;
    new_trip.price_ = 300;
    new_trip.trip_number_ = "TEST1";

    size_t old_trip_count = ctrl.getData().getAllTrips().size();
    ctrl.addTrip(new_trip);
    size_t new_trip_count = ctrl.getData().getAllTrips().size();
    assert(new_trip_count == old_trip_count + 1);

    // 现在孤岛→北京应有路径
    auto req = makeReq(5, 1, Strategy::FASTEST, TransportType::TRAIN, 400);
    QueryResult result = ctrl.query(req);
    assert(result.has_solution());
    assertPathValid(result.paths[0], 1);
    assert(result.paths[0].segments_[0].trip_.id_ == 100);

    std::cout << "  [PASS] 添加班次 TEST1 后, 孤岛→北京 查询有解" << std::endl;
}

// =============================================
// C9: 删除班次 — 删除后查询受影响
// =============================================
// 删除 G7(北京→深圳直达), 则查询需改走换乘

static void test_C9_remove_trip() {
    std::cout << "--- C9: 删除班次 + 查询受影响 ---" << std::endl;

    ConsultController ctrl;
    ctrl.loadData(testDataDir());

    // 删除 G7 (id=7), 北京→深圳直达消失
    ctrl.removeTrip(7);

    // query 北京→深圳 LEAST_TRANSFERS, 应不再是 G7 直达
    auto req = makeReq(1, 4, Strategy::LEAST_TRANSFERS, TransportType::TRAIN, 400);
    QueryResult result = ctrl.query(req);

    assert(result.has_solution());
    // 应走换乘路径（至少 2 段）, 不应是 G7
    for (const auto& seg : result.paths[0].segments_) {
        assert(seg.trip_.id_ != 7);
    }
    assert(result.paths[0].transfer_count_ >= 1);

    // 确认数据层已无 id=7
    for (const auto& t : ctrl.getData().getAllTrips()) {
        assert(t.id_ != 7);
    }

    std::cout << "  [PASS] 删除 G7 后, 北京→深圳 最少换乘需" << result.paths[0].transfer_count_ << " 次换乘 (原为 0)" << std::endl;
}

// =============================================
// C10: 城市不存在时 query 抛出异常
// =============================================

static void test_C10_query_invalid_city() {
    std::cout << "--- C10: query 不存在城市 ---" << std::endl;

    ConsultController ctrl;
    ctrl.loadData(testDataDir());

    auto req = makeReq(1, 999, Strategy::FASTEST, TransportType::TRAIN, 400);

    bool threw = false;
    try {
        ctrl.query(req);
    } catch (const std::invalid_argument& e) {
        threw = true;
        std::cout << "  [INFO] 捕获异常: " << e.what() << std::endl;
    }

    assert(threw);
    std::cout << "  [PASS] 城市不存在时正确抛出异常" << std::endl;
}

// =============================================
// 主函数
// =============================================

int main() {
    std::cout << "========================================\n";
    std::cout << "  ConsultController 测试\n";
    std::cout << "========================================\n" << std::endl;

    test_C1_fastest_query();
    test_C2_cheapest_query();
    test_C3_least_transfer_query();
    test_C4_mixed_fastest();
    test_C5_no_solution();
    test_C6_add_city();
    test_C7_remove_city();
    test_C8_add_trip();
    test_C9_remove_trip();
    test_C10_query_invalid_city();

    std::cout << "\n========================================\n";
    std::cout << "  全部 Controller 测试通过!\n";
    std::cout << "========================================\n";
    return 0;
}
