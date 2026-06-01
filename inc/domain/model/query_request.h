#pragma once

#include "domain_types.h"

// TODO: 后续建议将 QueryRequest / QueryResult 拆分到独立头文件，或合入 domain_types.h
// 查询请求
struct QueryRequest {
    int from_city_id_;             // 出发城市id
    int to_city_id_;               // 到达城市id
    Strategy strategy_;            // 查询策略
    TransportType transport_;      // 基础功能只用一种交通工具
    int depart_after_;             // 最早出发时间（绝对分钟），默认 = "现在"
};

// 查询结果
struct QueryResult {
    std::vector<Path> paths;                              // 最优路径列表（按策略排序，最多 N 条）
    std::string error_msg;                                // 无解时的提示信息
    bool has_solution() const { return !paths.empty(); }  // 是否有解
};