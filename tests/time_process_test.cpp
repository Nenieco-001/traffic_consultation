#include "tools/time_process.h"

#include <cassert>
#include <iostream>
#include <vector>

void test_timeStrToMinutes_valid() {
    std::cout << "--- timeStrToMinutes 合法输入测试 ---" << std::endl;

    // 标准格式 HH:MM
    assert(timeStrToMinutes("00:00") == 0);
    assert(timeStrToMinutes("08:00") == 480);
    assert(timeStrToMinutes("08:30") == 510);
    assert(timeStrToMinutes("12:00") == 720);
    assert(timeStrToMinutes("23:45") == 1425);
    assert(timeStrToMinutes("23:59") == 1439);

    // 短格式 H:MM
    assert(timeStrToMinutes("7:05") == 425);
    assert(timeStrToMinutes("0:00") == 0);
    assert(timeStrToMinutes("9:30") == 570);

    // 边界值
    assert(timeStrToMinutes("00:01") == 1);
    assert(timeStrToMinutes("23:59") == 1439);

    std::cout << "timeStrToMinutes(\"08:30\") = " << timeStrToMinutes("08:30") << std::endl;
    std::cout << "timeStrToMinutes(\"23:45\") = " << timeStrToMinutes("23:45") << std::endl;
    std::cout << "timeStrToMinutes(\"7:05\") = " << timeStrToMinutes("7:05") << std::endl;
    std::cout << "[PASS] timeStrToMinutes 合法输入测试通过\n" << std::endl;
}

void test_timeStrToMinutes_invalid() {
    std::cout << "--- timeStrToMinutes 非法输入测试 ---" << std::endl;

    // 小时超出范围
    assert(timeStrToMinutes("24:00") == -1);
    assert(timeStrToMinutes("99:00") == -1);

    // 分钟超出范围
    assert(timeStrToMinutes("12:60") == -1);
    assert(timeStrToMinutes("12:99") == -1);

    // 非数字字符
    assert(timeStrToMinutes("ab:cd") == -1);
    assert(timeStrToMinutes("12:ab") == -1);

    // 格式错误（缺少冒号）
    assert(timeStrToMinutes("1230") == -1);

    // 空字符串
    assert(timeStrToMinutes("") == -1);

    std::cout << "timeStrToMinutes(\"24:00\") = " << timeStrToMinutes("24:00") << " (expected -1)" << std::endl;
    std::cout << "timeStrToMinutes(\"12:60\") = " << timeStrToMinutes("12:60") << " (expected -1)" << std::endl;
    std::cout << "timeStrToMinutes(\"ab:cd\") = " << timeStrToMinutes("ab:cd") << " (expected -1)" << std::endl;
    std::cout << "[PASS] timeStrToMinutes 非法输入测试通过\n" << std::endl;
}

void test_minutesToTimeStr() {
    std::cout << "--- minutesToTimeStr 测试 ---" << std::endl;

    // 凌晨
    assert(minutesToTimeStr(0) == "00:00");

    // 正常时段
    assert(minutesToTimeStr(480) == "08:00");
    assert(minutesToTimeStr(510) == "08:30");
    assert(minutesToTimeStr(720) == "12:00");
    assert(minutesToTimeStr(1425) == "23:45");
    assert(minutesToTimeStr(1439) == "23:59");

    // 跨天时段（> 1440）
    assert(minutesToTimeStr(1500) == "25:00");

    // 边界
    assert(minutesToTimeStr(1440) == "24:00");
    assert(minutesToTimeStr(2880) == "48:00");

    std::cout << "minutesToTimeStr(0) = " << minutesToTimeStr(0) << std::endl;
    std::cout << "minutesToTimeStr(75) = " << minutesToTimeStr(75) << std::endl;
    std::cout << "minutesToTimeStr(1500) = " << minutesToTimeStr(1500) << std::endl;
    std::cout << "[PASS] minutesToTimeStr 测试通过\n" << std::endl;
}

void test_normalizeDayCrossing() {
    std::cout << "--- normalizeDayCrossing 跨天处理测试 ---" << std::endl;

    // 同一天：到达 > 出发，不调整
    assert(normalizeDayCrossing(480, 780) == 780);  // 08:00 -> 13:00

    // 跨天：到达 <= 出发，+1440
    assert(normalizeDayCrossing(1350, 390) == 1830);  // 22:30 -> 次日 06:30, 390+1440=1830
    assert(normalizeDayCrossing(1380, 120) == 1560);  // 23:00 -> 次日 02:00, 120+1440=1560
    assert(normalizeDayCrossing(0, 0) == 1440);       // 00:00 -> 次日 00:00

    // 午夜前后
    int dep = timeStrToMinutes("22:30");             // 1350
    int arr = timeStrToMinutes("01:15");             // 75
    assert(normalizeDayCrossing(dep, arr) == 1515);  // 75 + 1440

    std::cout << "normalizeDayCrossing(480, 780) = " << normalizeDayCrossing(480, 780) << " (同日)" << std::endl;
    std::cout << "normalizeDayCrossing(1350, 390) = " << normalizeDayCrossing(1350, 390) << " (跨天)" << std::endl;
    std::cout << "[PASS] normalizeDayCrossing 跨天处理测试通过\n" << std::endl;
}

void test_roundtrip() {
    std::cout << "--- timeStrToMinutes ↔ minutesToTimeStr 双向转换测试 ---" << std::endl;

    // 一组典型时刻，验证双向转换一致性
    std::vector<int> test_minutes = {0, 1, 60, 480, 510, 720, 1020, 1439};
    for (int mins : test_minutes) {
        std::string time_str = minutesToTimeStr(mins);
        int parsed = timeStrToMinutes(time_str);
        assert(parsed == mins);
        std::cout << mins << " -> \"" << time_str << "\" -> " << parsed << std::endl;
    }

    std::cout << "[PASS] 双向转换测试通过\n" << std::endl;
}

int main() {
    std::cout << "========================================\n";
    std::cout << "  时间工具函数测试\n";
    std::cout << "========================================\n" << std::endl;

    test_timeStrToMinutes_valid();
    test_timeStrToMinutes_invalid();
    test_minutesToTimeStr();
    test_normalizeDayCrossing();
    test_roundtrip();

    std::cout << "========================================\n";
    std::cout << "  全部测试通过!\n";
    std::cout << "========================================\n";
    return 0;
}
