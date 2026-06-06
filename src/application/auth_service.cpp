#include "application/auth_service.h"

#include <functional>  // std::hash

#include "infrastructure/file_io.h"

static constexpr auto kSalt = "TrafficConsult2026";

void AuthService::loadData(const std::string& dir) {
    auto users = file_io::loadUsers(dir);
    for (auto& u : users)
        user_repo_.addUser(u.username_, u.password_hash_, u.role_);
}

void AuthService::saveData(const std::string& dir) {
    file_io::saveUsers(user_repo_.getAllUsers(), dir);
}

std::optional<User> AuthService::login(const std::string& username, const std::string& password) {
    auto user = user_repo_.findByUsername(username);
    if (!user)
        return std::nullopt;
    if (user->password_hash_ != hashPassword(password))  // 比对哈希值而非明文
        return std::nullopt;
    return user;
}

std::optional<User> AuthService::registerUser(const std::string& username, const std::string& password, UserRole role) {
    if (user_repo_.findByUsername(username).has_value())
        return std::nullopt;  // 用户名冲突

    auto hashed = hashPassword(password);
    return user_repo_.addUser(username, hashed, role);
}

bool AuthService::hasAnyUser() const {
    return user_repo_.count() > 0;
}

std::string AuthService::hashPassword(const std::string& password) {
    // 盐值拼接到密码前再哈希，使相同密码产生不同哈希（与无盐相比）
    // std::hash 输出 size_t，课程演示足够了
    auto h = std::hash<std::string>{}(kSalt + password);
    return std::to_string(h);
}
