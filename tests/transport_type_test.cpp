#include "model/transport_type.h"

#include <iostream>
#include <cassert>

void test_enum_values() {
    std::cout << "--- enum 值验证 ---" << std::endl;

    assert(static_cast<int>(TransportType::TRAIN) == 0);
    assert(static_cast<int>(TransportType::PLANE) == 1);
    std::cout << "TransportType::TRAIN = " << static_cast<int>(TransportType::TRAIN) << std::endl;
    std::cout << "TransportType::PLANE = " << static_cast<int>(TransportType::PLANE) << std::endl;

    assert(static_cast<int>(Strategy::FASTEST) == 0);
    assert(static_cast<int>(Strategy::CHEAPEST) == 1);
    assert(static_cast<int>(Strategy::LEAST_TRANSFERS) == 2);
    std::cout << "Strategy::FASTEST     = " << static_cast<int>(Strategy::FASTEST) << std::endl;
    std::cout << "Strategy::CHEAPEST    = " << static_cast<int>(Strategy::CHEAPEST) << std::endl;
    std::cout << "Strategy::LEAST_TRANSFERS = " << static_cast<int>(Strategy::LEAST_TRANSFERS) << std::endl;

    std::cout << "[PASS] enum 值验证通过\n" << std::endl;
}

void test_city() {
    std::cout << "--- City 结构体测试 ---" << std::endl;

    City city{1, "北京"};
    assert(city.id == 1);
    assert(city.name == "北京");

    City default_city{};
    assert(default_city.id == 0);
    assert(default_city.name.empty());

    std::cout << "city.id   = " << city.id << std::endl;
    std::cout << "city.name = " << city.name << std::endl;
    std::cout << "[PASS] City 结构体测试通过\n" << std::endl;
}

void test_trip() {
    std::cout << "--- Trip 结构体测试 ---" << std::endl;

    // 火车班次
    Trip train_trip{1, TransportType::TRAIN, 1, 2, 480, 780, 550, "G1"};
    assert(train_trip.id == 1);
    assert(train_trip.type == TransportType::TRAIN);
    assert(train_trip.from_city_id == 1);
    assert(train_trip.to_city_id == 2);
    assert(train_trip.departure_time == 480);   // 08:00
    assert(train_trip.arrival_time == 780);      // 13:00
    assert(train_trip.price == 550);
    assert(train_trip.trip_number == "G1");

    // 飞机班次
    Trip flight_trip{13, TransportType::PLANE, 1, 2, 420, 510, 1200, "CA1234"};
    assert(flight_trip.id == 13);
    assert(flight_trip.type == TransportType::PLANE);
    assert(flight_trip.trip_number == "CA1234");

    std::cout << "train: id=" << train_trip.id << " " << train_trip.trip_number
              << " " << train_trip.departure_time << "->" << train_trip.arrival_time
              << " ¥" << train_trip.price << std::endl;
    std::cout << "flight: id=" << flight_trip.id << " " << flight_trip.trip_number
              << " " << flight_trip.departure_time << "->" << flight_trip.arrival_time
              << " ¥" << flight_trip.price << std::endl;
    std::cout << "[PASS] Trip 结构体测试通过\n" << std::endl;
}

void test_segment() {
    std::cout << "--- Segment 结构体测试 ---" << std::endl;

    Trip trip{1, TransportType::TRAIN, 1, 2, 480, 780, 550, "G1"};

    // 首段等待时间为 0
    Segment first_segment{trip, 0};
    assert(first_segment.trip.id == 1);
    assert(first_segment.wait_time == 0);

    // 中转段等待时间 > 0
    Segment transfer_segment{trip, 30};
    assert(transfer_segment.wait_time == 30);

    std::cout << "first segment wait_time = " << first_segment.wait_time << std::endl;
    std::cout << "transfer segment wait_time = " << transfer_segment.wait_time << std::endl;
    std::cout << "[PASS] Segment 结构体测试通过\n" << std::endl;
}

void test_path_single_segment() {
    std::cout << "--- Path 单段路径测试 ---" << std::endl;

    Trip trip{1, TransportType::TRAIN, 1, 2, 480, 780, 550, "G1"};
    Segment seg{trip, 0};
    Path path{{seg}, 300, 550, 0};

    assert(path.segments.size() == 1);
    assert(path.total_time == 300);           // 780 - 480
    assert(path.total_price == 550);
    assert(path.transfer_count == 0);
    assert(path.departure_time() == 480);
    assert(path.arrival_time() == 780);

    std::cout << "total_time=" << path.total_time
              << " total_price=" << path.total_price
              << " transfer_count=" << path.transfer_count << std::endl;
    std::cout << "departure=" << path.departure_time() << " arrival=" << path.arrival_time() << std::endl;
    std::cout << "[PASS] Path 单段路径测试通过\n" << std::endl;
}

void test_path_multi_segment() {
    std::cout << "--- Path 多段路径测试 ---" << std::endl;

    // 成都(5)→武汉(4)：D633 08:00-12:00 ¥380
    Trip trip1{10, TransportType::TRAIN, 5, 4, 480, 720, 380, "D633"};
    // 武汉(4)→广州(3)：G1001 12:30-15:30 ¥460，等待 30 分钟
    Trip trip2{6, TransportType::TRAIN, 4, 3, 750, 930, 460, "G1001"};

    Segment seg1{trip1, 0};    // 首段无等待
    Segment seg2{trip2, 30};   // 武汉站等待 30 分钟

    int total_time = 930 - 480;         // 450 分钟
    int total_price = 380 + 460;         // ¥840
    int transfer_count = 1;             // 一次换乘

    Path path{{seg1, seg2}, total_time, total_price, transfer_count};

    // 验证字段
    assert(path.segments.size() == 2);
    assert(path.total_time == 450);
    assert(path.total_price == 840);
    assert(path.transfer_count == 1);

    // 验证派生方法
    assert(path.departure_time() == 480);   // 首段 D633 出发
    assert(path.arrival_time() == 930);     // 末段 G1001 到达

    // 验证每段详情
    assert(path.segments[0].trip.trip_number == "D633");
    assert(path.segments[0].wait_time == 0);
    assert(path.segments[1].trip.trip_number == "G1001");
    assert(path.segments[1].wait_time == 30);

    std::cout << "路线: 成都(5) -> 武汉(4) -> 广州(3)" << std::endl;
    std::cout << "  段1: " << path.segments[0].trip.trip_number
              << " " << path.segments[0].trip.departure_time
              << "->" << path.segments[0].trip.arrival_time << std::endl;
    std::cout << "  段2: " << path.segments[1].trip.trip_number
              << " " << path.segments[1].trip.departure_time
              << "->" << path.segments[1].trip.arrival_time
              << " (wait=" << path.segments[1].wait_time << ")" << std::endl;
    std::cout << "  总耗时=" << path.total_time << "min"
              << " 总票价=¥" << path.total_price
              << " 换乘=" << path.transfer_count << "次" << std::endl;
    std::cout << "[PASS] Path 多段路径测试通过\n" << std::endl;
}

void test_path_cross_day() {
    std::cout << "--- Path 跨天场景测试 ---" << std::endl;

    // 西安(6)→北京(1)：G666 14:00-18:00 ¥520
    Trip trip1{9, TransportType::TRAIN, 6, 1, 840, 1080, 520, "G666"};
    // 北京(1)→武汉(4)：G503 19:00-22:30 ¥480
    Trip trip2{11, TransportType::TRAIN, 1, 4, 1140, 1350, 480, "G503"};
    // 武汉(4)→广州(3)：G1009 06:30-09:30（次日）¥400
    Trip trip3{12, TransportType::TRAIN, 4, 3, 390, 570, 400, "G1009"};

    Segment seg1{trip1, 0};
    Segment seg2{trip2, 60};    // 北京站等 60 分钟
    // 第三段出发 390 < 第二段到达 1350，表示次日出发
    // 等待时间 = (390 + 1440) - 1350 = 480 分钟
    Segment seg3{trip3, 480};

    // arrival_time 需跨天处理: 570 + 1440 = 2010
    int total_time = (570 + 1440) - 840;  // 2010 - 840 = 1170
    int total_price = 520 + 480 + 400;     // ¥1400
    int transfer_count = 2;                // 两次换乘

    Path path{{seg1, seg2, seg3}, total_time, total_price, transfer_count};

    assert(path.segments.size() == 3);
    assert(path.total_time == 1170);
    assert(path.total_price == 1400);
    assert(path.transfer_count == 2);
    assert(path.departure_time() == 840);       // 首段 14:00
    assert(path.arrival_time() == 570);          // 末段原始到达 09:30（跨天由上层处理）

    std::cout << "路线: 西安(6) -> 北京(1) -> 武汉(4) -> 广州(3)" << std::endl;
    for (size_t i = 0; i < path.segments.size(); ++i) {
        const auto& s = path.segments[i];
        std::cout << "  段" << (i+1) << ": " << s.trip.trip_number
                  << " wait=" << s.wait_time << "min"
                  << " " << s.trip.departure_time << "->" << s.trip.arrival_time << std::endl;
    }
    std::cout << "  总耗时=" << path.total_time << "min"
              << " 总票价=¥" << path.total_price
              << " 换乘=" << path.transfer_count << "次" << std::endl;
    std::cout << "[PASS] Path 跨天场景测试通过\n" << std::endl;
}

int main() {
    std::cout << "========================================\n";
    std::cout << "  TransportType 结构体测试\n";
    std::cout << "========================================\n" << std::endl;

    test_enum_values();
    test_city();
    test_trip();
    test_segment();
    test_path_single_segment();
    test_path_multi_segment();
    test_path_cross_day();

    std::cout << "========================================\n";
    std::cout << "  全部测试通过!\n";
    std::cout << "========================================\n";
    return 0;
}
