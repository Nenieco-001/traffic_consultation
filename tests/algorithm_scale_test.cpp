/**
 * @file algorithm_scale_test.cpp
 * @brief 算法扩展性测试 — 1000 城市合成数据集
 *
 * 测试目标：
 *   1. 验证 1000 城规模下三种算法能正确求解
 *   2. 记录单次查询平均耗时
 *   3. 验证时间/空间复杂度趋势
 *
 * 数据集：tests_data/algorithm_scale_1000/ 下 1000 城 2994 班次
 *
 * 编译依赖：
 *   transport_data, file_io, file_config, dijkstra, time_process
 *
 * 用法：
 *   cmake --build --preset linux-debug
 *   ./bin/algorithm_scale_test
 */

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "domain/algorithm/dijkstra.h"
#include "domain/data/transport_data.h"
#include "domain/tools/time_process.h"
#include "infrastructure/file_io.h"

// ===== 测试配置 =====
static const char* DATA_DIR = "tests_data/algorithm_scale_1000";

// 计时工具
using Clock = std::chrono::high_resolution_clock;

struct TimedResult {
    Path path;
    long long elapsed_ns;
};

// 执行一次查询并计时
static TimedResult timedQuery(const TransportData& data, City from, City to, int depart_time,
                               TransportType type, Strategy strategy) {
    auto t0 = Clock::now();
    Path p;
    switch (strategy) {
        case Strategy::FASTEST:
            p = algo::findFastestPath(data, from, to, depart_time, type);
            break;
        case Strategy::CHEAPEST:
            p = algo::findCheapestPath(data, from, to, depart_time, type);
            break;
        case Strategy::LEAST_TRANSFERS:
            p = algo::findLeastTransferPath(data, from, to, depart_time, type);
            break;
    }
    auto t1 = Clock::now();
    long long ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    return {p, ns};
}

// 重复 N 次取平均
static long long averageTime(const TransportData& data, City from, City to, int depart_time,
                              TransportType type, Strategy strategy, int iterations) {
    long long total = 0;
    for (int i = 0; i < iterations; ++i) {
        auto t0 = Clock::now();
        switch (strategy) {
            case Strategy::FASTEST:
                algo::findFastestPath(data, from, to, depart_time, type);
                break;
            case Strategy::CHEAPEST:
                algo::findCheapestPath(data, from, to, depart_time, type);
                break;
            case Strategy::LEAST_TRANSFERS:
                algo::findLeastTransferPath(data, from, to, depart_time, type);
                break;
        }
        auto t1 = Clock::now();
        total += std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
    }
    return total / iterations;
}

static const char* passFail(bool ok) { return ok ? "[PASS]" : "[FAIL]"; }

// ===== 主测试逻辑 =====
int main() {
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "===============================================" << std::endl;
    std::cout << "  算法扩展性测试 — 1000 城市合成数据集" << std::endl;
    std::cout << "===============================================" << std::endl;

    // ---- 加载数据 ----
    std::cout << "\n--- 加载数据 ---" << std::endl;
    TransportData data;
    try {
        data = file_io::loadFromFile(DATA_DIR);
    } catch (const std::exception& e) {
        std::cout << "[FAIL] 数据加载失败: " << e.what() << std::endl;
        return 1;
    }

    auto cities = data.getAllCities();
    auto trips = data.getAllTrips();
    std::cout << "  城市: " << cities.size() << "  (期望 1000)" << std::endl;
    std::cout << "  班次: " << trips.size() << "  (期望 ~3000)" << std::endl;

    bool load_ok = (cities.size() == 1000 && trips.size() > 2000);
    std::cout << "  " << passFail(load_ok) << " 数据完整性检查" << std::endl;
    if (!load_ok) {
        std::cout << "  数据异常，终止测试。" << std::endl;
        return 1;
    }

    City city1{1, "City1"}, city5{5, "City5"}, city10{10, "City10"};
    City city50{50, "City50"}, city100{100, "City100"}, city500{500, "City500"};
    City city999{999, "City999"};

    bool all_pass = true;

    // ============================================================
    // T1 — 近距直达：City1 → City5
    // ============================================================
    {
        std::cout << "\n--- T1: 近距直达  City1 → City5 (FASTEST/CHEAPEST) ---" << std::endl;
        bool ok = true;

        auto r1 = timedQuery(data, city1, city5, 400, TransportType::MIXED, Strategy::FASTEST);
        bool seg_ok = !r1.path.segments_.empty();
        ok = ok && seg_ok;
        std::cout << "  " << passFail(seg_ok) << " FASTEST: " << r1.elapsed_ns / 1000000.0 << " ms"
                  << ", " << r1.path.segments_.size() << " 段, " << r1.path.total_time_ << " min, "
                  << r1.path.total_price_ << " 元" << std::endl;

        auto r2 = timedQuery(data, city1, city5, 400, TransportType::MIXED, Strategy::CHEAPEST);
        seg_ok = !r2.path.segments_.empty();
        ok = ok && seg_ok;
        std::cout << "  " << passFail(seg_ok) << " CHEAPEST: " << r2.elapsed_ns / 1000000.0 << " ms"
                  << ", " << r2.path.segments_.size() << " 段, " << r2.path.total_time_ << " min, "
                  << r2.path.total_price_ << " 元" << std::endl;

        std::cout << "  " << passFail(ok) << " T1 近距直达" << std::endl;
        all_pass = all_pass && ok;
    }

    // ============================================================
    // T2 — 中距多跳：City1 → City100 (MIXED / TRAIN / PLANE)
    // ============================================================
    {
        std::cout << "\n--- T2: 中距多跳  City1 → City100 ---" << std::endl;
        bool ok = true;

        auto r1 = timedQuery(data, city1, city100, 420, TransportType::MIXED, Strategy::FASTEST);
        bool seg_ok = !r1.path.segments_.empty();
        ok = ok && seg_ok;
        std::cout << "  " << passFail(seg_ok) << " MIXED/FASTEST: " << r1.elapsed_ns / 1000000.0 << " ms"
                  << ", " << r1.path.segments_.size() << " 段, " << r1.path.total_time_ << " min" << std::endl;

        auto r2 = timedQuery(data, city1, city100, 420, TransportType::MIXED, Strategy::CHEAPEST);
        seg_ok = !r2.path.segments_.empty();
        ok = ok && seg_ok;
        std::cout << "  " << passFail(seg_ok) << " MIXED/CHEAPEST: " << r2.elapsed_ns / 1000000.0 << " ms"
                  << ", " << r2.path.segments_.size() << " 段, " << r2.path.total_price_ << " 元" << std::endl;

        auto r3 = timedQuery(data, city1, city100, 420, TransportType::MIXED, Strategy::LEAST_TRANSFERS);
        seg_ok = !r3.path.segments_.empty();
        ok = ok && seg_ok;
        std::cout << "  " << passFail(seg_ok) << " MIXED/LEAST_TRANSFERS: " << r3.elapsed_ns / 1000000.0 << " ms"
                  << ", " << r3.path.segments_.size() << " 段 (最少换乘)" << std::endl;

        std::cout << "  " << passFail(ok) << " T2 中距多跳" << std::endl;
        all_pass = all_pass && ok;
    }

    // ============================================================
    // T3 — 远距查询：City1 → City999（最远端点）
    // ============================================================
    {
        std::cout << "\n--- T3: 远距查询  City1 → City999 ---" << std::endl;
        bool ok = true;

        auto r1 = timedQuery(data, city1, city999, 400, TransportType::MIXED, Strategy::FASTEST);
        bool seg_ok = !r1.path.segments_.empty();
        ok = ok && seg_ok;
        std::cout << "  " << passFail(seg_ok) << " FASTEST: " << r1.elapsed_ns / 1000000.0 << " ms"
                  << ", " << r1.path.segments_.size() << " 段, " << r1.path.total_time_ << " min" << std::endl;

        auto r2 = timedQuery(data, city1, city999, 400, TransportType::MIXED, Strategy::CHEAPEST);
        seg_ok = !r2.path.segments_.empty();
        ok = ok && seg_ok;
        std::cout << "  " << passFail(seg_ok) << " CHEAPEST: " << r2.elapsed_ns / 1000000.0 << " ms"
                  << ", " << r2.path.segments_.size() << " 段, " << r2.path.total_price_ << " 元" << std::endl;

        auto r3 = timedQuery(data, city1, city999, 400, TransportType::MIXED, Strategy::LEAST_TRANSFERS);
        seg_ok = !r3.path.segments_.empty();
        ok = ok && seg_ok;
        std::cout << "  " << passFail(seg_ok) << " LEAST_TRANSFERS: " << r3.elapsed_ns / 1000000.0 << " ms"
                  << ", " << r3.path.segments_.size() << " 段" << std::endl;

        std::cout << "  " << passFail(ok) << " T3 远距查询" << std::endl;
        all_pass = all_pass && ok;
    }

    // ============================================================
    // T4 — 无解查询：孤立城市 ID 9999（不存在）
    // ============================================================
    {
        std::cout << "\n--- T4: 无解查询  City1 → City9999 (不存在) ---" << std::endl;
        City nonexist{9999, "NONEXIST"};
        auto r1 = timedQuery(data, city1, nonexist, 400, TransportType::MIXED, Strategy::FASTEST);
        bool empty_ok = r1.path.segments_.empty();
        std::cout << "  " << passFail(empty_ok) << " FASTEST 返回空路径: " << r1.elapsed_ns / 1000000.0 << " ms" << std::endl;
        all_pass = all_pass && empty_ok;
    }

    // ============================================================
    // T5 — 同类目的地（同一城市出发和到达）
    // ============================================================
    {
        std::cout << "\n--- T5: 同城查询  City50 → City50 (应返回空路径) ---" << std::endl;
        auto r1 = timedQuery(data, city50, city50, 400, TransportType::MIXED, Strategy::FASTEST);
        bool empty_ok = r1.path.segments_.empty();
        std::cout << "  " << passFail(empty_ok) << " FASTEST: " << r1.elapsed_ns / 1000000.0 << " ms" << std::endl;
        all_pass = all_pass && empty_ok;
    }

    // ============================================================
    // T6 — 策略比较：三种策略在同一路线上的结果对比
    // ============================================================
    {
        std::cout << "\n--- T6: 多策略对比  City10 → City500 ---" << std::endl;

        auto fastest = timedQuery(data, city10, city500, 480, TransportType::MIXED, Strategy::FASTEST);
        auto cheapest = timedQuery(data, city10, city500, 480, TransportType::MIXED, Strategy::CHEAPEST);
        auto least = timedQuery(data, city10, city500, 480, TransportType::MIXED, Strategy::LEAST_TRANSFERS);

        bool f_ok = !fastest.path.segments_.empty();
        bool c_ok = !cheapest.path.segments_.empty();
        bool l_ok = !least.path.segments_.empty();
        bool ok = f_ok && c_ok && l_ok;

        std::cout << "  " << passFail(ok) << " 三策略均有解" << std::endl;

        if (ok) {
            // FASTEST 的总耗时应 ≤ CHEAPEST 的总耗时
            bool time_check = fastest.path.total_time_ <= cheapest.path.total_time_;
            // CHEAPEST 的总价应 ≤ FASTEST 的总价
            bool cost_check = cheapest.path.total_price_ <= fastest.path.total_price_;
            // LEAST_TRANSFERS 的换乘数应 ≤ 其他两者
            bool transfer_check = (least.path.transfer_count_ <= fastest.path.transfer_count_ &&
                                   least.path.transfer_count_ <= cheapest.path.transfer_count_);

            std::cout << "  FASTEST: " << fastest.path.total_time_ << " min, "
                      << fastest.path.total_price_ << " 元, "
                      << fastest.path.transfer_count_ << " 换乘  (" << fastest.elapsed_ns / 1000000.0 << " ms)" << std::endl;
            std::cout << "  CHEAPEST: " << cheapest.path.total_time_ << " min, "
                      << cheapest.path.total_price_ << " 元, "
                      << cheapest.path.transfer_count_ << " 换乘  (" << cheapest.elapsed_ns / 1000000.0 << " ms)" << std::endl;
            std::cout << "  LEAST_TRANSFERS: " << least.path.total_time_ << " min, "
                      << least.path.total_price_ << " 元, "
                      << least.path.transfer_count_ << " 换乘  (" << least.elapsed_ns / 1000000.0 << " ms)" << std::endl;

            std::cout << "  " << passFail(time_check) << " FASTEST 耗时 ≤ CHEAPEST" << std::endl;
            std::cout << "  " << passFail(cost_check) << " CHEAPEST 费用 ≤ FASTEST" << std::endl;
            std::cout << "  " << passFail(transfer_check) << " LEAST_TRANSFERS 换乘最少" << std::endl;

            ok = ok && time_check && cost_check && transfer_check;
        }
        all_pass = all_pass && ok;
    }

    // ============================================================
    // T7 — 交通工具过滤：MIXED 可用 → 过滤后无混入（合成数据不保证每种类型都有路径）
    // ============================================================
    {
        std::cout << "\n--- T7: 交通工具过滤  City1 → City100 ---" << std::endl;

        auto mixed = timedQuery(data, city1, city100, 400, TransportType::MIXED, Strategy::FASTEST);
        bool mixed_ok = !mixed.path.segments_.empty();
        std::cout << "  " << passFail(mixed_ok) << " MIXED/FASTEST: " << mixed.elapsed_ns / 1000000.0 << " ms, "
                  << mixed.path.total_time_ << " min" << std::endl;

        // 验证 MIXED 结果中 TransportType 合法（TRAIN 或 PLANE）
        bool type_ok = true;
        for (const auto& seg : mixed.path.segments_) {
            if (seg.trip_.type_ != TransportType::TRAIN && seg.trip_.type_ != TransportType::PLANE) {
                type_ok = false;
                break;
            }
        }
        std::cout << "  " << passFail(type_ok) << " 班次类型合法 (TRAIN/PLANE)" << std::endl;

        // TRAIN only — 可选，不要求必有解（依赖合成数据随机性）
        auto train_only = timedQuery(data, city1, city100, 400, TransportType::TRAIN, Strategy::FASTEST);
        bool train_has_path = !train_only.path.segments_.empty();
        std::cout << "  [INFO] TRAIN only: "
                  << (train_has_path ? (std::to_string(train_only.elapsed_ns / 1000000.0) + " ms, " +
                                        std::to_string(train_only.path.total_time_) + " min")
                                     : std::string("无路径（数据限制，非算法问题）"))
                  << std::endl;

        all_pass = all_pass && mixed_ok && type_ok;
    }

    // ============================================================
    // T8 — 循环性能基准（100 次取平均）
    // ============================================================
    {
        std::cout << "\n--- T8: 循环性能基准 (100 次取平均) ---" << std::endl;

        const int N = 100;
        long long t_fast, t_cheap, t_least;

        t_fast = averageTime(data, city1, city500, 400, TransportType::MIXED, Strategy::FASTEST, N);
        t_cheap = averageTime(data, city1, city500, 400, TransportType::MIXED, Strategy::CHEAPEST, N);
        t_least = averageTime(data, city1, city500, 400, TransportType::MIXED, Strategy::LEAST_TRANSFERS, N);

        std::cout << "  FASTEST:          " << t_fast / 1000000.0 << " ms  (1000城 " << data.getAllTrips().size() << " 边)"
                  << std::endl;
        std::cout << "  CHEAPEST:         " << t_cheap / 1000000.0 << " ms  (1000城 " << data.getAllTrips().size() << " 边)"
                  << std::endl;
        std::cout << "  LEAST_TRANSFERS:  " << t_least / 1000000.0 << " ms  (1000城 " << data.getAllTrips().size() << " 边)"
                  << std::endl;

        // 状态数组空间估算
        int max_id = data.maxCityId();
        size_t state_bytes = static_cast<size_t>(max_id + 1) *
            (sizeof(int) * 2 + sizeof(std::optional<int>) + sizeof(std::optional<Segment>));
        std::cout << "  Dijkstra 状态数组大小: " << max_id + 1 << " 元素, " << (state_bytes / 1024.0) << " KB" << std::endl;

        std::cout << "  [PASS] T8 循环性能基准" << std::endl;
    }

    // ============================================================
    // 汇总
    // ============================================================
    std::cout << "\n===============================================" << std::endl;
    if (all_pass) {
        std::cout << "  全部算法扩展性测试通过!" << std::endl;
    } else {
        std::cout << "  部分测试失败，请检查以上 [FAIL] 项" << std::endl;
    }
    std::cout << "===============================================" << std::endl;

    return all_pass ? 0 : 1;
}
