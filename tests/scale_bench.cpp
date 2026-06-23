// 快速多规模性能测试 — 10/100/1000 城单次计时
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "domain/algorithm/dijkstra.h"
#include "domain/data/transport_data.h"
#include "domain/tools/time_process.h"
#include "infrastructure/file_io.h"

using Clock = std::chrono::high_resolution_clock;

struct ScaleResult {
    int cities;
    int edges;
    double fastest_ms;
    double cheapest_ms;
    double least_ms;
    double state_kb;
};

static ScaleResult testScale(const char* data_dir, City from, City to, int depart) {
    auto data = file_io::loadFromFile(data_dir);
    auto cities = data.getAllCities();
    auto trips = data.getAllTrips();
    int V = static_cast<int>(cities.size());
    int E = static_cast<int>(trips.size());

    auto t0 = Clock::now();
    auto p1 = algo::findFastestPath(data, from, to, depart, TransportType::MIXED);
    auto t1 = Clock::now();
    double t_fast = std::chrono::duration<double, std::milli>(t1 - t0).count();

    t0 = Clock::now();
    auto p2 = algo::findCheapestPath(data, from, to, depart, TransportType::MIXED);
    t1 = Clock::now();
    double t_cheap = std::chrono::duration<double, std::milli>(t1 - t0).count();

    t0 = Clock::now();
    auto p3 = algo::findLeastTransferPath(data, from, to, depart, TransportType::MIXED);
    t1 = Clock::now();
    double t_least = std::chrono::duration<double, std::milli>(t1 - t0).count();

    int max_id = data.maxCityId();
    double state_kb = static_cast<double>(max_id + 1) *
        (sizeof(int) * 2 + sizeof(int) * 2 + sizeof(int)) / 1024.0;

    return {V, E, t_fast, t_cheap, t_least, state_kb};
}

int main() {
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "城市数  边数   最快到达    最省钱     最少换乘    状态数组\n";
    std::cout << "----------------------------------------------------------\n";

    // 10 城: City1 → City5
    auto r10 = testScale("tests_data/algorithm_scale_10", {1,""}, {5,""}, 400);
    std::cout << r10.cities << "      " << r10.edges << "     "
              << r10.fastest_ms << " ms    " << r10.cheapest_ms << " ms    "
              << r10.least_ms << " ms    " << r10.state_kb << " KB\n";

    // 100 城: City1 → City50
    auto r100 = testScale("tests_data/algorithm_scale_100", {1,""}, {50,""}, 400);
    std::cout << r100.cities << "     " << r100.edges << "    "
              << r100.fastest_ms << " ms    " << r100.cheapest_ms << " ms    "
              << r100.least_ms << " ms    " << r100.state_kb << " KB\n";

    // 1000 城: City1 → City500
    auto r1000 = testScale("tests_data/algorithm_scale_1000", {1,""}, {500,""}, 400);
    std::cout << r1000.cities << "    " << r1000.edges << "   "
              << r1000.fastest_ms << " ms   " << r1000.cheapest_ms << " ms   "
              << r1000.least_ms << " ms   " << r1000.state_kb << " KB\n";

    return 0;
}
