#include "domain/data/user_repository.h"

#include <algorithm>

// 添加用户
User UserRepository::addUser(const std::string& username, const std::string& password_hash, UserRole role) {
    User user;               // 创建一个新的 User 对象
    user.id_ = getNextId();  // 设置用户 ID
    user.username_ = username;
    user.password_hash_ = password_hash;
    user.role_ = role;
    users_.push_back(user);
    return user;
}

// 根据用户名查找用户
std::optional<User> UserRepository::findByUsername(const std::string& username) const {
    // 使用 std::ranges::find_if （ C++20 ）查找匹配用户名的用户，最终返回一个迭代器
    auto it = std::ranges::find_if(users_, [&](const User& u) { return u.username_ == username; });
    // 可采用 std::find_if (C++11 ) 替代 std::ranges::find_if ，但需要手动指定范围
    // auto it = std::find_if(users_.begin(), users_.end(), [&](const User& u) { return u.username_ == username; });
    if (it != users_.end())
        return *it;
    return std::nullopt;
}

// 根据用户 ID 查找用户
std::optional<User> UserRepository::findById(int id) const {
    auto it = std::ranges::find_if(users_, [id](const User& u) { return u.id_ == id; });
    if (it != users_.end())
        return *it;
    return std::nullopt;
}

// 获取所有用户
const std::vector<User>& UserRepository::getAllUsers() const {
    return users_;
}

// 获取用户数量
size_t UserRepository::count() const {
    return users_.size();
}

// 获取下一个用户 ID
int UserRepository::getNextId() const {
    int max_id = 0;
    for (const auto& u : users_)
        if (u.id_ > max_id)
            max_id = u.id_;
    return max_id + 1;
}
