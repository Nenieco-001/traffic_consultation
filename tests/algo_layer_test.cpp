// 算法层测试
// IO 层读取 tests_data/algo_test/.dat → 数据层 → 算法层
// 所有测试 depart_time = 400 (06:40)

#include "algo/dijkstra.h"
#include "data/transport_data.h"
#include "io/file_io.h"

#include <cassert>
#include <iostream>
#include <string>

// =============================================
// 辅助函数
// =============================================

static TransportData loadTestData() {
    std::string dir = std::string(PROJECT_ROOT) + "/tests_data/algo_test";
    return file_io::loadFromFile(dir);
}

// 验证路径基本不变量
static void assertPathInvariants(const Path& path) {
    if (!path.segments_.empty()) {
        assert(path.total_time_ > 0);
        assert(path.total_price_ > 0);
        assert(path.transfer_count_ == static_cast<int>(path.segments_.size()) - 1);
        for (const auto& seg : path.segments_) {
            assert(seg.wait_time_ >= 0);
        }
    }
}

// =============================================
// T1: 直达最快 — 北京(1)→上海(2)
// =============================================
// 3 趟列车 G1(420-660,553元)、G2(600-780,633元)、G3(840-1020,500元)
// 出发 400：G1 总耗时=20+240=260（最早到 660），G2=380，G3=620
// 期望：G1，arrival=660, total_time=260, total_price=553

static void test_T1_fastest_direct() {
    std::cout << "--- T1: 直达最快 北京→上海 ---" << std::endl;

    auto data = loadTestData();
    auto cities = data.getAllCities();
    Path path = algo::findFastestPath(data, cities[0], cities[1], 400, TransportType::TRAIN);

    assert(path.segments_.size() == 1);
    assert(path.segments_[0].trip_.id_ == 1);
    assert(path.segments_[0].trip_.trip_number_ == "G1");
    assert(path.segments_[0].wait_time_ == 20);
    assert(path.total_time_ == 260);
    assert(path.total_price_ == 553);
    assert(path.transfer_count_ == 0);
    assertPathInvariants(path);

    std::cout << "  [PASS] G1: depart 420, arrive 660, total=260, price=553" << std::endl;
}

// =============================================
// T2: 直达最省 — 北京(1)→上海(2)
// =============================================
// G3 票价最低(500元)
// 期望：G3，total_price=500，wait=440(840-400)

static void test_T2_cheapest_direct() {
    std::cout << "--- T2: 直达最省 北京→上海 ---" << std::endl;

    auto data = loadTestData();
    auto cities = data.getAllCities();
    Path path = algo::findCheapestPath(data, cities[0], cities[1], 400, TransportType::TRAIN);

    assert(path.segments_.size() == 1);
    assert(path.segments_[0].trip_.id_ == 3);
    assert(path.segments_[0].trip_.trip_number_ == "G3");
    assert(path.segments_[0].wait_time_ == 440);
    assert(path.total_time_ == 620);
    assert(path.total_price_ == 500);
    assert(path.transfer_count_ == 0);
    assertPathInvariants(path);

    std::cout << "  [PASS] G3: depart 840, arrive 1020, total=620, price=500" << std::endl;
}

// =============================================
// T3: 一次换乘最快 — 北京(1)→广州(3)，TRAIN 无直达
// =============================================
// 最快组合：G1(1→2,到 660) + G4(2→3,到 1320)
// 期望：arrival=1320, total_time=920, price=553+700=1253

static void test_T3_one_transfer_fastest() {
    std::cout << "--- T3: 一次换乘最快 北京→广州(TRAIN) ---" << std::endl;

    auto data = loadTestData();
    auto cities = data.getAllCities();
    Path path = algo::findFastestPath(data, cities[0], cities[2], 400, TransportType::TRAIN);

    assert(path.segments_.size() == 2);
    assert(path.segments_[0].trip_.id_ == 1);   // G1
    assert(path.segments_[0].trip_.trip_number_ == "G1");
    assert(path.segments_[0].wait_time_ == 20);
    assert(path.segments_[1].trip_.id_ == 4);   // G4
    assert(path.segments_[1].trip_.trip_number_ == "G4");
    assert(path.segments_[1].wait_time_ == 420);
    assert(path.transfer_count_ == 1);
    assert(path.total_time_ == 920);
    assert(path.total_price_ == 1253);
    assertPathInvariants(path);

    std::cout << "  [PASS] G1(→上海) + G4(→广州): arrive 1320, total=920, price=1253" << std::endl;
}

// =============================================
// T4: 一次换乘最省 — 北京(1)→广州(3)
// =============================================
// 最省组合：G3(500元) + G4(700元) = 1200元（G5 800元更贵）
// 期望：total_price=1200, 2 segments

static void test_T4_one_transfer_cheapest() {
    std::cout << "--- T4: 一次换乘最省 北京→广州 ---" << std::endl;

    auto data = loadTestData();
    auto cities = data.getAllCities();
    Path path = algo::findCheapestPath(data, cities[0], cities[2], 400, TransportType::TRAIN);

    assert(path.segments_.size() == 2);
    assert(path.segments_[0].trip_.id_ == 3);   // G3
    assert(path.segments_[0].wait_time_ == 440);
    assert(path.segments_[1].trip_.id_ == 4);   // G4
    assert(path.segments_[1].wait_time_ == 60);
    assert(path.transfer_count_ == 1);
    assert(path.total_price_ == 1200);
    assert(path.total_time_ == 920);
    assertPathInvariants(path);

    std::cout << "  [PASS] G3(500) + G4(700): arrive 1320, total=920, price=1200" << std::endl;
}

// =============================================
// T5: 多次换乘最省 — 北京(1)→深圳(4)
// =============================================
// G7 直达 2000 元太贵，G3+G4+G6=1300 元更省
// G3(500) + G4(700) + G6(100) = 1300, G4 到站即转 G6(wait=0)
// 期望：3 segments, total_price=1300

static void test_T5_multi_transfer_cheapest() {
    std::cout << "--- T5: 多次换乘最省 北京→深圳 ---" << std::endl;

    auto data = loadTestData();
    auto cities = data.getAllCities();
    Path path = algo::findCheapestPath(data, cities[0], cities[3], 400, TransportType::TRAIN);

    assert(path.segments_.size() == 3);
    assert(path.segments_[0].trip_.id_ == 3);   // G3
    assert(path.segments_[0].wait_time_ == 440);
    assert(path.segments_[1].trip_.id_ == 4);   // G4
    assert(path.segments_[1].wait_time_ == 60);
    assert(path.segments_[2].trip_.id_ == 6);   // G6
    assert(path.segments_[2].wait_time_ == 0);   // G4 到站即转 G6
    assert(path.transfer_count_ == 2);
    assert(path.total_price_ == 1300);
    assert(path.total_time_ == 980);
    assertPathInvariants(path);

    std::cout << "  [PASS] G3+G4+G6: arrive 1380, total=980, price=1300 (3 segments)" << std::endl;
}

// =============================================
// T6: 无可行路径 — 北京(1)→孤岛(5)
// =============================================
// Island 无任何班次进出，算法无法到达
// 期望：空 Path, segments 为空

static void test_T6_no_path() {
    std::cout << "--- T6: 无可行路径 北京→孤岛 ---" << std::endl;

    auto data = loadTestData();
    auto cities = data.getAllCities();
    // Island = cities[4] (id_=5)
    Path path = algo::findFastestPath(data, cities[0], cities[4], 400, TransportType::TRAIN);

    assert(path.segments_.empty());
    assert(path.total_time_ == 0);
    assert(path.total_price_ == 0);
    assert(path.transfer_count_ == 0);

    std::cout << "  [PASS] 无可达路径，返回空 Path" << std::endl;
}

// =============================================
// T7: 等待时间验证
// =============================================
// 跨用例验证所有 segment 的 wait_time ≥ 0 且符合预期

static void test_T7_wait_time() {
    std::cout << "--- T7: 等待时间验证 ---" << std::endl;

    auto data = loadTestData();
    auto cities = data.getAllCities();

    // 最快直达：wait=20（G1 420-400）
    Path p1 = algo::findFastestPath(data, cities[0], cities[1], 400, TransportType::TRAIN);
    assert(p1.segments_[0].wait_time_ == 20);

    // 一次换乘最快：wait=[20, 420]
    Path p3 = algo::findFastestPath(data, cities[0], cities[2], 400, TransportType::TRAIN);
    assert(p3.segments_[0].wait_time_ == 20);
    assert(p3.segments_[1].wait_time_ == 420);

    // 一次换乘最省：wait=[440, 60]（G3→G4）
    Path p4 = algo::findCheapestPath(data, cities[0], cities[2], 400, TransportType::TRAIN);
    assert(p4.segments_[0].wait_time_ == 440);
    assert(p4.segments_[1].wait_time_ == 60);

    // 多次换乘：wait=[440, 60, 0]（G6 同站台换乘，wait=0）
    Path p5 = algo::findCheapestPath(data, cities[0], cities[3], 400, TransportType::TRAIN);
    assert(p5.segments_[0].wait_time_ == 440);
    assert(p5.segments_[1].wait_time_ == 60);
    assert(p5.segments_[2].wait_time_ == 0);

    std::cout << "  [PASS] 所有等待时间验证通过（均 ≥0 且符合预期）" << std::endl;
}

// =============================================
// T8: 最少换乘 — 北京(1)→深圳(4)
// =============================================
// BFS 逐层扩展，G7(1→4 直达) 在第二层被找到
// 期望：1 segment, G7, transfer_count=0

static void test_T8_least_transfer() {
    std::cout << "--- T8: 最少换乘 北京→深圳 ---" << std::endl;

    auto data = loadTestData();
    auto cities = data.getAllCities();
    Path path = algo::findLeastTransferPath(data, cities[0], cities[3], 400, TransportType::TRAIN);

    assert(path.segments_.size() == 1);
    assert(path.segments_[0].trip_.id_ == 7);   // G7 直达
    assert(path.segments_[0].trip_.trip_number_ == "G7");
    assert(path.segments_[0].wait_time_ == 200);
    assert(path.transfer_count_ == 0);
    assert(path.total_price_ == 2000);
    assert(path.total_time_ == 680);
    assertPathInvariants(path);

    std::cout << "  [PASS] G7 直达(1 segment): arrive 1080, total=680, price=2000, transfer=0" << std::endl;
}

// =============================================
// T9: 跨交通工具(MIXED) — 北京(1)→广州(3)
// =============================================
// 最快：CZ8888 航班(480-660, 80+180=260min) 比火车组合快
// 最省：G3+G5 火车(900元) 比航班(1500元)便宜

static void test_T9_mixed_type() {
    std::cout << "--- T9a: MIXED 最快 北京→广州 ---" << std::endl;

    auto data = loadTestData();
    auto cities = data.getAllCities();

    // 最快：航班 CZ8888（arrive 660）
    Path fastest = algo::findFastestPath(data, cities[0], cities[2], 400, TransportType::MIXED);
    assert(fastest.segments_.size() == 1);
    assert(fastest.segments_[0].trip_.id_ == 8);
    assert(fastest.segments_[0].trip_.trip_number_ == "CZ8888");
    assert(fastest.segments_[0].wait_time_ == 80);
    assert(fastest.total_time_ == 260);
    assert(fastest.total_price_ == 1500);
    assert(fastest.transfer_count_ == 0);
    assertPathInvariants(fastest);
    std::cout << "  [PASS] 航班 CZ8888: arrive 660, total=260, price=1500 (最快)" << std::endl;

    std::cout << "--- T9b: MIXED 最省 北京→广州 ---" << std::endl;

    // 最省：G3+G4 火车组合（1200元）比航班（1500元）便宜
    Path cheapest = algo::findCheapestPath(data, cities[0], cities[2], 400, TransportType::MIXED);
    assert(cheapest.segments_.size() == 2);
    assert(cheapest.total_price_ == 1200);
    assert(cheapest.segments_[0].trip_.id_ == 3);
    assert(cheapest.segments_[1].trip_.id_ == 4);
    assert(cheapest.transfer_count_ == 1);
    assertPathInvariants(cheapest);
    std::cout << "  [PASS] G3+G4 火车: arrive 1320, total=920, price=1200 (最省)" << std::endl;
}

// =============================================
// 主函数
// =============================================

int main() {
    std::cout << "========================================\n";
    std::cout << "  算法层测试\n";
    std::cout << "========================================\n" << std::endl;

    test_T1_fastest_direct();
    test_T2_cheapest_direct();
    test_T3_one_transfer_fastest();
    test_T4_one_transfer_cheapest();
    test_T5_multi_transfer_cheapest();
    test_T6_no_path();
    test_T7_wait_time();
    test_T8_least_transfer();
    test_T9_mixed_type();

    std::cout << "========================================\n";
    std::cout << "  全部算法层测试通过!\n";
    std::cout << "========================================\n";
    return 0;
}
