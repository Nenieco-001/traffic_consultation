#pragma once

#include <functional>
#include <optional>
#include <vector>

#include "application/auth_service.h"
#include "application/consult_controller.h"

struct MenuItem {
    std::string text_;
    std::function<void()> action_;
};

struct MenuPage {
    std::string title_;
    std::vector<MenuItem> items_;
};

// 页面栈式菜单，依赖 ConsultController + AuthService
class Menu {
  public:
    // 页面索引 — 构造函数按此顺序注册，各 handleXxx 通过 pushPage(index) 跳转
    static constexpr int PAGE_LOGIN = 0;
    static constexpr int PAGE_MAIN = 1;  // 登录后按角色动态构建
    static constexpr int PAGE_CONSULT = 2;
    static constexpr int PAGE_CITY = 3;
    static constexpr int PAGE_SCHEDULE = 4;
    static constexpr int PAGE_DATA_MGMT = 5;  // 导出/导入/统计

    Menu(ConsultController& consultController, AuthService& authService);
    void showMainMenu();

    int addPage(const MenuPage& page);
    void popPage();
    void pushPage(int page_index);

    // 认证
    void handleLogin();
    void handleRegister(bool force_admin = false);
    void handleLogout();

    // 交通咨询
    void handleTrafficConsultation();

    // 城市管理
    void handleAddCity();
    void handleRemoveCity();  // 含级联班次警告

    // 班次管理
    void handleAddTrip();
    void handleModifyTrip();  // 修改班次：重新输入所有字段
    void handleRemoveTrip();

    // 数据管理
    void handleExportData();
    void handleImportData();
    void handleShowStatistics();

    void showPathDetails(const Path& path);
    static void clearScreen();

  private:
    ConsultController& consultController;  // 注入：查询 + CRUD
    AuthService& auth_service_;            // 注入：登录 + 注册
    std::optional<User> current_user_;     // 当前登录用户，nullopt 表示未登录

    std::vector<MenuPage> pages_;
    std::vector<int> page_stack_;

    int currentPage() const;
    void renderCurrentPage();
    void rebuildMainMenu();  // 根据 current_user_->role_ 决定菜单项

    // --- 输入验证及权限辅助函数 ---
    bool requireAdmin();
    void waitForEnter(const std::string& prompt = "\n按回车键继续...");

    std::optional<int> readInt(const std::string& prompt, const std::string& error_msg);
    std::optional<int> readIntInRange(const std::string& prompt, int min, int max,
                                      const std::string& error_msg);

    bool readNonEmptyString(const std::string& prompt, std::string& out,
                            const std::string& error_msg);

    std::optional<int> readTime(const std::string& prompt);
};
