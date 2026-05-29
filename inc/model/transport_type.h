#pragma once

#include <string>
#include <vector>

// 交通工具类型
// 支持火车和飞机两种交通工具
enum class TransportType { TRAIN, PLANE };

// 决策策略
// 支持三种查询策略：最快、最便宜、最少换乘
enum class Strategy { FASTEST, CHEAPEST, LEAST_TRANSFERS };

// 城市 - 节点
struct City {
    int id;
    std::string name;
};

// 班次（一趟列车或一次航班）
struct Trip {
    int id;                   // 班次编号
    TransportType type;       // 火车 / 飞机
    int from_city_id;         // 出发城市
    int to_city_id;           // 到达城市
    int departure_time;       // 出发时刻（绝对分钟，如 08:30 → 510）
    int arrival_time;         // 到达时刻（绝对分钟）
    int price;                // 票价（元）
    std::string trip_number;  // 车次号/航班号，如 "G123" / "CA1234"
};

// 行程段（一条路径中的一段） - 边
struct Segment {
    Trip trip;      // 乘坐的班次
    int wait_time;  // 在本站等待时间（分钟），首段为 0
};

// 一条完整路径 - 路径
struct Path {
    std::vector<Segment> segments;  // 依次乘坐的班次列表
    int total_time;                 // 总耗时（含等待，分钟）
    int total_price;                // 总票价（元）
    int transfer_count;             // 换乘次数（段数 - 1）

    // 派生字段
    
    int departure_time() const { return segments.empty() ? -1 : segments.front().trip.departure_time; }  // 出发时刻
    int arrival_time() const { return segments.empty() ? -1 : segments.back().trip.arrival_time; }       // 到达时刻
};