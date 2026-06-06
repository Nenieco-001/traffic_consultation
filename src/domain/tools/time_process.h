#pragma once
#include <string>

// 分钟 -> "HH:MM"（支持跨天，如 1500 → "25:00"）
std::string minutesToTimeStr(int minutes);
// "HH:MM" -> 分钟，返回 -1 表示格式错误
int timeStrToMinutes(const std::string& time_str);