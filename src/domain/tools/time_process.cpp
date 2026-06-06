#include "domain/tools/time_process.h"

#include <cassert>
#include <cctype>
#include <format>

// 分钟 -> "HH:MM"（支持跨天，如 1500 → "25:00"）
std::string minutesToTimeStr(int minutes) {
    assert(minutes >= 0);  // 负数不合法，调用者应确保输入有效
    int hours = minutes / 60;
    int mins = minutes % 60;
    return std::format("{:02d}:{:02d}", hours, mins);

    // TODO: 未来需要优化跨天逻辑，例如 25:00 -> 01:00 (+1)
}

// "HH:MM" / "H:MM" -> 分钟，输入格式非法时返回 -1
int timeStrToMinutes(const std::string& time_str) {
    // 基本格式检查：长度为 5 （HH:MM）或 4 （H:MM）
    if (time_str.size() != 5 && time_str.size() != 4)
        return -1;
    if (time_str[2] != ':' && time_str[1] != ':')
        return -1;
    for (size_t i = 0; i < time_str.size(); ++i) {
        if (time_str.size() == 5 && i == 2)
            continue;  // HH:MM 格式，跳过冒号
        if (time_str.size() == 4 && i == 1)
            continue;  // H:MM 格式，跳过冒号
        if (!std::isdigit(static_cast<unsigned char>(time_str[i])))
            return -1;  // 非数字字符非法
    }
    int hours = std::stoi(time_str.substr(0, time_str.size() == 5 ? 2 : 1));
    int mins = std::stoi(time_str.substr(time_str.size() == 5 ? 3 : 2, 2));
    if (hours < 0 || hours > 23 || mins < 0 || mins > 59)
        return -1;
    return hours * 60 + mins;
}