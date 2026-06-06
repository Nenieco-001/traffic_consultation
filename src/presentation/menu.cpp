#include "presentation/menu.h"

#include <filesystem>
#include <iostream>
#include <limits>

#include "domain/tools/time_process.h"
#include "infrastructure/file_io.h"

// ============================================================
// 构造函数：依次注册 6 个页面，初始栈 = [PAGE_LOGIN]
// ============================================================
Menu::Menu(ConsultController& consultController, AuthService& authService)
    : consultController(consultController), auth_service_(authService) {
    // Page 0: 登录/注册
    MenuPage login_page;
    login_page.title_ = "全国交通咨询系统 - 登录";
    login_page.items_ = {
        {"登录",
         [this] {
             handleLogin();
         }},
        {"注册",
         [this] {
             handleRegister();
         }},
        {"退出系统",
         [this] {
             page_stack_.clear();
         }},  // 清空栈 → showMainMenu 退出循环
    };
    addPage(login_page);

    // Page 1: 主菜单 — 占位，登录后由 rebuildMainMenu() 填充
    MenuPage main_page;
    main_page.title_ = "全国交通咨询系统";
    addPage(main_page);

    // Page 2: 交通咨询
    MenuPage consult_page;
    consult_page.title_ = "交通咨询";
    consult_page.items_ = {
        {"开始查询",
         [this] {
             handleTrafficConsultation();
         }},
        {"返回上一级",
         [this] {
             popPage();
         }},
    };
    addPage(consult_page);

    // Page 3: 城市管理
    MenuPage city_page;
    city_page.title_ = "城市管理";
    city_page.items_ = {
        {"添加城市",
         [this] {
             handleAddCity();
         }},
        {"删除城市",
         [this] {
             handleRemoveCity();
         }},
        {"返回上一级",
         [this] {
             popPage();
         }},
    };
    addPage(city_page);

    // Page 4: 班次管理
    MenuPage schedule_page;
    schedule_page.title_ = "班次管理";
    schedule_page.items_ = {
        {"添加班次",
         [this] {
             handleAddTrip();
         }},
        {"修改班次",
         [this] {
             handleModifyTrip();
         }},
        {"删除班次",
         [this] {
             handleRemoveTrip();
         }},
        {"返回上一级",
         [this] {
             popPage();
         }},
    };
    addPage(schedule_page);

    // Page 5: 数据管理
    MenuPage data_page;
    data_page.title_ = "数据管理";
    data_page.items_ = {
        {"导出数据",
         [this] {
             handleExportData();
         }},
        {"导入数据",
         [this] {
             handleImportData();
         }},
        {"数据统计",
         [this] {
             handleShowStatistics();
         }},
        {"返回上一级",
         [this] {
             popPage();
         }},
    };
    addPage(data_page);

    page_stack_.push_back(PAGE_LOGIN);
}

// ============================================================
// 页面栈操作：pushPage → 进入子页，popPage → 返回上级
// ============================================================
int Menu::currentPage() const {
    if (page_stack_.empty())
        return -1;
    return page_stack_.back();
}

int Menu::addPage(const MenuPage& page) {
    pages_.push_back(page);
    return static_cast<int>(pages_.size() - 1);
}

void Menu::popPage() {
    if (!page_stack_.empty())
        page_stack_.pop_back();
}

void Menu::pushPage(int page_index) {
    if (page_index >= 0 && page_index < static_cast<int>(pages_.size()))
        page_stack_.push_back(page_index);
}

// ============================================================
// 认证
// ============================================================
void Menu::handleLogin() {
    clearScreen();
    std::cout << "===== 登录 =====" << std::endl;

    std::string username, password;
    std::cout << "用户名: ";
    std::cin >> username;
    if (username.empty()) {
        std::cout << "用户名不能为空" << std::endl;
        std::cout << "\n按回车键继续...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }
    std::cout << "密码: ";
    std::cin >> password;
    if (password.empty()) {
        std::cout << "密码不能为空" << std::endl;
        std::cout << "\n按回车键继续...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    auto user = auth_service_.login(username, password);
    if (user.has_value()) {
        current_user_ = user.value();
        rebuildMainMenu();  // 关键：根据 role 重新生成主菜单项
        page_stack_.clear();
        page_stack_.push_back(PAGE_MAIN);
        std::cout << "登录成功！欢迎 " << username << std::endl;
    } else {
        std::cout << "登录失败：用户名或密码错误" << std::endl;
    }

    std::cout << "\n按回车键继续...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

void Menu::handleRegister(bool force_admin) {
    clearScreen();
    std::cout << "===== 注册 =====" << std::endl;

    std::string username, password;
    std::cout << "用户名: ";
    std::cin >> username;
    if (username.empty()) {
        std::cout << "用户名不能为空" << std::endl;
        std::cout << "\n按回车键继续...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }
    std::cout << "密码: ";
    std::cin >> password;
    if (password.empty()) {
        std::cout << "密码不能为空" << std::endl;
        std::cout << "\n按回车键继续...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    UserRole role = UserRole::ADMIN;
    if (!force_admin) {
        std::cout << "角色 (1=普通用户 2=管理员): ";
        int role_choice;
        if (!(std::cin >> role_choice)|| role_choice < 1 || role_choice > 2) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "输入无效，默认为普通用户" << std::endl;
            role_choice = 1;
        }
        role = (role_choice == 2) ? UserRole::ADMIN : UserRole::NORMAL;
    }

    auto user = auth_service_.registerUser(username, password, role);
    if (user.has_value()) {
        // 注册成功自动登录，减少用户操作步骤
        current_user_ = user.value();   // TIPS:可以使用移动语义 std::move(user.value())，但这里 User 结构体较小，直接复制也无大碍
        rebuildMainMenu();
        page_stack_.clear();
        page_stack_.push_back(PAGE_MAIN);
        std::cout << "注册成功！已自动登录。欢迎 " << username << std::endl;
    } else {
        std::cout << "注册失败：用户名已存在" << std::endl;
    }

    std::cout << "\n按回车键继续...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

void Menu::handleLogout() {
    current_user_.reset();
    page_stack_.clear();
    page_stack_.push_back(PAGE_LOGIN);

    clearScreen();
    std::cout << "已退出登录" << std::endl;
    std::cout << "\n按回车键继续...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

// ============================================================
// 主菜单重建：根据角色展示不同功能入口
//   ADMIN → 交通咨询 + 城市管理 + 班次管理 + 数据管理 + 退出登录
//   NORMAL → 交通咨询 + 退出登录
// ============================================================
void Menu::rebuildMainMenu() {
    auto& main_page = pages_[PAGE_MAIN];
    main_page.items_.clear();

    main_page.items_.push_back({"交通咨询", [this] {
                                    pushPage(PAGE_CONSULT);
                                }});

    if (current_user_.has_value() && current_user_->role_ == UserRole::ADMIN) {
        main_page.items_.push_back({"城市管理", [this] {
                                        pushPage(PAGE_CITY);
                                    }});
        main_page.items_.push_back({"班次管理", [this] {
                                        pushPage(PAGE_SCHEDULE);
                                    }});
        main_page.items_.push_back({"数据管理", [this] {
                                        pushPage(PAGE_DATA_MGMT);
                                    }});
    }

    main_page.items_.push_back({"退出登录", [this] {
                                    handleLogout();
                                }});
}

// ============================================================
// 页面渲染
// ============================================================
void Menu::renderCurrentPage() {
    clearScreen();

    int idx = currentPage();
    if (idx < 0)
        return;

    const auto& page = pages_[idx];
    std::cout << "===== " << page.title_ << " =====" << std::endl;

    // 不同页面展示专属数据：咨询/城市页显示城市列表，班次页显示班次列表
    switch (idx) {
        case PAGE_CONSULT:
        case PAGE_CITY:
            std::cout << "\n--- 城市列表 ---\n";
            for (const auto& city : consultController.getData().getAllCities())
                std::cout << city.id_ << ". " << city.name_ << "\n";
            break;

        case PAGE_SCHEDULE:
            std::cout << "\n--- 班次列表 ---\n";
            for (const auto& trip : consultController.getData().getAllTrips())
                std::cout << "ID:" << trip.id_ << " " << trip.trip_number_ << " (" << trip.type_ << ") "
                          << trip.from_city_id_ << " -> " << trip.to_city_id_
                          << ", 出发:" << minutesToTimeStr(trip.departure_time_)
                          << " 到达:" << minutesToTimeStr(trip.arrival_time_) << " " << trip.price_ << "元\n";
            break;

        default: break;
    }

    if (idx == PAGE_MAIN && current_user_.has_value()) {
        std::cout << "\n当前用户: " << current_user_->username_ << " ("
                  << (current_user_->role_ == UserRole::ADMIN ? "管理员" : "普通用户") << ")";
    }

    std::cout << "\n\n--- 菜单 ---\n";
    for (size_t i = 0; i < page.items_.size(); ++i)
        std::cout << (i + 1) << ". " << page.items_[i].text_ << "\n";

    std::cout << "\n请选择: ";
}

// ============================================================
// 主事件循环：渲染 → 输入 → 执行，page_stack_ 空则退出
// ============================================================
void Menu::showMainMenu() {
    // 首次运行：无用户时自动跳转注册，强制注册为管理员
    if (!auth_service_.hasAnyUser()) {
        clearScreen();
        std::cout << "检测到系统首次运行，请注册管理员账号。\n" << std::endl;
        std::cout << "按回车键开始注册...";
        std::cin.get();
        handleRegister(true);
        if (page_stack_.empty())
            return;  // 注册异常（如 EOF）则直接退出
    }

    while (!page_stack_.empty()) {
        renderCurrentPage();

        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        int idx = currentPage();
        if (idx < 0)
            break;

        const auto& page = pages_[idx];
        if (choice >= 1 && choice <= static_cast<int>(page.items_.size()))
            page.items_[choice - 1].action_();
    }

    clearScreen();
    std::cout << "感谢使用，再见！" << std::endl;
}

// ============================================================
// 交通咨询：选城市 → 输时间/策略 → 展示结果 → 可选查看详情
// ============================================================
void Menu::handleTrafficConsultation() {
    clearScreen();
    std::cout << "===== 交通咨询 - 查询 =====" << std::endl;

    std::cout << "\n--- 城市列表 ---\n";
    for (const auto& city : consultController.getData().getAllCities())
        std::cout << city.id_ << ". " << city.name_ << "\n";

    int from_id, to_id, transport_choice, strategy_choice;
    std::string time_str;

    std::cout << "\n请输入出发城市编号: ";
    if (!(std::cin >> from_id)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "城市编号无效" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.get();
        return;
    }
    // 验证出发城市存在
    bool from_ok = false;
    for (const auto& c : consultController.getData().getAllCities())
        if (c.id_ == from_id) { from_ok = true; break; }
    if (!from_ok) {
        std::cout << "城市编号 " << from_id << " 不存在" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    std::cout << "请输入到达城市编号: ";
    if (!(std::cin >> to_id)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "城市编号无效" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }
    // 验证到达城市存在
    bool to_ok = false;
    for (const auto& c : consultController.getData().getAllCities())
        if (c.id_ == to_id) { to_ok = true; break; }
    if (!to_ok) {
        std::cout << "城市编号 " << to_id << " 不存在" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }
    if (from_id == to_id) {
        std::cout << "出发城市和到达城市不能相同" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    // 输入出发时间、交通工具、查询策略，均有基本验证
    std::cout << "请输入出发时间（HH:MM）: ";
    std::cin >> time_str;
    int depart_time = timeStrToMinutes(time_str);
    if (depart_time < 0) {
        std::cout << "时间格式错误，请输入 HH:MM（如 08:30）" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    // 交通工具：1=火车 2=飞机 3=不限
    std::cout << "交通工具 (1=火车 2=飞机 3=不限): ";
    if (!(std::cin >> transport_choice) || transport_choice < 1 || transport_choice > 3) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "选择无效，请输入 1-3" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }
    TransportType transport = TransportType::TRAIN;
    if (transport_choice == 2)
        transport = TransportType::PLANE;
    else if (transport_choice == 3)
        transport = TransportType::MIXED;

    // 查询策略：1=最快 2=最便宜 3=最少换乘
    std::cout << "查询策略 (1=最快 2=最便宜 3=最少换乘): ";
    if (!(std::cin >> strategy_choice) || strategy_choice < 1 || strategy_choice > 3) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "选择无效，请输入 1-3" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }
    Strategy strategy = Strategy::FASTEST;
    if (strategy_choice == 2)
        strategy = Strategy::CHEAPEST;
    else if (strategy_choice == 3)
        strategy = Strategy::LEAST_TRANSFERS;

    // 构造查询请求并执行，展示结果
    QueryRequest req;
    req.from_city_id_ = from_id;
    req.to_city_id_ = to_id;
    req.transport_ = transport;
    req.strategy_ = strategy;
    req.depart_after_ = depart_time;

    // 委托 ConsultController 处理查询逻辑，获取结果
    QueryResult result = consultController.query(req);

    clearScreen();
    std::cout << "===== 查询结果 =====" << std::endl;
    if (!result.error_msg.empty()) {
        std::cout << "未找到可行路径。原因: " << result.error_msg << std::endl;
    } else {
        for (size_t i = 0; i < result.paths.size(); ++i) {
            const auto& path = result.paths[i];
            std::cout << (i + 1) << ". 总耗时: " << path.total_time_ << "分钟, " << "总票价: " << path.total_price_
                      << "元, " << "换乘: " << path.transfer_count_ << "次\n";
        }

        int detail_choice;
        std::cout << "\n请输入要查看详情的路径编号 (0=跳过): ";
        std::cin >> detail_choice;
        if (detail_choice >= 1 && detail_choice <= static_cast<int>(result.paths.size()))
            showPathDetails(result.paths[detail_choice - 1]);
    }

    std::cout << "\n按回车键返回...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

// ============================================================
// 城市管理
// ============================================================
void Menu::handleAddCity() {
    // 权限检查：仅管理员可添加城市 - （正常情况下用户无法看到这个选项，但这里做双重保险）
    if (!current_user_.has_value() || current_user_->role_ != UserRole::ADMIN) {
        std::cout << "无权限：仅管理员可执行此操作" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    clearScreen();
    std::cout << "===== 添加城市 =====" << std::endl;

    std::string name;
    std::cout << "请输入城市名称: ";
    std::cin >> name;
    if (name.empty()) {
        std::cout << "城市名称不能为空" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }
    // 检查重名
    // TIPS: 这里简单线性查找，数据量大时可优化为哈希表或数据库索引
    for (const auto& c : consultController.getData().getAllCities()) {
        if (c.name_ == name) {
            std::cout << "城市 \"" << name << "\" 已存在" << std::endl;
            std::cout << "\n按回车键返回...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin.get();
            return;
        }
    }

    // 自增 ID：取当前最大 id + 1，简单够用（小规模数据）
    // TIPS: 生产环境建议使用 UUID 或数据库自增主键，避免删除后 ID 重复等问题
    int new_id = 1;
    for (const auto& c : consultController.getData().getAllCities())
        if (c.id_ >= new_id)
            new_id = c.id_ + 1;

    consultController.addCity(City{new_id, name});
    consultController.saveData();
    std::cout << "已添加城市: " << name << " (id=" << new_id << ")" << std::endl;

    std::cout << "\n按回车键返回...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

/**
 * 删除城市（含级联警告）：
 * 1. 查询该城市关联的所有班次，列出详情
 * 2. 有关联 → 警告并要求 y/n 确认；无关联 → 直接继续
 * 3. 确认后 TransportData::removeCity() 内部会 std::erase_if 删除城市和关联班次
 */
void Menu::handleRemoveCity() {
    if (!current_user_.has_value() || current_user_->role_ != UserRole::ADMIN) {
        std::cout << "无权限：仅管理员可执行此操作" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    clearScreen();
    std::cout << "===== 删除城市 =====" << std::endl;

    for (const auto& city : consultController.getData().getAllCities())
        std::cout << city.id_ << ". " << city.name_ << "\n";

    int city_id;
    std::cout << "\n请输入要删除的城市编号: ";
    if (!(std::cin >> city_id)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "城市编号无效" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    // 检查城市是否存在并获取名称
    std::string city_name;
    for (const auto& c : consultController.getData().getAllCities()) {
        if (c.id_ == city_id) {
            city_name = c.name_;
            break;
        }
    }
    if (city_name.empty()) {
        std::cout << "城市编号不存在" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    // ---- 级联检查关键代码 ----
    // 先查关联班次（不会修改数据），再决定是否删除
    auto affected = consultController.getData().getTripsByCity(city_id);
    if (!affected.empty()) {
        std::cout << "\n⚠ 警告：城市 \"" << city_name << "\" (id=" << city_id << ") 下有 " << affected.size()
                  << " 条关联班次：\n";
        for (const auto& t : affected) {
            std::cout << "  ID:" << t.id_ << " " << t.trip_number_ << " (" << t.type_ << ") " << t.from_city_id_
                      << " -> " << t.to_city_id_ << ", " << minutesToTimeStr(t.departure_time_) << " - "
                      << minutesToTimeStr(t.arrival_time_) << ", " << t.price_ << "元\n";
        }
        std::cout << "删除城市将同时删除以上 " << affected.size() << " 条班次。\n";
        std::cout << "是否继续？(y/n): ";
        std::string confirm;
        std::cin >> confirm;
        if (confirm != "y" && confirm != "Y") {
            std::cout << "已取消删除" << std::endl;
            std::cout << "\n按回车键返回...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin.get();
            return;
        }
    } else {
        std::cout << "该城市下无关联班次，可直接删除。" << std::endl;
    }

    consultController.removeCity(city_id);
    consultController.saveData();
    std::cout << "已删除城市: " << city_name << " (id=" << city_id << ")" << std::endl;

    std::cout << "\n按回车键返回...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

// ============================================================
// 班次管理
// ============================================================
void Menu::handleAddTrip() {
    if (!current_user_.has_value() || current_user_->role_ != UserRole::ADMIN) {
        std::cout << "无权限：仅管理员可执行此操作" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    clearScreen();
    std::cout << "===== 添加班次 =====" << std::endl;

    Trip trip;
    std::cout << "班次号（如 G123）: ";
    std::cin >> trip.trip_number_;
    if (trip.trip_number_.empty()) {
        std::cout << "班次号不能为空" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    std::string type_str;
    std::cout << "类型 (火车/飞机): ";
    std::cin >> type_str;
    if (type_str != "火车" && type_str != "飞机") {
        std::cout << "类型无效，请输入 火车 或 飞机" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }
    trip.type_ = (type_str == "飞机") ? TransportType::PLANE : TransportType::TRAIN;

    const auto& cities = consultController.getData().getAllCities();

    std::cout << "出发城市编号: ";
    if (!(std::cin >> trip.from_city_id_)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "城市编号无效" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }
    // 验证出发城市存在
    bool from_ok = false;
    for (const auto& c : cities)
        if (c.id_ == trip.from_city_id_) { from_ok = true; break; }
    if (!from_ok) {
        std::cout << "城市编号 " << trip.from_city_id_ << " 不存在" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    std::cout << "到达城市编号: ";
    if (!(std::cin >> trip.to_city_id_)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "城市编号无效" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }
    // 验证到达城市存在
    bool to_ok = false;
    for (const auto& c : cities)
        if (c.id_ == trip.to_city_id_) { to_ok = true; break; }
    if (!to_ok) {
        std::cout << "城市编号 " << trip.to_city_id_ << " 不存在" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }
    if (trip.from_city_id_ == trip.to_city_id_) {
        std::cout << "出发城市和到达城市不能相同" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    std::string dep_str, arr_str;
    std::cout << "出发时间（HH:MM）: ";
    std::cin >> dep_str;
    trip.departure_time_ = timeStrToMinutes(dep_str);
    if (trip.departure_time_ < 0) {
        std::cout << "时间格式错误，请输入 HH:MM（如 08:30）" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    std::cout << "到达时间（HH:MM）: ";
    std::cin >> arr_str;
    trip.arrival_time_ = timeStrToMinutes(arr_str);
    if (trip.arrival_time_ < 0) {
        std::cout << "时间格式错误，请输入 HH:MM（如 08:30）" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }
    if (trip.arrival_time_ <= trip.departure_time_) {
        std::cout << "到达时间必须晚于出发时间" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    std::cout << "票价: ";
    if (!(std::cin >> trip.price_) || trip.price_ < 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "票价无效，请输入非负整数" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    consultController.addTrip(trip);
    consultController.saveData();
    std::cout << "已添加班次: " << trip.trip_number_ << std::endl;

    std::cout << "\n按回车键返回...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

// ============================================================
// 修改班次：选择已有班次 → 显示当前值 → 重新输入所有字段
// ============================================================
void Menu::handleModifyTrip() {
    if (!current_user_.has_value() || current_user_->role_ != UserRole::ADMIN) {
        std::cout << "无权限：仅管理员可执行此操作" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    clearScreen();
    std::cout << "===== 修改班次 =====" << std::endl;

    // 显示当前班次列表
    const auto& all_trips = consultController.getData().getAllTrips();
    for (const auto& trip : all_trips)
        std::cout << "ID:" << trip.id_ << "  " << trip.trip_number_ << " (" << trip.type_ << ") "
                  << trip.from_city_id_ << " -> " << trip.to_city_id_
                  << ", 出发:" << minutesToTimeStr(trip.departure_time_)
                  << " 到达:" << minutesToTimeStr(trip.arrival_time_) << " " << trip.price_ << "元\n";

    int trip_id;
    std::cout << "\n请输入要修改的班次 ID: ";
    if (!(std::cin >> trip_id)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "班次 ID 无效" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    // 查找原班次
    Trip old_trip;
    bool found = false;
    for (const auto& t : all_trips) {
        if (t.id_ == trip_id) {
            old_trip = t;
            found = true;
            break;
        }
    }
    if (!found) {
        std::cout << "班次 ID " << trip_id << " 不存在" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    // 显示当前值并引导重新输入
    std::cout << "\n当前班次: " << old_trip.trip_number_ << std::endl;
    Trip new_trip = old_trip;  // 保留原值，后面覆盖

    std::string input;
    std::cout << "班次号（当前: " << old_trip.trip_number_ << "）: ";
    std::cin >> input;
    if (!input.empty())
        new_trip.trip_number_ = input;

    std::cout << "类型（当前: " << (old_trip.type_ == TransportType::TRAIN ? "火车" : "飞机") << "）(火车/飞机): ";
    std::cin >> input;
    if (input == "火车" || input == "飞机")
        new_trip.type_ = (input == "飞机") ? TransportType::PLANE : TransportType::TRAIN;

    const auto& cities = consultController.getData().getAllCities();

    std::cout << "出发城市编号（当前: " << old_trip.from_city_id_ << "）: ";
    if (!(std::cin >> new_trip.from_city_id_)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "城市编号无效" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }
    // 验证出发城市
    bool from_ok = false;
    for (const auto& c : cities)
        if (c.id_ == new_trip.from_city_id_) { from_ok = true; break; }
    if (!from_ok) {
        std::cout << "城市编号 " << new_trip.from_city_id_ << " 不存在" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    std::cout << "到达城市编号（当前: " << old_trip.to_city_id_ << "）: ";
    if (!(std::cin >> new_trip.to_city_id_)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "城市编号无效" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }
    // 验证到达城市
    bool to_ok = false;
    for (const auto& c : cities)
        if (c.id_ == new_trip.to_city_id_) { to_ok = true; break; }
    if (!to_ok) {
        std::cout << "城市编号 " << new_trip.to_city_id_ << " 不存在" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }
    if (new_trip.from_city_id_ == new_trip.to_city_id_) {
        std::cout << "出发城市和到达城市不能相同" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    std::string dep_str, arr_str;
    std::cout << "出发时间 HH:MM（当前: " << minutesToTimeStr(old_trip.departure_time_) << "）: ";
    std::cin >> dep_str;
    int dep = timeStrToMinutes(dep_str);
    if (dep >= 0)
        new_trip.departure_time_ = dep;

    std::cout << "到达时间 HH:MM（当前: " << minutesToTimeStr(old_trip.arrival_time_) << "）: ";
    std::cin >> arr_str;
    int arr = timeStrToMinutes(arr_str);
    if (arr >= 0)
        new_trip.arrival_time_ = arr;

    if (new_trip.arrival_time_ <= new_trip.departure_time_) {
        std::cout << "到达时间必须晚于出发时间" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    std::cout << "票价（当前: " << old_trip.price_ << "）: ";
    if (!(std::cin >> new_trip.price_) || new_trip.price_ < 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "票价无效，请输入非负整数" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    // 修改班次（保持原 id 不变）
    new_trip.id_ = old_trip.id_;
    if (consultController.modifyTrip(old_trip.id_, new_trip)) {
        consultController.saveData();
        std::cout << "班次已修改: " << new_trip.trip_number_ << " (ID=" << old_trip.id_ << ")" << std::endl;
    } else {
        std::cout << "修改失败：班次不存在" << std::endl;
    }

    std::cout << "\n按回车键返回...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

void Menu::handleRemoveTrip() {
    if (!current_user_.has_value() || current_user_->role_ != UserRole::ADMIN) {
        std::cout << "无权限：仅管理员可执行此操作" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    clearScreen();
    std::cout << "===== 删除班次 =====" << std::endl;

    for (const auto& trip : consultController.getData().getAllTrips())
        std::cout << "ID:" << trip.id_ << "  " << trip.trip_number_ << " (" << trip.type_ << ") " << trip.from_city_id_
                  << " -> " << trip.to_city_id_ << "\n";

    int trip_id;
    std::cout << "\n请输入要删除的班次 ID: ";
    if (!(std::cin >> trip_id)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "班次 ID 无效" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }
    // 验证班次存在
    bool trip_found = false;
    for (const auto& t : consultController.getData().getAllTrips())
        if (t.id_ == trip_id) { trip_found = true; break; }
    if (!trip_found) {
        std::cout << "班次 ID " << trip_id << " 不存在" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    consultController.removeTrip(trip_id);
    consultController.saveData();
    std::cout << "已删除班次 ID: " << trip_id << std::endl;

    std::cout << "\n按回车键返回...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

// ============================================================
// 数据管理：导出 / 导入（预览确认）/ 统计
// ============================================================
void Menu::handleExportData() {
    if (!current_user_.has_value() || current_user_->role_ != UserRole::ADMIN) {
        std::cout << "无权限：仅管理员可执行此操作" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    clearScreen();
    std::cout << "===== 导出数据 =====" << std::endl;

    std::string dir;
    std::cout << "请输入目标目录路径: ";
    std::cin >> dir;
    if (dir.empty()) {
        std::cout << "目录路径不能为空" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    try {
        // saveData(dir) 会在 dir 下创建 city.dat, train_schedules.dat, flight_schedules.dat
        consultController.saveData(dir);
        std::cout << "已导出到: " << dir << std::endl;
        std::cout << "  - city.dat\n  - train_schedules.dat\n  - flight_schedules.dat" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "导出失败: " << e.what() << std::endl;
    }

    std::cout << "\n按回车键返回...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

/**
 * 导入数据流程：
 * 1. file_io::loadFromFile(dir) 预览 — 只读不修改当前数据
 * 2. 展示预览结果让用户判断是否正确
 * 3. 确认后正式 loadData + saveData（覆盖生产目录）
 * 4. 取消则数据不受影响
 */
void Menu::handleImportData() {
    if (!current_user_.has_value() || current_user_->role_ != UserRole::ADMIN) {
        std::cout << "无权限：仅管理员可执行此操作" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    clearScreen();
    std::cout << "===== 导入数据 =====" << std::endl;

    std::string dir;
    std::cout << "请输入源目录路径: ";
    std::cin >> dir;
    if (dir.empty()) {
        std::cout << "目录路径不能为空" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }
    if (!std::filesystem::exists(dir)) {
        std::cout << "目录路径不存在: " << dir << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    try {
        // 预览：先读取但不覆盖，让用户确认
        auto preview = file_io::loadFromFile(dir);
        std::cout << "\n发现数据：" << std::endl;
        std::cout << "  城市: " << preview.getAllCities().size() << " 个" << std::endl;
        std::cout << "  班次: " << preview.getAllTrips().size() << " 个" << std::endl;

        std::cout << "\n是否确认导入并覆盖当前数据？(y/n): ";
        std::string confirm;
        std::cin >> confirm;
        if (confirm != "y" && confirm != "Y") {
            std::cout << "已取消导入" << std::endl;
            std::cout << "\n按回车键返回...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin.get();
            return;
        }

        consultController.loadData(dir);  // 加载
        consultController.saveData();     // 同步到生产目录
        std::cout << "导入成功！当前数据已更新。" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "导入失败: " << e.what() << std::endl;
    }

    std::cout << "\n按回车键返回...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

/**
 * 数据统计：遍历所有班次，计算：
 * - 城市数、班次数
 * - 火车/飞机数量及占比
 * - 票价区间、平均票价
 * - 最早/最晚出发时间
 */
void Menu::handleShowStatistics() {
    if (!current_user_.has_value() || current_user_->role_ != UserRole::ADMIN) {
        std::cout << "无权限：仅管理员可执行此操作" << std::endl;
        std::cout << "\n按回车键返回...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
        return;
    }

    clearScreen();
    std::cout << "===== 数据统计 =====" << std::endl;

    const auto& data = consultController.getData();
    const auto& cities = data.getAllCities();
    const auto& trips = data.getAllTrips();

    size_t city_count = cities.size();
    size_t trip_count = trips.size();
    size_t train_count = 0, plane_count = 0;
    int min_price = std::numeric_limits<int>::max();
    int max_price = 0;
    long long price_sum = 0;
    int min_departure = 1440, max_departure = 0;

    // 一次遍历完成所有统计，O(n)
    for (const auto& t : trips) {
        if (t.type_ == TransportType::TRAIN)
            ++train_count;
        else
            ++plane_count;
        if (t.price_ < min_price)
            min_price = t.price_;
        if (t.price_ > max_price)
            max_price = t.price_;
        price_sum += t.price_;
        if (t.departure_time_ < min_departure)
            min_departure = t.departure_time_;
        if (t.departure_time_ > max_departure)
            max_departure = t.departure_time_;
    }

    std::cout << "\n--- 城市 ---" << std::endl;
    std::cout << "  城市总数: " << city_count << std::endl;

    std::cout << "\n--- 班次 ---" << std::endl;
    std::cout << "  班次总数: " << trip_count << std::endl;
    if (trip_count > 0) {
        double train_pct = 100.0 * train_count / trip_count;
        double plane_pct = 100.0 * plane_count / trip_count;
        std::cout << "  火车: " << train_count << "  (" << train_pct << "%)" << std::endl;
        std::cout << "  飞机: " << plane_count << "  (" << plane_pct << "%)" << std::endl;
    }

    if (trip_count > 0) {
        double avg_price = static_cast<double>(price_sum) / trip_count;
        std::cout << "\n--- 票价 ---" << std::endl;
        std::cout << "  区间: " << min_price << " - " << max_price << " 元" << std::endl;
        std::cout << "  平均: " << avg_price << " 元" << std::endl;
        std::cout << "\n--- 时间 ---" << std::endl;
        std::cout << "  最早班次: " << minutesToTimeStr(min_departure) << std::endl;
        std::cout << "  最晚班次: " << minutesToTimeStr(max_departure) << std::endl;
    }

    std::cout << "\n按回车键返回...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

// ============================================================
// 路径详情
// ============================================================
void Menu::showPathDetails(const Path& path) {
    clearScreen();
    std::cout << "===== 路径详情 =====" << std::endl;
    for (const auto& seg : path.segments_) {
        std::cout << "班次: " << seg.trip_.trip_number_ << " (" << seg.trip_.type_ << "), " << seg.trip_.from_city_id_
                  << " -> " << seg.trip_.to_city_id_ << "\n"
                  << "  出发: " << minutesToTimeStr(seg.trip_.departure_time_)
                  << "  到达: " << minutesToTimeStr(seg.trip_.arrival_time_) << "\n"
                  << "  等待: " << (seg.wait_time_ == 0 ? "-" : std::to_string(seg.wait_time_) + "分钟")
                  << "  票价: " << seg.trip_.price_ << "元\n";
        std::cout << "-----------------------------\n";
    }
    std::cout << "总耗时: " << path.total_time_ << "分钟, " << "总票价: " << path.total_price_ << "元, "
              << "换乘: " << path.transfer_count_ << "次\n"
              << std::endl;
}

// ============================================================
// 清屏
// ============================================================
void Menu::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}


// TODO:当前的输入和权限检查代码在每个函数里都有重复，可以考虑封装成一个辅助函数来简化