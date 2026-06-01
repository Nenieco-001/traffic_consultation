# National Traffic Consultation Simulation System

> [中文版](README.md)

> **Disclaimer**: This project is a personal practice project. Code quality and engineering practices are for reference only. It may contain design flaws, uncovered edge cases, and incomplete features. **Not recommended for production or formal use.**

---

## Overview

A C++ console application that simulates nationwide transportation consultation. Users can query optimal routes between cities by train or plane, with three optimization strategies: **Fastest**, **Cheapest**, and **Least Transfers**.

### Features

- **Route Query**: Select origin/destination cities, transport mode, and strategy to get optimal routes with detailed itineraries
- **City Management**: Add/delete cities with automatic data persistence
- **Schedule Management**: Add/delete train schedules and flight schedules
- **Three Strategies**:
  - Fastest — Earliest Arrival Dijkstra
  - Cheapest — Lexicographic (cost, arrival) Dijkstra
  - Least Transfers — Constrained BFS

---

## 🏗 Architecture

Four-layer architecture with strict one-way dependency rules:

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

### Layer Responsibilities

| Layer | Responsibility | Depends On |
|-------|---------------|------------|
| **Domain** | Entities, algorithms, in-memory data, time tools | Standard library |
| **Application** | ConsultController orchestrates queries & management | Domain + Infrastructure |
| **Infrastructure** | FileIO serialization, FileConfig | Domain |
| **Presentation** | Console menu UI | Application |

---

## Quick Start

### Prerequisites

- GCC ≥ 11 (C++20 support)
- CMake ≥ 3.25
- Linux / WSL

### Build & Run

```bash
# Configure
cmake --preset wsl-gcc

# Build all
cmake --build --preset linux-debug

# Run main program
./bin/Traffic_Information
```

### Run Tests

```bash
for f in bin/*_test; do echo "=== $f ===" && "$f"; done
```

---

## Project Structure

```
├── inc/domain/model/         — Public entity declarations (City, Trip, Segment, Path, enums)
├── src/
│   ├── domain/               — Algorithms, TransportData, time tools
│   │   ├── algorithm/        — Dijkstra / BFS (3 strategies)
│   │   ├── data/             — TransportData in-memory repository
│   │   └── tools/            — Time conversion (HH:MM ↔ absolute minutes)
│   ├── application/          — ConsultController
│   ├── infrastructure/       — file_io, file_config
│   └── presentation/         — main.cpp, Menu
├── tests/                    — 7 unit test files
├── tests_data/               — Test data (separate directories)
├── data/                     — Production data (6 cities, 32 trips)
├── bin/                      — Build output
└── docs/                     — Documentation
    ├── GOAL.md               — Design document
    ├── TASK.md               — Task tracking
    └── CODE_REVIEW.md        — Conventions & review guide
```

---

## Test Coverage

| Target | File | Description |
|--------|------|-------------|
| Algorithms | `algo_layer_test.cpp` | T1-T9 comprehensive strategy tests |
| Data Layer | `data_layer_test.cpp` | CRUD + time window filtering |
| Controller | `controller_test.cpp` | C1-C10 integration tests |
| File IO | `file_io_test.cpp` | Read/write roundtrip |
| Time Tools | `time_process_test.cpp` | Minutes↔string conversion |
| Query Structs | `query_request_test.cpp` | QueryRequest/Result structs |
| Domain Types | `transport_type_test.cpp` | Enums & struct validation |

---

## Progress

| Iteration | Content | Status |
|-----------|---------|--------|
| 0 | Infrastructure (data structs, CMake) | ✅ Complete |
| 1 | Data layer & sample data | ✅ Complete |
| 2 | Core algorithms (3 strategies) | ✅ Complete |
| 3a | Architecture refactor (4-layer) | ✅ Complete |
| 3 | Console UI + Admin features | 🔄 In Progress |
| 4 | Web Frontend (optional) | ⏳ Pending |
| 5 | Testing & Report | ⏳ Pending |

---

## Known Issues

- Recursive menu loops may cause stack overflow (needs while-loop refactor)
- No cascading delete when removing cities (orphaned trips remain)
- Incomplete user input validation
- City names with spaces not supported

---

## License

This is a personal practice project, provided for reference and learning purposes only.
