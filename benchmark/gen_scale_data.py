#!/usr/bin/env python3
"""
生成 1000 城市算法扩展性测试数据。
输出文件格式与 file_io 兼容：
  city.dat            — CITY <id> <name>
  train_schedules.dat — TRIP <id> TRAIN <from> <to> <dep> <arr> <price> <no>
  flight_schedules.dat— TRIP <id> PLANE <from> <to> <dep> <arr> <price> <no>

图结构：每城市向后最多 3 个邻城，确保连通 (i → i+1 必连)，其余概率 60%。
"""

import os, random

random.seed(42)

OUT = os.path.join(os.path.dirname(__file__) or ".", "..", "tests_data", "algorithm_scale_1000")
os.makedirs(OUT, exist_ok=True)

NUM_CITIES = 1000
TRIPS_PER_CITY = 3

city_lines = []
trip_lines_train = []
trip_lines_plane = []
trip_id = 1

for i in range(1, NUM_CITIES + 1):
    city_lines.append(f"CITY {i} City{i}")

for src in range(1, NUM_CITIES + 1):
    neighbors = 0
    for dst in range(src + 1, NUM_CITIES + 1):
        if neighbors >= TRIPS_PER_CITY:
            break
        if dst == src + 1 or random.random() < 0.6:
            dep = 400 + random.randint(0, 800)
            arr = dep + 60 + random.randint(0, 300)
            price = 100 + random.randint(0, 900)
            ttype = "TRAIN" if random.random() < 0.5 else "PLANE"
            line = f"TRIP {trip_id} {ttype} {src} {dst} {dep} {arr} {price} S{trip_id}"
            if ttype == "TRAIN":
                trip_lines_train.append(line)
            else:
                trip_lines_plane.append(line)
            trip_id += 1
            neighbors += 1

with open(os.path.join(OUT, "city.dat"), "w") as f:
    f.write("\n".join(city_lines) + "\n")

with open(os.path.join(OUT, "train_schedules.dat"), "w") as f:
    f.write("\n".join(trip_lines_train) + "\n")

with open(os.path.join(OUT, "flight_schedules.dat"), "w") as f:
    f.write("\n".join(trip_lines_plane) + "\n")

total_trips = len(trip_lines_train) + len(trip_lines_plane)
print(f"已生成: {NUM_CITIES} 城市, {total_trips} 班次 ({len(trip_lines_train)} TRAIN / {len(trip_lines_plane)} PLANE)")
print(f"路径: {OUT}/")
