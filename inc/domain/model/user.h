#pragma once

#include <string>
#include <string_view>

// 用户角色枚举
enum class UserRole { ADMIN, NORMAL };

// 将 UserRole 枚举转换为字符串
inline std::string userRoleToString(UserRole role) {
    return role == UserRole::ADMIN ? "ADMIN" : "NORMAL";
}

// 将字符串转换为 UserRole 枚举
inline UserRole stringToUserRole(std::string_view sv) {
    if (sv == "ADMIN")
        return UserRole::ADMIN;
    return UserRole::NORMAL;
}

// 用户结构体
struct User {
    int id_;
    std::string username_;
    std::string password_hash_;
    UserRole role_;
};
