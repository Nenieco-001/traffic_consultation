#include "domain/model/query_request.h"

#include <cassert>
#include <iostream>

void test_query_request_default() {
    std::cout << "--- QueryRequest 默认值测试 ---" << std::endl;

    QueryRequest req{};
    
    // assert(): 断言，用于验证条件是否为真，如果条件为假则程序会终止并输出错误信息

    std::cout << "from_city_id = " << req.from_city_id_ << std::endl;
    std::cout << "to_city_id = " << req.to_city_id_ << std::endl;
    std::cout << "[PASS] QueryRequest 默认值测试通过\n" << std::endl;
}

void test_query_request_full() {
    std::cout << "--- QueryRequest 完整初始化测试 ---" << std::endl;

    // 查询北京(1)→上海(2)，火车，最快，08:00 后出发，2小时时间窗
    QueryRequest req{1, 2, Strategy::FASTEST, TransportType::TRAIN, 480};

    assert(req.from_city_id_ == 1);
    assert(req.to_city_id_ == 2);
    assert(req.strategy_ == Strategy::FASTEST);
    assert(req.transport_ == TransportType::TRAIN);
    assert(req.depart_after_ == 480);         // 08:00

    std::cout << "from=" << req.from_city_id_ << " to=" << req.to_city_id_ << " strategy=FASTEST" << " transport=TRAIN"
              << " depart_after=" << req.depart_after_ << std::endl;
    std::cout << "[PASS] QueryRequest 完整初始化测试通过\n" << std::endl;
}

void test_query_request_other_strategies() {
    std::cout << "--- QueryRequest 不同策略测试 ---" << std::endl;

    // 最省钱策略
    QueryRequest cheap_req{1, 2, Strategy::CHEAPEST, TransportType::PLANE, 0};
    assert(cheap_req.strategy_ == Strategy::CHEAPEST);
    assert(cheap_req.transport_ == TransportType::PLANE);
    std::cout << "strategy=CHEAPEST transport=PLANE" << std::endl;

    // 最少换乘策略
    QueryRequest transfer_req{1, 2, Strategy::LEAST_TRANSFERS, TransportType::TRAIN, 600};
    assert(transfer_req.strategy_ == Strategy::LEAST_TRANSFERS);
    assert(transfer_req.depart_after_ == 600);  // 10:00
    std::cout << "strategy=LEAST_TRANSFERS depart_after=600" << std::endl;

    std::cout << "[PASS] QueryRequest 不同策略测试通过\n" << std::endl;
}

void test_query_result_has_solution() {
    std::cout << "--- QueryResult has_solution() 测试 ---" << std::endl;

    // 无解：空路径列表
    QueryResult empty_result;
    assert(empty_result.paths.empty());
    assert(empty_result.has_solution() == false);
    assert(empty_result.error_msg.empty());
    std::cout << "empty paths: has_solution = " << empty_result.has_solution() << std::endl;

    // 有解：路径列表非空
    Trip trip{1, TransportType::TRAIN, 1, 2, 480, 780, 550, "G1"};
    Segment seg{trip, 0};
    Path path{{seg}, 300, 550, 0};

    QueryResult result_with_path;
    result_with_path.paths.push_back(path);
    assert(result_with_path.has_solution() == true);
    assert(result_with_path.paths.size() == 1);
    std::cout << "with 1 path: has_solution = " << result_with_path.has_solution() << std::endl;

    // 错误信息
    QueryResult error_result;
    error_result.error_msg = "未找到可行路径";
    assert(error_result.has_solution() == false);
    assert(error_result.error_msg == "未找到可行路径");
    std::cout << "error_msg = " << error_result.error_msg << std::endl;

    std::cout << "[PASS] QueryResult has_solution() 测试通过\n" << std::endl;
}

void test_query_result_multiple_paths() {
    std::cout << "--- QueryResult 多路径结果测试 ---" << std::endl;

    Trip trip1{1, TransportType::TRAIN, 1, 2, 480, 780, 550, "G1"};
    Trip trip2{2, TransportType::TRAIN, 1, 2, 840, 1110, 450, "G3"};

    Path fast_path{{Segment{trip1, 0}}, 300, 550, 0};
    Path cheap_path{{Segment{trip2, 0}}, 270, 450, 0};

    QueryResult result;
    result.paths.push_back(fast_path);
    result.paths.push_back(cheap_path);

    assert(result.paths.size() == 2);
    assert(result.has_solution() == true);

    // 验证路径 1（最快）
    assert(result.paths[0].total_time_ == 300);
    assert(result.paths[0].total_price_ == 550);

    // 验证路径 2（最省）
    assert(result.paths[1].total_time_ == 270);
    assert(result.paths[1].total_price_ == 450);

    std::cout << "paths count = " << result.paths.size() << std::endl;
    std::cout << "  path[0]: time=" << result.paths[0].total_time_ << " price=" << result.paths[0].total_price_
              << std::endl;
    std::cout << "  path[1]: time=" << result.paths[1].total_time_ << " price=" << result.paths[1].total_price_
              << std::endl;
    std::cout << "[PASS] QueryResult 多路径结果测试通过\n" << std::endl;
}

int main() {
    std::cout << "========================================\n";
    std::cout << "  QueryRequest / QueryResult 结构体测试\n";
    std::cout << "========================================\n" << std::endl;

    test_query_request_default();
    test_query_request_full();
    test_query_request_other_strategies();
    test_query_result_has_solution();
    test_query_result_multiple_paths();

    std::cout << "========================================\n";
    std::cout << "  全部测试通过!\n";
    std::cout << "========================================\n";
    return 0;
}
