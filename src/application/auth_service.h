/**
 * @file auth_service.h
 * @brief 应用层 — 认证服务，依赖 UserRepository（领域层）+ file_io（基础设施层）
 */

#pragma once

#include <optional>
#include <string>

#include "domain/data/user_repository.h"

class AuthService {
  public:
    void loadData(const std::string& dir = "");  // 从文件加载用户
    void saveData(const std::string& dir = "");

    std::optional<User> login(const std::string& username, const std::string& password);
    std::optional<User> registerUser(const std::string& username, const std::string& password, UserRole role);
    bool hasAnyUser() const;

  private:
    UserRepository user_repo_;

    // 带盐哈希：std::hash 不是密码学安全，但课程项目演示哈希思想
    // 用固定盐值防止简单彩虹表匹配
    static std::string hashPassword(const std::string& password);
};
