/**
 * @file file_io.h
 * @brief 基础设施层 — 文件持久化
 *
 * 提供交通数据和用户数据的序列化/反序列化功能。
 * 数据文件格式为纯文本，每行以类型前缀开头：
 *   - CITY:   CITY <id> <name>
 *   - TRIP:   TRIP <id> <TRAIN|PLANE> <from> <to> <departure> <arrival> <price> <number>
 *   - USER:   USER <id> <username> <password_hash> <ADMIN|NORMAL>
 *
 * 路径规则：
 *   - 参数 path/dirPath 为空 → 使用 FileConfig 单例获取默认路径（PROJECT_ROOT/data/）
 *   - 参数非空 → 视为目录，在该目录下读写 city.dat / train_schedules.dat / flight_schedules.dat / user.dat
 *
 * 分层说明：本模块位于基础设施层，依赖领域层的数据类型（City、Trip、User、TransportData），
 * 但领域层不依赖本模块 — 符合整洁架构的依赖倒置原则。
 */

#pragma once

#include <string>
#include <vector>

#include "domain/data/transport_data.h"
#include "domain/model/user.h"

namespace file_io {

    // 交通数据持久化 ============================================
    void saveToFile(const std::vector<City>& cities, const std::vector<Trip>& trips, const std::string& path = "");
    [[nodiscard]] TransportData loadFromFile(const std::string& dirPath = "");

    // 用户数据持久化 ============================================
    void saveUsers(const std::vector<User>& users, const std::string& path = "");
    [[nodiscard]] std::vector<User> loadUsers(const std::string& dirPath = "");

}  // namespace file_io
