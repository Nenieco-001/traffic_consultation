// Dijkstra 算法实现
//
// C++11+ 特性：auto 类型推导、范围 for、lambda 表达式、模板、列表初始化
// C++14：std::greater<> 透明比较器
// C++17：std::optional、结构化绑定
// C++20：三路比较（operator<=> 隐式生成所有关系运算符）
//
// 泛型 dijkstraGeneric 承载所有单源时变图最短路径查询：
//   - DistType = int          → 边权 = wait + duration，最快到达
//   - DistType = CheapWeight  → 边权 = (price, wait+duration) 字典序，最省钱
//
// 同一份松弛逻辑、优先队列、prev 回溯，通过 Distance 模板参数统一。
//
// TODO: 全场景 Pareto 约束最短路径（NP-Hard）
//       当前 lexicographic 方案是 Pareto 的特例（费用为绝对优先维度）

#include "domain/algorithm/dijkstra.h"

#include <algorithm>  // std::reverse
#include <optional>   // std::optional
#include <queue>      // std::priority_queue
#include <utility>    // std::pair, std::move
#include <vector>     // std::vector

namespace {

    // FeasibleTrip：可衔接班次 + 预计算的等待时间和行程时长
    // C++11 聚合类型（无自定义构造函数，可直接用 {} 初始化）
    struct FeasibleTrip {
        Trip trip;          // 班次信息（含 trip.departure_time_ 发车时刻、trip.arrival_time_ 到站时刻）
        int wait;           // 在车站的等待分钟数（负值 +1440 表示等次日班次）
        int duration;       // 本段行程分钟数（arrival < departure 时 +1440 表示跨天运行）
        int total_time;     // wait + duration，预计算避免重复加法
    };

    // getFeasibleTrips：筛选从 city_id 出发、类型匹配的班次
    // curr_tod — 当前"一天中的分钟数"(0~1439)，用于计算等待时间
    // 返回 vector<FeasibleTrip> — C++11 返回值类型推导；范围 for 遍历全量班次（C++11）
    static std::vector<FeasibleTrip> getFeasibleTrips(const TransportData& data, int city_id, int curr_tod,
                                                      TransportType type) {
        std::vector<FeasibleTrip> result;
        // const auto&：C++11 范围 for，const 引用避免拷贝
        for (const auto& trip : data.getAllTrips()) {
            if (trip.from_city_id_ != city_id)
                continue;
            if (type != TransportType::MIXED && trip.type_ != type)
                continue;

            // 跨天处理：分钟数以 1440 为周期（一天），负值 +1440 映射到次日
            int wait = trip.departure_time_ - curr_tod;     // 需等待的分钟数
            if (wait < 0)
                wait += 1440;                               // 班次在次日，等一整天
            int duration = trip.arrival_time_ - trip.departure_time_;  // 行程时长
            if (duration < 0)
                duration += 1440;                           // 跨天运行（如 23:00→06:00）

            result.push_back({trip, wait, duration, wait + duration});  // C++11 列表初始化
        }
        return result;
    }

    // buildPath：沿 prev 数组从终点倒推到起点，重建完整 Path
    // prev[] — std::optional<Segment> (C++17)，nullopt 表示"无前驱"（即起点）
    //           seg.has_value() 判断是否有前驱；seg->trip_.from_city_id_ 回溯上一城市
    // dest_city_id — 终点城市编号
    // dest_arrival_time — 到达终点的绝对时刻（算法计算的 dist 值，从起点累计）
    // depart_time — 从起点出发的绝对时刻（固定参数，如 400 = 06:40）
    // 注意：Trip::arrival_time_（班次的到站时刻）≠ dest_arrival_time（算法计算的最早到达时刻）
    //       前者是单趟班次的固定属性，后者是算法状态（经过等待 + 多段行程累积）
    static Path buildPath(const std::vector<std::optional<Segment>>& prev,
                          int dest_city_id, int dest_arrival_time, int depart_time) {
        Path path;
        // auto 推导为 std::optional<Segment>（C++17），遍历方向：终点 → 起点
        for (auto seg = prev[dest_city_id]; seg.has_value();
             seg = prev[seg->trip_.from_city_id_]) {
            path.segments_.push_back(*seg);               // *seg 解引用 optional
        }
        std::reverse(path.segments_.begin(), path.segments_.end());  // 反转得到起点→终点

        path.total_time_ = dest_arrival_time - depart_time;         // 总耗时 = 到达 - 出发
        path.total_price_ = 0;
        path.transfer_count_ = static_cast<int>(path.segments_.size()) - 1;  // 段数 - 1
        for (const auto& seg : path.segments_)                      // 范围 for（C++11）
            path.total_price_ += seg.trip_.price_;

        // C++11 移动语义：编译器自动使用 NRVO 或无拷贝返回
        return path;
    }

    // CheapWeight：字典序边权类型（费用优先，同费取最早到达）
    // C++20 聚合类型，支持 {} 初始化
    // 重载 + / < / > 以满足 dijkstraGeneric 对 DistType 的接口要求
    struct CheapWeight {
        int cost_;        // 累计费用（主排序键）
        int arrival_;     // 到达该城市的绝对时刻（次排序键）

        // Lexicographic 字典序比较：cost 优先，arrival 其次
        bool operator>(const CheapWeight& o) const {
            return cost_ > o.cost_ || (cost_ == o.cost_ && arrival_ > o.arrival_);
        }
        bool operator<(const CheapWeight& o) const {
            return cost_ < o.cost_ || (cost_ == o.cost_ && arrival_ < o.arrival_);
        }
        CheapWeight operator+(const CheapWeight& d) const {
            return {cost_ + d.cost_, arrival_ + d.arrival_};
        }
    };

    // ============================================================
    // dijkstraGeneric — 泛型 Dijkstra 模板
    // ============================================================
    //
    // dist[v] = 最早到达 v 的绝对时刻（DistType = int）
    //         或 (最小费用, 最早到达) 字典序（DistType = CheapWeight）
    // 边权由 extract_weight 计算：
    //   DistType = int：边权 = wait + duration（最快）
    //   DistType = CheapWeight：边权 = (price, wait+duration)（最省钱字典序）
    //
    // 时变图：FIFO 性质保证贪心有效；跨天处理由 getFeasibleTrips 封装
    //
    // 模板参数：
    //   DistType — 距离类型（int / CheapWeight），需支持 + < > 和默认构造
    //   WeightFunc — 可调用对象 (const FeasibleTrip&) → DistType
    //     extract_weight && — C++11 万能引用（右值引用 + 引用折叠）
    //
    // 参数说明：
    //   depart_time  — 从起点出发的绝对时刻（如 400 = 06:40，固定参数不变）
    //   init_dist    — dist[起点] 初值（int 型 = depart_time；CheapWeight 型 = {0, depart_time}）
    //   extract_weight — 边权函数，由外部传入（lambda 或函数指针）

    template <typename DistType, typename WeightFunc>
    Path dijkstraGeneric(const TransportData& data, City from_city, City to_city, int depart_time,
                         TransportType transport_type, DistType init_dist, WeightFunc&& extract_weight) {
        // 同城直达：无需乘车，直接返回空路径
        if (from_city.id_ == to_city.id_)
            return Path{};                              // C++11 {} 聚合初始化，int 成员零填充

        // ---- 状态数组（数组长度 = 最大 city_id + 1） ----
        // dist[v] — 距离状态（int = 最早到达绝对时刻；CheapWeight = (最小费用, 最早到达)）
        //           用 std::optional 代替 sentinel，nullopt 表示"尚未到达"
        // arrival_time[v] — 到达时刻（int，与 DistType 独立），用于可衔接班次判断和 buildPath
        // prev[v] — 到达 v 所乘坐的班次（std::optional<Segment>，C++17）
        //           用于最终重建路径，nullopt 表示起点（无前驱）
        auto city_count = static_cast<size_t>(data.maxCityId()) + 1;  // 数组 size = max_id + 1
        std::vector<std::optional<DistType>> dist(city_count, std::nullopt);
        std::vector<int> arrival_time(city_count, -1);  // -1 表示尚未到达
        std::vector<std::optional<Segment>> prev(city_count, std::nullopt);  // 无前驱

        dist[from_city.id_] = init_dist;
        arrival_time[from_city.id_] = depart_time;

        // ---- 优先队列（小根堆） ----
        // std::pair<DistType,int> 默认按 first 比较，std::greater<>（C++14）实现小根堆
        // 同一城市可能多次入队（多条候选路径），但只有最短的会被处理
        // pair::operator> 调用 DistType::operator>（CheapWeight 为字典序，int 为数值序）
        std::priority_queue<std::pair<DistType, int>, std::vector<std::pair<DistType, int>>, std::greater<>> pq;
        pq.emplace(init_dist, from_city.id_);           // emplace（C++11）原地构造

        while (!pq.empty()) {
            // 结构化绑定（C++17）：将 pair 的两个成员分别赋予 curr_val 和 city_id
            auto [curr_val, city_id] = pq.top();        // curr_val = dist[city_id] 的候选值
            pq.pop();

            // 过时检查：如果当前候选值 > 已记录的最优值，跳过
            // 因为同一个城市可能被推入多次，只有最小值有效
            // dist 已设置（has_value）→ 候选更差则跳过；nullopt → 信任候选（不会发生：只推入已松弛的节点）
            if (dist[city_id].has_value() && curr_val > *dist[city_id])
                continue;

            // 首次 pop 到终点 → 最优解（Dijkstra 贪心性质保证）
            if (city_id == to_city.id_)
                return buildPath(prev, city_id, arrival_time[city_id], depart_time);

            // ---- 松弛操作 ----
            // 遍历当前城市出发的所有可衔接班次，尝试更新相邻城市的 dist
            // curr_tod：当前时刻取模 1440 = 一天中的分钟数 (0~1439)，用于计算等待时间
            int curr_tod = arrival_time[city_id] % 1440;
            for (const auto& ft : getFeasibleTrips(data, city_id, curr_tod, transport_type)) {
                // new_arrival — 从当前城市乘车到下一城市的绝对到达时刻（始终为 int）
                // new_dist    — 新的距离值（类型由 DistType 决定，通过 extract_weight 计算）
                int new_arrival = arrival_time[city_id] + ft.total_time;
                DistType new_dist = curr_val + extract_weight(ft);
                int to = ft.trip.to_city_id_;

                // 更新条件：
                //   nullopt → 首次到达，无条件更新
                //   否则 DistType 的 operator< 判断是否更优
                if (!dist[to].has_value() || new_dist < *dist[to]) {
                    dist[to] = new_dist;
                    arrival_time[to] = new_arrival;
                    prev[to] = Segment{ft.trip, ft.wait};
                    pq.emplace(new_dist, to);
                }
            }
        }

        // 队列耗尽仍未到达终点 → 无可达路径
        return Path{};
    }

}  // namespace

namespace algo {

    // findFastestPath — 委托泛型模板，DistType = int，以 wait + duration 为边权
    // init_dist = depart_time，边权 = total_time（行程总耗时）
    // new_dist = curr_dist + total_time = 到达下一站的绝对时刻
    // C++11 lambda 表达式：捕获 []、参数 const FeasibleTrip&、返回 total_time
    //   lambda 被模板参数 WeightFunc 推导，展开为函数对象，可内联避免函数调用开销

    Path findFastestPath(const TransportData& data, City from_city, City to_city, int depart_time,
                         TransportType transport_type) {
        return dijkstraGeneric<int>(data, from_city, to_city, depart_time, transport_type, depart_time,
                                    [](const FeasibleTrip& ft) { return ft.total_time; });
    }

    // ============================================================
    // findCheapestPath — Lexicographic Dijkstra（最省钱）
    // ============================================================
    //
    // 委托 dijkstraGeneric<CheapWeight>，边权 = (price, wait+duration)
    // init_dist = {0, depart_time}：起点费用 0，到达时刻 = 出发时刻
    //
    // CheapWeight::operator< 实现字典序：(cost, arrival)
    //   — 票价非负 → 贪心成立（高费用中间状态不可能追回）
    //   — 同费用保留最早到达 → 次优维度不丢失

    Path findCheapestPath(const TransportData& data, City from_city, City to_city, int depart_time,
                          TransportType transport_type) {
        // 显式指定 DistType = CheapWeight，传入 lambda 从 FeasibleTrip 提取 (price, total_time)
        return dijkstraGeneric<CheapWeight>(data, from_city, to_city, depart_time, transport_type,
                                            CheapWeight{0, depart_time},
                                            [](const FeasibleTrip& ft) {
                                                return CheapWeight{ft.trip.price_, ft.total_time};
                                            });
    }

    // ============================================================
    // findLeastTransferPath — 最少换乘（BFS）
    // ============================================================
    //
    // BFS 按层扩展，每层 = 一段行程，FIFO 队列保证首次到达目标城市时段数最少
    // 时间复杂度 O(V + E)（每个城市入队一次，每条边检查一次）
    //
    // 也可以用 dijkstraGeneric 以边权=1 实现（等价，BFS 更直观）：
    //   dijkstraGeneric(data, from, to, depart_time, type, 0,
    //                    [](const FeasibleTrip&) { return 1; })

    Path findLeastTransferPath(const TransportData& data, City from_city, City to_city, int depart_time,
                               TransportType transport_type) {
        if (from_city.id_ == to_city.id_)
            return Path{};

        // level[v] — 到达城市 v 的最少段数（BFS 的"层号"，-1 表示未到达）
        // arrival_time[v] — 到达 v 的绝对时刻（同 fastest，用于计算总耗时）
        // prev[v] — 前驱班次，std::optional<Segment>（C++17）
        auto city_count = static_cast<size_t>(data.maxCityId()) + 1;
        std::vector<int> level(city_count, -1);
        std::vector<int> arrival_time(city_count, -1);
        std::vector<std::optional<Segment>> prev(city_count, std::nullopt);

        level[from_city.id_] = 0;               // 起点算第 0 层
        arrival_time[from_city.id_] = depart_time;

        std::queue<int> q;                       // std::queue — FIFO（先进先出）容器适配器
        q.push(from_city.id_);

        while (!q.empty()) {
            int u = q.front();   // 当前城市编号（范围 for 遍历其出边班次）
            q.pop();

            if (u == to_city.id_)
                return buildPath(prev, u, arrival_time[u], depart_time);

            int curr_tod = arrival_time[u] % 1440;
            for (const auto& ft : getFeasibleTrips(data, u, curr_tod, transport_type)) {
                int v = ft.trip.to_city_id_;  // 下一城市
                if (level[v] != -1)
                    continue;                 // 已通过更少的段数到达过

                level[v] = level[u] + 1;                             // 层号 +1（段数 +1）
                arrival_time[v] = arrival_time[u] + ft.total_time;  // 到达时刻
                prev[v] = Segment{ft.trip, ft.wait};
                q.push(v);
            }
        }

        return Path{};          // 无解
    }

}  // namespace algo
