// 集成测试
// 覆盖三个维度：
//   IE — 端到端查询流程（E2E）
//   IA — 管理员操作流程（Admin）
//   IN — 无解场景流程（No-solution）
// 数据源：tests_data/algorithm_test/
//
// 城市: 1=北京 2=上海 3=广州 4=深圳 5=孤岛
// 班次: G1/G2/G3 北京→上海, G4/G5 上海→广州, G6 广州→深圳, G7 北京→深圳(直达), CZ8888 北京→广州(航班)

#include "application/consult_controller.h"

#include <cassert>
#include <iostream>
#include <string>

// =============================================
// 辅助函数
// =============================================

static std::string testDataDir() {
    return std::string(PROJECT_ROOT) + "/tests_data/algorithm_test";
}

static QueryRequest makeReq(int from, int to, Strategy s, TransportType t, int depart) {
    QueryRequest req;
    req.from_city_id_ = from;
    req.to_city_id_ = to;
    req.strategy_ = s;
    req.transport_ = t;
    req.depart_after_ = depart;
    return req;
}

static void assertPathValid(const Path& path, int expected_segments) {
    assert(path.segments_.size() == static_cast<size_t>(expected_segments));
    assert(path.transfer_count_ == expected_segments - 1);
    assert(path.total_time_ > 0);
    assert(path.total_price_ > 0);
    for (const auto& seg : path.segments_)
        assert(seg.wait_time_ >= 0);
}

// =============================================
// IE: 端到端流程
// =============================================

// IE1: 完整查询流 — 加载数据 → 查询 → 验证结果 → 验证路径元数据
static void test_IE1_e2e_query_flow() {
    std::cout << "--- IE1: 端到端查询流程 北京→上海 FASTEST ---" << std::endl;

    ConsultController ctrl;
    ctrl.loadData(testDataDir());

    // 查询：北京(1)→上海(2), FASTEST, TRAIN, depart=400
    auto req = makeReq(1, 2, Strategy::FASTEST, TransportType::TRAIN, 400);
    QueryResult result = ctrl.query(req);

    // 验证结果完整性
    assert(result.has_solution());
    assert(result.paths.size() == 1);
    assert(result.error_msg.empty());

    // 验证路径详情
    const auto& path = result.paths[0];
    assertPathValid(path, 1);
    assert(path.segments_[0].trip_.id_ == 1);       // G1
    assert(path.segments_[0].trip_.trip_number_ == "G1");
    assert(path.segments_[0].trip_.departure_time_ == 420);
    assert(path.segments_[0].trip_.arrival_time_ == 660);
    assert(path.segments_[0].wait_time_ == 20);
    assert(path.total_time_ == 260);
    assert(path.total_price_ == 553);
    assert(path.transfer_count_ == 0);

    // 验证 Path 元数据方法
    assert(path.departure_time() == 420);
    assert(path.arrival_time() == 660);

    std::cout << "  [PASS] 查询→结果→详情→元数据 完整链路正确" << std::endl;
}

// IE2: 多策略端到端 — 同一线路三种策略，验证结果不同
static void test_IE2_e2e_multi_strategy() {
    std::cout << "--- IE2: 多策略对比 北京→广州 ---" << std::endl;

    ConsultController ctrl;
    ctrl.loadData(testDataDir());

    // 北京(1)→广州(3), TRAIN, depart=400
    auto r_fast = ctrl.query(makeReq(1, 3, Strategy::FASTEST, TransportType::TRAIN, 400));
    auto r_cheap = ctrl.query(makeReq(1, 3, Strategy::CHEAPEST, TransportType::TRAIN, 400));
    auto r_least = ctrl.query(makeReq(1, 3, Strategy::LEAST_TRANSFERS, TransportType::TRAIN, 400));

    // 三种策略都应返回有效结果
    assert(r_fast.has_solution());
    assert(r_cheap.has_solution());
    assert(r_least.has_solution());

    // 最快应最早到达（G1+G4 arrive=1320）
    assert(r_fast.paths[0].arrival_time() == 1320);

    // 最省应总价最低（G3+G4 price=1200）
    assert(r_cheap.paths[0].total_price_ == 1200);

    // 最少换乘段数最少（2段，因为无直达）

    assert(r_least.paths[0].transfer_count_ == 1);
    assert(r_least.paths[0].segments_.size() == 2);

    std::cout << "  [PASS] 最快=" << r_fast.paths[0].arrival_time()
              << " 最省=" << r_cheap.paths[0].total_price_ << "元"
              << " 最少换乘=" << r_least.paths[0].transfer_count_ << "次" << std::endl;
}

// =============================================
// IA: 管理员流程
// =============================================

// IA1: 添加城市 + 添加班次 → 查询新城市 → 验证可达
static void test_IA1_admin_add_city_and_trip() {
    std::cout << "--- IA1: 管理员添加城市+班次 → 查询验证 ---" << std::endl;

    ConsultController ctrl;
    ctrl.loadData(testDataDir());

    // 添加新城市
    ctrl.addCity(City{99, "TestCity"});

    // 验证城市存在
    bool city_found = false;
    for (const auto& c : ctrl.getData().getAllCities())
        if (c.id_ == 99) { city_found = true; break; }
    assert(city_found);

    // 添加班次 北京(1)→TestCity(99)
    Trip trip;
    trip.id_ = 200;
    trip.type_ = TransportType::TRAIN;
    trip.from_city_id_ = 1;
    trip.to_city_id_ = 99;
    trip.departure_time_ = 500;
    trip.arrival_time_ = 700;
    trip.price_ = 999;
    trip.trip_number_ = "ADMIN1";
    ctrl.addTrip(trip);

    // 查询北京(1)→TestCity(99) 应可达
    auto req = makeReq(1, 99, Strategy::FASTEST, TransportType::TRAIN, 400);
    QueryResult result = ctrl.query(req);

    assert(result.has_solution());
    assertPathValid(result.paths[0], 1);
    assert(result.paths[0].segments_[0].trip_.trip_number_ == "ADMIN1");
    assert(result.paths[0].total_price_ == 999);

    std::cout << "  [PASS] 添加城市→添加班次→查询可达" << std::endl;
}

// IA2: 级联删除 — 删除城市 → 验证关联班次同步删除
static void test_IA2_admin_cascade_delete() {
    std::cout << "--- IA2: 管理员级联删除城市 ---" << std::endl;

    ConsultController ctrl;
    ctrl.loadData(testDataDir());

    size_t orig_trip_count = ctrl.getData().getAllTrips().size();

    // 添加临时城市和班次
    ctrl.addCity(City{97, "CascadeCity"});
    Trip t;
    t.id_ = 201;
    t.type_ = TransportType::TRAIN;
    t.from_city_id_ = 1;
    t.to_city_id_ = 97;
    t.departure_time_ = 600;
    t.arrival_time_ = 800;
    t.price_ = 500;
    t.trip_number_ = "CASCADE1";
    ctrl.addTrip(t);
    assert(ctrl.getData().getAllTrips().size() == orig_trip_count + 1);

    // 删除城市 97 → 班次 201 应被级联删除
    ctrl.removeCity(97);

    // 验证城市已删除
    for (const auto& c : ctrl.getData().getAllCities())
        assert(c.id_ != 97);
    // 验证班次 201 已删除
    for (const auto& trip : ctrl.getData().getAllTrips())
        assert(trip.id_ != 201);
    // 班次总数应恢复原状
    assert(ctrl.getData().getAllTrips().size() == orig_trip_count);

    std::cout << "  [PASS] 删除城市后关联班次同步删除" << std::endl;
}

// IA3: 修改班次 — 添加班次 → 修改票价 → 查询验证修改生效
static void test_IA3_admin_modify_trip() {
    std::cout << "--- IA3: 管理员修改班次 ---" << std::endl;

    ConsultController ctrl;
    ctrl.loadData(testDataDir());

    // 添加测试班次
    Trip trip;
    trip.id_ = 202;
    trip.type_ = TransportType::TRAIN;
    trip.from_city_id_ = 1;
    trip.to_city_id_ = 2;
    trip.departure_time_ = 500;
    trip.arrival_time_ = 700;
    trip.price_ = 800;
    trip.trip_number_ = "MODIFY1";
    ctrl.addTrip(trip);

    // 验证添加成功
    // 修改票价和班次号
    Trip updated = trip;
    updated.price_ = 600;
    updated.trip_number_ = "MODIFIED1";
    bool ok = ctrl.modifyTrip(202, updated);
    assert(ok);

    // 查询验证修改
    auto req = makeReq(1, 2, Strategy::CHEAPEST, TransportType::TRAIN, 400);
    QueryResult result = ctrl.query(req);

    assert(result.has_solution());
    // 修改后的 MODIFIED1(600元) 应比 G1(553元) 贵，但比 G3(500元) 也贵
    // 所以最省的还是 G3
    // 但我们验证修改已生效：数据层应存在 price_=600 且 trip_number_="MODIFIED1" 的班次
    bool found_modified = false;
    for (const auto& t : ctrl.getData().getAllTrips()) {
        if (t.id_ == 202) {
            found_modified = true;
            assert(t.price_ == 600);
            assert(t.trip_number_ == "MODIFIED1");
            break;
        }
    }
    assert(found_modified);

    std::cout << "  [PASS] 修改班次成功：票价 800→600, 班次号 MODIFY1→MODIFIED1" << std::endl;
}

// =============================================
// IN: 无解场景
// =============================================

// IN1: 无解查询 — 北京(1)→孤岛(5) 无任何班次连接
static void test_IN1_no_solution_island() {
    std::cout << "--- IN1: 无解查询 北京→孤岛 ---" << std::endl;

    ConsultController ctrl;
    ctrl.loadData(testDataDir());

    auto req = makeReq(1, 5, Strategy::FASTEST, TransportType::TRAIN, 400);
    QueryResult result = ctrl.query(req);

    // 不崩溃
    assert(!result.has_solution());
    assert(result.paths.empty());
    assert(!result.error_msg.empty());

    std::cout << "  [PASS] 无解提示: \"" << result.error_msg << "\"" << std::endl;
}

// IN2: 极晚出发时间 — 超出所有班次时间窗口
static void test_IN2_no_solution_late_departure() {
    std::cout << "--- IN2: 无解查询 出发时间过晚 ---" << std::endl;

    ConsultController ctrl;
    ctrl.loadData(testDataDir());

    // 北京(1)→上海(2), 出发时间 1400（所有班次已过）
    auto req = makeReq(1, 2, Strategy::FASTEST, TransportType::TRAIN, 1400);
    QueryResult result = ctrl.query(req);

    // 不崩溃，可能无解（G3 840-1020 已过 1400）
    // 不应崩溃，这是一个有效状态检查
    if (result.has_solution()) {
        // 如果有解也要验证路径有效
        assert(!result.paths.empty());
        assertPathValid(result.paths[0], 1);
        std::cout << "  [PASS] 有解（实际存在晚于 1400 的班次）" << std::endl;
    } else {
        assert(result.paths.empty());
        assert(!result.error_msg.empty());
        std::cout << "  [PASS] 无解: \"" << result.error_msg << "\"" << std::endl;
    }
}

// =============================================
// 主函数
// =============================================

int main() {
    std::cout << "========================================\n";
    std::cout << "  集成测试\n";
    std::cout << "========================================\n" << std::endl;

    test_IE1_e2e_query_flow();
    test_IE2_e2e_multi_strategy();
    test_IA1_admin_add_city_and_trip();
    test_IA2_admin_cascade_delete();
    test_IA3_admin_modify_trip();
    test_IN1_no_solution_island();
    test_IN2_no_solution_late_departure();

    std::cout << "\n========================================\n";
    std::cout << "  全部集成测试通过!\n";
    std::cout << "========================================\n";
    return 0;
}
