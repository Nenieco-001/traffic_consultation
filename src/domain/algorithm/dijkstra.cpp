// Dijkstra 算法实现
//
// 设计：
//   泛型函数 dijkstraGeneric 承载"最快到达"的时变图 Dijkstra
//   findCheapestPath 独立实现 lexicographic（字典序）Dijkstra
//
// 两种策略在算法上的关系：
//   最快到达 → 时变图 Dijkstra（dist = 最早到达时间）
//     边权 = wait + duration（即行程耗时），dist 和 arrival_time 自然地合一
//     时间复杂度 O(E log V)
//
//   最省钱 → Lexicographic (cost, arrival) Dijkstra
//     松弛仍遍历真实班次（含跨天等待），但状态按 (费用, 到达时刻) 字典序比较
//     费用优先，同费用时取最早到达（从而获得同费用下最快的时间）
//     由于票价非负，更高费用的中间状态不可能追回成为全局更优 → Dijkstra 贪心成立
//     时间复杂度 O(E log V)
//
//   两者共享一个工具函数 getFeasibleTrips，集中处理等待时间和跨天逻辑
//
// TODO: 全场景 Pareto 约束最短路径（NP-Hard）
//       如需同时最小化时间和费用，需保留每个城市的 Pareto 前沿（多状态）
//       当前 lexicographic 方案是 Pareto 的特例（费用为绝对优先维度）
// TODO: 最少换乘路径（边权为 1 的 Dijkstra 或 BFS)
// TODO: 尝试学习 A* 加速（启发式可用城市间直线距离）
//

#include "domain/algorithm/dijkstra.h"

#include <algorithm>  // std::reverse, std::sort
#include <limits>     // std::numeric_limits<int>::max()
#include <optional>   // std::optional
#include <queue>      // std::priority_queue
#include <utility>    // std::pair, std::move
#include <vector>     // std::vector

namespace {

    // ============================================================
    // 工具：获取从某城市出发的所有可衔接班次
    // ============================================================
    // TODO(perf): 每次松弛遍历全量班次 O(N)，数据量大时可建 from_city_id 索引降为 O(1)
    // FeasibleTrip：一条可衔接的班次 + 预计算好的等待时间和行程时长
    struct FeasibleTrip {
        Trip trip;     // 原始的班次信息（票价、起止城市、时刻等）
        int wait;      // 在车站等待该班次出发的分钟数（含跨天，e.g. 在 B 站等 5 小时 = 300）
        int duration;  // 该班次的行程时间（含跨天，e.g. 运行 1.5 小时 = 90）
    };

    // 参数：city_id = 出发城市, curr_tod = 当前时刻(一天中的分钟数 0-1439), type = 交通工具类型
    // 返回：从 city_id 出发的所有符合 type 的班次 + 等待时间 + 行程时长
    static std::vector<FeasibleTrip> getFeasibleTrips(const TransportData& data, int city_id, int curr_tod,
                                                      TransportType type) {
        std::vector<FeasibleTrip> result;
        // 遍历全部班次，筛选出 from_city_id 匹配的
        for (const auto& trip : data.getAllTrips()) {
            // 跳过不从该城市出发的班次
            if (trip.from_city_id_ != city_id)
                continue;
            // 按交通工具类型过滤（MIXED 表示接受所有类型）
            if (type != TransportType::MIXED && trip.type_ != type)
                continue;

            // 等待时间 = 班次出发时刻 - 当前时刻
            // e.g. 当前 10:00(=600), 班次 14:00(=840) → wait=240 分钟
            // e.g. 当前 22:00(=1320), 班次 06:00(=360) → wait = 360-1320 = -960, 加 1440 后 = 480(等 8 小时到次日)
            int wait = trip.departure_time_ - curr_tod;
            if (wait < 0)
                wait += 1440;  // 次日班次：等待到第二天同一时刻

            // 行程时长 = 到达时刻 - 出发时刻
            // 如果到达 < 出发，说明跨天（e.g. 23:00 出发 06:00 到达 → arr-dep = -1020, 加 1440 = 420 分钟）
            int duration = trip.arrival_time_ - trip.departure_time_;
            if (duration < 0)
                duration += 1440;

            result.push_back({trip, wait, duration});
        }
        return result;
    }

    // ============================================================
    // 泛型 Dijkstra 核心（最快到达专用）
    // ============================================================
    //
    // 参数:
    //   data           = 数据源（城市 + 班次）
    //   from_city      = 起点城市
    //   to_city        = 终点城市
    //   depart_time    = 起点出发时刻（绝对分钟，如 08:30 → 510）
    //   transport_type = 交通工具类型（TRAIN / PLANE / MIXED）
    //   init_dist      = 起点的 dist 初始值（最快到达 = depart_time）
    //   extract_weight = 泛型函数：(trip, wait, duration) → 边权（最快到达 = wait + duration）
    //
    // 注意：
    //   这是时变图 Dijkstra，dist[v] = 最早到达 v 的时刻（不是"累计耗时"）
    //   边权 = wait + duration，new_dist = curr_dist + wait + duration = 到达下一站的绝对时刻
    //   所以 dist[v] 直接就是"最早到达时刻"，与 Dijkstra 的"累计距离"语义一致

    template <typename WeightFunc>
    // extract_weight 是一个可调用对象，接受 (trip, wait, duration) 参数，返回边权值
    Path dijkstraGeneric(const TransportData& data, City from_city, City to_city, int depart_time,
                         TransportType transport_type, int init_dist, WeightFunc&& extract_weight) {
        // ---- 边界：同城直达 ----
        // 起点 == 终点，不需要乘车，耗时 0 费用 0 换乘 0
        if (from_city.id_ == to_city.id_) {
            Path path;
            path.total_time_ = 0;
            path.total_price_ = 0;
            path.transfer_count_ = 0;
            return path;
        }

        // ---- 状态数组 ----
        // dist[v]        = 最早到达 v 的时刻（绝对分钟，可 > 1440 表示跨天）
        //                  这是 PQ 的排序依据，也是 Dijkstra 的"最优值"
        // arrival_time[v] = 到达 v 的绝对时刻（和 dist[v] 值相同，分开存是为了语义清晰）
        // prev[v]        = 到达 v 所乘坐的班次（含等待时间），用于最终重建路径
        auto city_count = data.getAllCities().size();
        std::vector<int> dist(city_count, std::numeric_limits<int>::max());  // 初始化为无穷大
        std::vector<int> arrival_time(city_count, -1);                       // -1 表示尚未到达
        std::vector<std::optional<Segment>> prev(city_count, std::nullopt);  // nullopt 表示无前驱（起点）

        dist[from_city.id_] = init_dist;            // 起点最早到达时刻 = 出发时刻
        arrival_time[from_city.id_] = depart_time;  // 起点的到达时刻 = 出发时刻（已在起点）

        // ---- 小根堆 ----
        // 元素 = (dist, city_id)，dist 小的优先出队
        // 使用 std::greater<> 使 top() 返回 dist 最小的元素
        // 同一个城市可能入队多次（多条路径），出队时通过 dist[...] 判断是否过时
        std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<>> pq;
        pq.emplace(init_dist, from_city.id_);  // 初始状态：起点出发

        // ---- Dijkstra 主循环 ----
        while (!pq.empty()) {
            // 取出当前最优候选状态：
            //   curr_val = dist 值（最早到达时刻）
            //   city_id  = 城市编号
            auto [curr_val, city_id] = pq.top();
            pq.pop();

            // 过时检查：PQ 中可能存了该城市的多个状态，
            // 如果 curr_val > dist[city_id]，说明该城市已经有更早到达的方案被处理过了
            if (curr_val > dist[city_id])
                continue;

            // ---- 终点判定 ----
            // 首次 pop 到目标城市，意味着当前 dist[city_id] 就是最早的到达时刻
            // Dijkstra 贪心性质保证了这一点
            if (city_id == to_city.id_) {
                // 重建路径：从终点沿 prev 倒推回起点
                // prev[v] = 到达 v 的 Segment(班次,等待时间)
                // seg->trip_.from_city_id_ = 上一个城市
                Path path;
                for (auto seg = prev[city_id]; seg.has_value(); seg = prev[seg->trip_.from_city_id_]) {
                    path.segments_.push_back(*seg);
                }
                // 倒推得到的是逆序（终点→起点），需要反转
                std::reverse(path.segments_.begin(), path.segments_.end());

                // 汇总统计信息
                path.total_time_ = arrival_time[city_id] - depart_time;  // 总耗时 = 到达时刻 - 出发时刻
                path.total_price_ = 0;
                path.transfer_count_ = static_cast<int>(path.segments_.size()) - 1;  // 段数-1 = 换乘次数
                for (const auto& seg : path.segments_) {
                    path.total_price_ += seg.trip_.price_;  // 累计总票价
                }
                return path;
            }

            // ---- 松弛 ----
            // 从当前城市出发，遍历所有可赶上的后续班次
            // 尝试通过每个班次到达下一城市，看能不能找到更早的到达
            int curr_arrival = arrival_time[city_id];  // 到达当前城市的绝对时刻
            int curr_tod = curr_arrival % 1440;        // 一天中的分钟数（0~1439）
            // ft = FeasibleTrip：一条可衔接的班次（含预计算的 wait 和 duration）
            for (const auto& ft : getFeasibleTrips(data, city_id, curr_tod, transport_type)) {
                // 到达下一城市 = 当前时刻 + 等待 + 行程时长
                int new_arrival = curr_arrival + ft.wait + ft.duration;
                // 新的 dist = 当前 dist + 边权。对于最快到达，边权 = wait + duration
                int new_dist = curr_val + extract_weight(ft.trip, ft.wait, ft.duration);

                // 如果新到达时间比之前记录的更早，就更新
                if (new_dist < dist[ft.trip.to_city_id_]) {
                    dist[ft.trip.to_city_id_] = new_dist;                   // 更新最早到达时刻
                    arrival_time[ft.trip.to_city_id_] = new_arrival;        // 更新绝对到达时刻
                    prev[ft.trip.to_city_id_] = Segment{ft.trip, ft.wait};  // 记录前驱
                    pq.emplace(new_dist, ft.trip.to_city_id_);              // 加入候选队列
                }
            }
        }

        // PQ 耗尽仍未到达目标城市 → 无解
        return Path();
    }

}  // namespace

namespace algo {

    // ============================================================
    // findFastestPath — 最快到达
    // ============================================================
    // 委托给 dijkstraGeneric，传参：
    //   init_dist = depart_time（起点最早到达时刻 = 出发时刻）
    //   边权 = wait + duration（从当前时刻到抵达下一站的总耗时）
    // 因此 new_dist = curr_dist + wait + duration = 到达下一站的绝对时刻 = dist / arrival_time

    Path findFastestPath(const TransportData& data, City from_city, City to_city, int depart_time,
                         TransportType transport_type) {
        return dijkstraGeneric(data, from_city, to_city, depart_time, transport_type, depart_time,
                               [](const Trip&, int wait, int duration) { return wait + duration; });
    }

    // ============================================================
    // findCheapestPath — 最省钱（Lexicographic Dijkstra）
    // ============================================================
    //
    // 算法思路
    //   状态 = (累计费用, 到达时刻)
    //   PQ 按 (cost, arrival) 字典序出队：cost 越小越优先，cost 相同时 arrival 越小越优先
    //   松弛仍遍历真实班次（含跨天等待），因为：
    //     ① 需要计算等待时间和到达时刻（总耗时、时间可行性）
    //     ② 票价作为边权加入累计费用
    //
    // 正确性
    //   票价非负 → 更高费用的中间状态不可能追回成为全局更优 → Dijkstra 贪心成立
    //   相同费用下保留最早到达 → 次优维度不丢失

    // CheapState：PQ 中的状态节点
    // 三个字段意图明确：累计费用、到达时刻、城市编号

    struct CheapState {
        int cost_;     // 累计费用（主排序键，越小越优先）
        int arrival_;  // 到达该城市的时刻（次排序键，同费用时越大越晚到，越晚越不优先）
        int city_id_;  // 城市编号

        // operator> 被 std::greater<CheapState> 调用，实现小根堆
        // 先比 cost，再比 arrival（字典序）
        friend bool operator>(const CheapState& a, const CheapState& b) {
            if (a.cost_ != b.cost_)
                return a.cost_ > b.cost_;
            return a.arrival_ > b.arrival_;
        }
    };

    Path findCheapestPath(const TransportData& data, City from_city, City to_city, int depart_time,
                          TransportType transport_type) {
        // ---- 边界：同城直达 ----
        if (from_city.id_ == to_city.id_) {
            Path path;
            path.total_time_ = 0;
            path.total_price_ = 0;
            path.transfer_count_ = 0;
            return path;
        }

        // ---- 状态数组 ----
        // dist_cost[v] = 到达 v 的最小累计费用（主优化目标）
        //                 这是 PQ 的第一排序键，也是算法的"最优值"
        // arr_time[v]  = 达到最小费用时的最早到达时刻（次优目标）
        //                 同费用多条路径时，保留到得最早的那条
        // prev[v]      = 到达 v 乘坐的班次（含等待时间），重建路径用
        auto city_count = data.getAllCities().size();
        std::vector<int> dist_cost(city_count, std::numeric_limits<int>::max());  // 初始费用 = 无穷大
        std::vector<int> arr_time(city_count, -1);                                // -1 表示尚未到达
        std::vector<std::optional<Segment>> prev(city_count, std::nullopt);       // 无前驱

        dist_cost[from_city.id_] = 0;           // 起点费用 = 0
        arr_time[from_city.id_] = depart_time;  // 起点到达时刻 = 出发时刻

        // ---- 小根堆 ----
        // (cost, arrival, city_id)，按字典序出队（cost 优先，cost 相同比 arrival）
        // 这里用 push{} 而不是 emplace()，因为 CheapState 是聚合类型，C++20 支持括号初始化
        std::priority_queue<CheapState, std::vector<CheapState>, std::greater<>> pq;
        pq.push({0, depart_time, from_city.id_});

        // ---- Dijkstra 主循环 ----
        while (!pq.empty()) {
            // s = 当前候选状态：累计费用、到达时刻、城市编号
            auto s = pq.top();
            pq.pop();

            // 过时判断：如果该城市已经有一个"费用更低，或同费用但到得更早"的状态被处理过，跳过
            // 即：(s.cost, s.arrival) 字典序大于 (dist_cost[city], arr_time[city]) 时过时
            if (s.cost_ > dist_cost[s.city_id_] ||
                (s.cost_ == dist_cost[s.city_id_] && s.arrival_ > arr_time[s.city_id_])) {
                continue;
            }

            // ---- 终点判定 ----
            // 首次 pop 到目标城市，说明当前就是最小费用（同费用下最早到达）
            if (s.city_id_ == to_city.id_) {
                // 重建路径：同 findFastestPath 一样，从终点沿 prev 倒推
                Path path;
                for (auto seg = prev[s.city_id_]; seg.has_value(); seg = prev[seg->trip_.from_city_id_]) {
                    path.segments_.push_back(*seg);
                }
                std::reverse(path.segments_.begin(), path.segments_.end());

                path.total_time_ = arr_time[s.city_id_] - depart_time;  // 总耗时含所有等待
                path.total_price_ = 0;
                path.transfer_count_ = static_cast<int>(path.segments_.size()) - 1;
                for (const auto& seg : path.segments_) {
                    path.total_price_ += seg.trip_.price_;  // 累计总票价
                }
                return path;
            }

            // ---- 松弛 ----
            // 尝试从当前城市乘班次去下一个城市
            int curr_arrival = arr_time[s.city_id_];  // 到达当前城市的时刻
            int curr_tod = curr_arrival % 1440;       // 一天中的分钟数
            // ft = FeasibleTrip：一条可衔接的班次（含 wait 和 duration）
            for (const auto& ft : getFeasibleTrips(data, s.city_id_, curr_tod, transport_type)) {
                // new_cost      = 当前累计费用 + 该班次票价（等待不增加费用）
                // new_arrival   = 当前到达时刻 + 等待 + 行程时长（等待会增加总耗时）
                int new_cost = s.cost_ + ft.trip.price_;
                int new_arrival = curr_arrival + ft.wait + ft.duration;

                int to = ft.trip.to_city_id_;  // 目标城市

                // 更新条件：
                // 费用更低 → 无条件更新（主目标改善）
                // 同费用但到达更早 → 更新（次目标改善）
                // 不会出现"费用更高但到达更早 → 更新"的情况，因为非负边权保证
                //     更高费用的中间状态永远不可能追回成为全局更优
                if (new_cost < dist_cost[to] || (new_cost == dist_cost[to] && new_arrival < arr_time[to])) {
                    dist_cost[to] = new_cost;              // 更新最小费用
                    arr_time[to] = new_arrival;            // 更新到达时刻
                    prev[to] = Segment{ft.trip, ft.wait};  // 记录前驱
                    pq.push({new_cost, new_arrival, to});  // 加入候选队列
                }
            }
        }

        // 无解
        return Path();
    }

    // ============================================================
    // findLeastTransferPath — 最少换乘（BFS）
    // ============================================================
    //
    // BFS 按层扩展，每层 = 一段行程
    // 队列先进先出保证首次到达目标城市时段数最少
    // 数据结构和路径重建与最快/最省钱保持一致的风格
    //
    // 备选实现（注释）：用 dijkstraGeneric 实现，边权=1
    //   以下注释代码展示了如何用 dijkstraGeneric 以边权=1 实现同样功能
    //   两者结果完全等价，BFS 更直观且适合作为教学对比
    //
    // Path findLeastTransferPath(const TransportData& data, City from_city, City to_city, int depart_time,
    //                            TransportType transport_type) {
    //     return dijkstraGeneric(data, from_city, to_city, depart_time, transport_type, 0,
    //                            [](const Trip&, int, int) { return 1; });
    // }
    //   Dijkstra 版说明：
    //     dist[v] = 到达 v 的最少段数（每段边权=1）
    //     transfer_count = dist[to] - 1 = 最少换乘次数
    //     arrival_time[v] 仍正常跟踪，total_time_ 正确

    Path findLeastTransferPath(const TransportData& data, City from_city, City to_city, int depart_time,
                               TransportType transport_type) {
        if (from_city.id_ == to_city.id_) {
            Path path;
            path.total_time_ = 0;
            path.total_price_ = 0;
            path.transfer_count_ = 0;
            return path;
        }

        // ---- 状态数组 ----
        // level[v]         = 到达 v 的最少段数
        // arrival_time[v]  = 到达 v 的时刻（用于总耗时）
        // prev[v]          = 到达 v 乘坐的班次 + 等待时间
        auto city_count = data.getAllCities().size();
        std::vector<int> level(city_count, -1);
        std::vector<int> arrival_time(city_count, -1);
        std::vector<std::optional<Segment>> prev(city_count, std::nullopt);

        level[from_city.id_] = 0;
        arrival_time[from_city.id_] = depart_time;

        // BFS 队列，按入队顺序逐层处理
        std::queue<int> q;
        q.push(from_city.id_);  // 初始状态：起点

        while (!q.empty()) {
            int u = q.front();
            q.pop();

            if (u == to_city.id_) {  // 首次到达目标城市，当前 level[u] 就是最少段数
                Path path;
                for (auto seg = prev[u]; seg.has_value(); seg = prev[seg->trip_.from_city_id_]) {
                    path.segments_.push_back(*seg);
                }
                std::reverse(path.segments_.begin(), path.segments_.end());
                path.total_time_ = arrival_time[u] - depart_time;
                path.total_price_ = 0;
                path.transfer_count_ = static_cast<int>(path.segments_.size()) - 1;
                for (const auto& seg : path.segments_) {
                    path.total_price_ += seg.trip_.price_;
                }
                return path;
            }

            // ---- 扩展下一层 ----
            int curr_arrival = arrival_time[u];
            int curr_tod = curr_arrival % 1440;

            for (const auto& ft : getFeasibleTrips(data, u, curr_tod, transport_type)) {
                int v = ft.trip.to_city_id_;  // 下一城市
                if (level[v] != -1)
                    continue;  // 该城市已通过更少段数到达过

                level[v] = level[u] + 1;
                arrival_time[v] = curr_arrival + ft.wait + ft.duration;
                prev[v] = Segment{ft.trip, ft.wait};
                q.push(v);
            }
        }

        return Path();
    }

}  // namespace algo
