#pragma once

#include <ostream>
#include <string>
#include <vector>

// 交通工具类型
// 支持火车和飞机两种交通工具
enum class TransportType { TRAIN, PLANE , MIXED};  // MIXED 表示两者皆可

// 决策策略
// 支持三种查询策略：最快、最便宜、最少换乘
enum class Strategy { FASTEST, CHEAPEST, LEAST_TRANSFERS };

// 城市 - 节点
struct City {
    int id_;
    std::string name_;
    // TODO: 未来可通过哈希表实现 缩写代号 (id_) -> 名称 (name_) 的快速查询
    // TODO(optimize): city.name_ 当前用 >> 读取，遇到含空格的多词城市名（如 "New York"）会截断，可用 getline 或引号包裹处理
};

// TransportType 的流输出运算符
inline std::ostream& operator<<(std::ostream& os, TransportType type) {
    switch (type) {
        case TransportType::TRAIN: os << "TRAIN"; break;
        case TransportType::PLANE: os << "PLANE"; break;
        case TransportType::MIXED: os << "MIXED"; break;
    }
    return os;
}

// 班次（一趟列车或一次航班）
struct Trip {
    int id_;                   // 班次编号
    TransportType type_;       // 火车 / 飞机
    int from_city_id_;         // 出发城市
    int to_city_id_;           // 到达城市
    int departure_time_;       // 出发时刻（绝对分钟，如 08:30 → 510）
    int arrival_time_;         // 到达时刻（绝对分钟）
    int price_;                // 票价（元）
    std::string trip_number_;  // 车次号/航班号，如 "G123" / "CA1234"
};

// 行程段（一条路径中的一段） - 边
struct Segment {
    Trip trip_;      // 乘坐的班次
    int wait_time_;  // 在本站等待时间（分钟），首段为 0
};

// 一条完整路径 - 路径
struct Path {
    std::vector<Segment> segments_;  // 依次乘坐的班次列表
    int total_time_;                 // 总耗时（含等待，分钟）
    int total_price_;                // 总票价（元）
    int transfer_count_;             // 换乘次数（段数 - 1）

    // 派生字段

    int departure_time() const { return segments_.empty() ? -1 : segments_.front().trip_.departure_time_; }  // 出发时刻
    int arrival_time() const { return segments_.empty() ? -1 : segments_.back().trip_.arrival_time_; }  // 到达时刻
};