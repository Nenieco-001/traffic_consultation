# 全国交通咨询模拟系统

> [English Version](README_EN.md)

> **免责声明**：本项目为个人练习作品，代码质量和工程规范仅供参考。可能存在设计缺陷、未覆盖的边界情况以及未完善的功能。**不建议直接用于生产环境或正式项目。**

---

## 项目概述

基于 C++ 的控制台应用程序，模拟全国城市间的交通咨询。用户可查询两城市间的最优出行路线，支持火车和飞机两种交通方式，提供**最快到达**、**最省钱**、**最少中转**三种决策策略。

### 主要功能

- **交通咨询**：选择起止城市、交通工具和策略，获取最优路径及详细行程
- **城市管理**：增删城市，数据自动持久化
- **班次管理**：增删列车时刻表和飞机航班
- **三种策略**：
  - 最快到达 — Earliest Arrival Dijkstra
  - 最省钱 — Lexicographic (cost, arrival) Dijkstra
  - 最少中转 — 约束 BFS

---

## 系统架构

四层分层架构，遵循严格的单向依赖规则：

```
┌─────────────────────────────────────────────────────┐
│                    Presentation                     │
│              Console UI · main.cpp · Menu           │
└──────────────────────────┬──────────────────────────┘
                           │ calls
                           ▼
┌─────────────────────────────────────────────────────┐
│                    Application                      │
│              ConsultController · Use Cases          │
└──────────────────────────┬──────────────────────────┘
                           │ coordinates
                           ▼
┌─────────────────────────────────────────────────────┐
│                      Domain                         │
│ ┌─────────────────────────────────────────────────┐ │
│ │  model/   ·  City · Trip · Segment · Path       │ │
│ │  algorithm/  ·  Dijkstra (FASTEST/CHEAPEST)     │ │
│ │              ·  BFS (LEAST_TRANSFERS)           │ │
│ │  tools/   ·  Time conversion  ·  HH:MM ↔ min    │ │
│ └─────────────────────────────────────────────────┘ │
│ ┌─────────────────────────────────────────────────┐ │
│ │  data/  ·  TransportData (In-memory Repository) │ │
│ └─────────────────────────────────────────────────┘ │
└──────────────────────────┬──────────────────────────┘
                           │ depends on
                           ▼
┌─────────────────────────────────────────────────────┐
│                  Infrastructure                     │
│ ┌─────────────────────────────────────────────────┐ │
│ │  file_io/     ·  Serialization / Deserialization│ │
│ └─────────────────────────────────────────────────┘ │
│ ┌─────────────────────────────────────────────────┐ │
│ │  file_config/  ·  PROJECT_ROOT · Path config    │ │
│ └─────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────┘
```

### 各层职责

| 层 | 职责 | 依赖 |
|----|------|------|
| **Domain** | 业务实体、算法、内存数据仓库、时间工具 | 标准库 |
| **Application** | ConsultController 编排查询与管理操作 | Domain + Infrastructure |
| **Infrastructure** | FileIO 序列化、FileConfig 路径配置 | Domain |
| **Presentation** | 控制台菜单 UI | Application |

---

## 快速开始

### 环境要求

- GCC ≥ 11（支持 C++20）
- CMake ≥ 3.25
- Linux / WSL

### 构建 & 运行

```bash
# 配置
cmake --preset wsl-gcc

# 编译全部
cmake --build --preset linux-debug

# 运行主程序
./bin/Traffic_Information
```

### 运行测试

```bash
for f in bin/*_test; do echo "=== $f ===" && "$f"; done
```

---

## 项目结构

```
├── inc/domain/model/         — 公共实体声明（City, Trip, Segment, Path, 枚举）
├── src/
│   ├── domain/               — 算法、TransportData、时间工具
│   │   ├── algorithm/        — Dijkstra / BFS 三种策略
│   │   ├── data/             — TransportData 内存仓库
│   │   └── tools/            — 时间转换（HH:MM ↔ 绝对分钟）
│   ├── application/          — ConsultController
│   ├── infrastructure/       — file_io, file_config
│   └── presentation/         — main.cpp, Menu
├── tests/                    — 7 个单元测试文件
├── tests_data/               — 测试数据（分目录）
├── data/                     — 生产数据（6 城市，32 班次）
├── bin/                      — 编译输出
└── docs/                     — 文档
    ├── GOAL.md               — 详细设计文档
    ├── TASK.md               — 任务追踪
    └── CODE_REVIEW.md        — 开发规范与审查
```

---

## 测试覆盖

| 测试目标 | 文件 | 说明 |
|---------|------|------|
| 算法层 | `algo_layer_test.cpp` | T1-T9 三种策略全面测试 |
| 数据层 | `data_layer_test.cpp` | 增删改查 + 时间窗过滤 |
| 控制器 | `controller_test.cpp` | C1-C10 集成测试 |
| 文件 IO | `file_io_test.cpp` | 读写往返测试 |
| 时间工具 | `time_process_test.cpp` | 分钟↔字符串转换 |
| 查询请求 | `query_request_test.cpp` | QueryRequest/Result 结构 |
| 类型实体 | `transport_type_test.cpp` | 枚举与结构体验证 |

---

## 进度状态

| 迭代 | 内容 | 状态 |
|------|------|------|
| 0 | 基础设施（数据结构、CMake） | ✅ 完成 |
| 1 | 数据层与样例数据 | ✅ 完成 |
| 2 | 核心算法（三种策略） | ✅ 完成 |
| 3a | 架构重构（四层架构） | ✅ 完成 |
| 3 | 控制台 UI + 管理员功能 | 🔄 进行中 |
| 4 | Web 前端（可选扩展） | ⏳ 待定 |
| 5 | 测试与报告 | ⏳ 待定 |

---

## 已知问题

- 递归菜单循环可能导致栈溢出（待重构为 while 循环）
- 删除城市未级联删除关联班次
- 用户输入验证不完善
- 城市名不支持空格

---

## 许可

本项目为个人练习作品，仅供参考和学习交流使用。
