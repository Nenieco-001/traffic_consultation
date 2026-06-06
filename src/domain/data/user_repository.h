#pragma once

#include <optional>
#include <vector>

#include "domain/model/user.h"

class UserRepository {
  public:
    User addUser(const std::string& username, const std::string& password_hash, UserRole role);
    std::optional<User> findByUsername(const std::string& username) const;
    std::optional<User> findById(int id) const;
    const std::vector<User>& getAllUsers() const;
    size_t count() const;

  private:
    std::vector<User> users_;
    int getNextId() const;
};