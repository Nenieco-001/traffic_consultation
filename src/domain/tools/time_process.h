#pragma once
#include <string>

// 分钟 -> "HH:MM"（支持跨天，如 1500 → "25:00"）
std::string minutesToTimeStr(int minutes);
// "HH:MM" -> 分钟，返回 -1 表示格式错误
int timeStrToMinutes(const std::string& time_str);
// 处理跨天：若到达时刻 < 出发时刻，到达时刻加 1440 表示次日
// TODO(cleanup): normalizeDayCrossing 当前未被任何模块调用，考虑移除或整合到 getFeasibleTrips
int normalizeDayCrossing(int departure_time, int arrival_time);