// 多轮独立运行取均值 + 标准差 — Release (-O2)
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "domain/algorithm/dijkstra.h"
#include "domain/data/transport_data.h"
#include "domain/tools/time_process.h"
#include "infrastructure/file_io.h"

using Clock = std::chrono::high_resolution_clock;

struct ScaleResult {
    int V, E;
    double fast_ms, cheap_ms, least_ms;
};

static ScaleResult testOnce(const char* data_dir, int from_id, int to_id, int depart) {
    auto data = file_io::loadFromFile(data_dir);
    auto cities = data.getAllCities();
    auto trips = data.getAllTrips();
    int V = static_cast<int>(cities.size());
    int E = static_cast<int>(trips.size());

    auto t0 = Clock::now();
    algo::findFastestPath(data, {from_id,""}, {to_id,""}, depart, TransportType::MIXED);
    auto t1 = Clock::now();
    double fast = std::chrono::duration<double, std::milli>(t1 - t0).count();

    t0 = Clock::now();
    algo::findCheapestPath(data, {from_id,""}, {to_id,""}, depart, TransportType::MIXED);
    t1 = Clock::now();
    double cheap = std::chrono::duration<double, std::milli>(t1 - t0).count();

    t0 = Clock::now();
    algo::findLeastTransferPath(data, {from_id,""}, {to_id,""}, depart, TransportType::MIXED);
    t1 = Clock::now();
    double least = std::chrono::duration<double, std::milli>(t1 - t0).count();

    return {V, E, fast, cheap, least};
}

int main() {
    const int ROUNDS = 10;
    const int DEPART = 400;

    struct ScaleCfg { const char* dir; int fid; int tid; const char* label; };
    ScaleCfg cfgs[] = {
        {"tests_data/algorithm_scale_10",   1, 5,   "10 城 (City1→City5)"},
        {"tests_data/algorithm_scale_100",  1, 50,  "100 城(City1→City50)"},
        {"tests_data/algorithm_scale_1000", 1, 500, "1000城(City1→City500)"},
    };

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "============================================================\n";
    std::cout << "  算法扩展性测试 — Release(-O2)  " << ROUNDS << " 轮独立运行\n";
    std::cout << "============================================================\n\n";

    // 最终汇总表格
    struct FinalRow { int V, E; double mf, sf, mc, sc, ml, sl; };
    std::vector<FinalRow> final_rows;

    for (auto& cfg : cfgs) {
        std::vector<double> fast_v, cheap_v, least_v;
        int run_E = 0, run_V = 0;

        std::cout << "--- " << cfg.label << " ---\n";
        for (int r = 0; r < ROUNDS; r++) {
            auto run = testOnce(cfg.dir, cfg.fid, cfg.tid, DEPART);
            if (r == 0) { run_E = run.E; run_V = run.V; }
            fast_v.push_back(run.fast_ms);
            cheap_v.push_back(run.cheap_ms);
            least_v.push_back(run.least_ms);
            std::cout << "  #" << (r+1) << "  F=" << std::setw(8) << run.fast_ms
                      << "  C=" << std::setw(8) << run.cheap_ms
                      << "  L=" << std::setw(8) << run.least_ms << " ms\n";
        }

        auto avg = [](auto& v) { return accumulate(v.begin(), v.end(), 0.0) / v.size(); };
        auto sdv = [](auto& v, double m) {
            double s = 0; for (auto x : v) s += (x-m)*(x-m);
            return sqrt(s / v.size());
        };

        double mf = avg(fast_v), sf = sdv(fast_v, mf);
        double mc = avg(cheap_v), sc = sdv(cheap_v, mc);
        double ml = avg(least_v), sl = sdv(least_v, ml);

        std::cout << "  ---------------------------------------------\n";
        std::cout << "  Mean ± σ   "
                  << mf << " ± " << sf << "   "
                  << mc << " ± " << sc << "   "
                  << ml << " ± " << sl << "  ms\n";
        std::cout << "  (V=" << run_V << ", E=" << run_E << ")\n\n";

        final_rows.push_back({run_V, run_E, mf, sf, mc, sc, ml, sl});
    }

    // 汇总表
    std::cout << "============================================================\n";
    std::cout << "  汇总 (Mean ± σ, ms)\n";
    std::cout << "============================================================\n";
    std::cout << "V      E     最快到达         最省钱          最少换乘\n";
    std::cout << "--------------------------------------------------------------\n";
    for (auto& r : final_rows) {
        std::cout << std::left << std::setw(6) << r.V << std::setw(7) << r.E
                  << std::setw(14) << (std::to_string(r.mf) + "±" + std::to_string(r.sf)) << "  "
                  << std::setw(14) << (std::to_string(r.mc) + "±" + std::to_string(r.sc)) << "  "
                  << (std::to_string(r.ml) + "±" + std::to_string(r.sl)) << "\n";
    }

    return 0;
}
