#include "presentation/menu.h"
#include "domain/tools/time_process.h"

// 菜单实现
Menu::Menu(ConsultController& consultController) : consultController(consultController) {}

// 主菜单循环
void Menu::showMainMenu() {
    // 清屏
    clearScreen();
    // 主菜单循环
    while (true) {
        std::cout << "===== 全国交通咨询系统 =====" << std::endl;
        std::cout << "1. 交通咨询" << std::endl;
        std::cout << "2. 城市管理" << std::endl;
        std::cout << "3. 班次管理" << std::endl;
        std::cout << "0. 退出" << std::endl;
        std::cout << "=============================" << std::endl;
        int choice;
        std::cin >> choice;
        switch (choice) {
            case 1: handleTrafficConsultation(); break;
            case 2: handleCityManagement(); break;
            case 3: handleScheduleManagement(); break;
            case 0: return;  // 退出菜单循环
            default: std::cout << "无效选择，请重新输入。" << std::endl;
        }
    }
}

// 处理交通咨询
void Menu::handleTrafficConsultation() {
    // 清屏
    clearScreen();
    // 显示城市列表
    std::cout << "===== 交通咨询 =====" << std::endl;
    std::cout << "===== 城市列表 =====" << std::endl;
    // 显示城市列表
    for (const auto& city : consultController.getData().getAllCities()) {  // 直接访问数据成员，简化示例
        std::cout << city.id_ << ". " << city.name_ << std::endl;
    }
    // 选择起止城市与出发时间
    int from_city_id, to_city_id, depart_time;
    std::cout << "请输入出发城市编号: ";
    std::cin >> from_city_id;
    // TODO(input): cin 失败时流进入错误状态，后续所有读取静默返回 0，需添加 cin.good() 检查
    std::cout << "请输入到达城市编号: ";
    std::cin >> to_city_id;
    std::cout << "请输入出发时间（HH:MM，例如 08:30）: ";
    // TODO: 后续用户可指定具体日期，当前示例先假设当天出发，输入时间转换为绝对分钟
    std::string time_str;
    std::cin >> time_str;
    // 转换为绝对分钟
    depart_time = timeStrToMinutes(time_str);

    // TODO:所有输入后续都要强化验证，当前示例先假设输入合法
    // 选择交通工具
    int transport_choice;
    std::cout << "请选择交通工具: 1. 只坐火车 2. 只坐飞机 3.均可: ";
    std::cin >> transport_choice;
    TransportType transport{};
    switch (transport_choice) {
        case 1: transport = TransportType::TRAIN; break;
        case 2: transport = TransportType::PLANE; break;
        case 3: transport = TransportType::MIXED; break;
        default: std::cout << "无效交通工具选择，默认使用火车。" << std::endl; transport = TransportType::TRAIN;
    }
    // 选择策略
    int strategy_choice;
    std::cout << "请选择查询策略: 1. 最快到达 2. 最省钱 3. 最少中转: ";
    std::cin >> strategy_choice;
    Strategy strategy;
    switch (strategy_choice) {
        case 1: strategy = Strategy::FASTEST; break;
        case 2: strategy = Strategy::CHEAPEST; break;
        case 3: strategy = Strategy::LEAST_TRANSFERS; break;
        default: std::cout << "无效策略选择，默认使用最快到达。" << std::endl; strategy = Strategy::FASTEST;
    }
    // 构造查询请求
    QueryRequest req;
    req.from_city_id_ = from_city_id;
    req.to_city_id_ = to_city_id;
    req.transport_ = transport;
    req.strategy_ = strategy;
    req.depart_after_ = depart_time;  // 使用转换后的绝对分钟数
    // 调用控制器查询
    QueryResult result = consultController.query(req);
    clearScreen();  // 查询结果展示前清屏
    // 解析结果
    if (!result.error_msg.empty()) {
        std::cout << "未找到可行路径。原因：" << result.error_msg << std::endl;
        return;
    }
    // 展示结果列表
    std::cout << "===== 查询结果 =====" << std::endl;
    for (size_t i = 0; i < result.paths.size(); ++i) {
        const auto& path = result.paths[i];
        std::cout << i + 1 << ". 总耗时: " << path.total_time_ << "分钟, " << "总票价: " << path.total_price_ << "元, "
                  << "换乘次数: " << path.transfer_count_ << std::endl;
    }
    // 用户输入编号查看详细行程
    int choice;
    std::cout << "请输入要详细查看的路径编号: ";
    std::cin >> choice;
    if (choice >= 1 && choice <= static_cast<int>(result.paths.size())) {
        showPathDetails(result.paths[choice - 1]);
    } else {
        std::cout << "无效的路径编号。" << std::endl;
    }
    // 按 0 返回主菜单，按 1 重新查询
    std::cout << "按 0 返回主菜单，按 1 重新查询: ";
    std::cin >> choice;
    if (choice == 0) {
        return;  // 返回主菜单
    } else if (choice == 1) {
        handleTrafficConsultation();  // 重新查询
        // TODO(robustness): 当前递归调用可能栈溢出，应改为 while 循环
    } else {
        std::cout << "无效选择，返回主菜单。" << std::endl;
        return;
    }
    // TODO: 需要加入循环允许用户误操作后重新输入等等返回上一页面的功能，当前示例先实现基本功能
    // TODO: 终端 UI 需要进一步优化显示效果，当前示例先实现基本功能
}

// 处理城市管理
void Menu::handleCityManagement() {
    clearScreen();
    std::cout << "===== 城市管理 =====" << std::endl;
    // 显示城市列表
    for (const auto& city : consultController.getData().getAllCities()) {  // 直接访问数据成员，简化示例
        std::cout << city.id_ << ". " << city.name_ << std::endl;
    }
    // 选择操作：添加城市、删除城市
    int choice;
    std::cout << "请选择操作: 1. 添加城市 2. 删除城市: ";
    std::cin >> choice;
    if (choice == 1) {
        // 添加城市
        std::string city_name;
        std::cout << "请输入要添加的城市名称: ";
        std::cin >> city_name;
        // 后续需要哈希表等方法来检查重复城市，当前示例先假设输入合法且不重复

        int new_id = 1;
        for (const auto& c : consultController.getData().getAllCities()) {
            if (c.id_ >= new_id)
                new_id = c.id_ + 1;  // 简单的 id 分配策略，找到当前最大 id 后加 1，实际应用中可改为更健壮的方案
        }
        consultController.addCity(City{new_id, city_name});
        consultController.saveData();
        // TODO(design): 删除城市后未级联删除关联班次，数据可能不一致
        std::cout << "已添加城市: " << city_name << " (id=" << new_id << ")" << std::endl;
    } else if (choice == 2) {
        // 删除城市
        int city_id;
        std::cout << "请输入要删除的城市编号: ";
        std::cin >> city_id;
        // 后续需要检查城市是否存在以及是否有关联的班次等，当前示例先假设输入合法且可以删除
        consultController.removeCity(city_id);
        consultController.saveData();
        std::cout << "已删除城市编号: " << city_id << std::endl;
    } else {
        std::cout << "无效选择。" << std::endl;
    }
    // 返回主菜单或继续管理城市
    std::cout << "按 0 返回主菜单，按 1 继续管理城市: ";
    std::cin >> choice;
    if (choice == 0) {
        return;  // 返回主菜单
    } else if (choice == 1) {
        handleCityManagement();  // 继续管理城市
    } else {
        std::cout << "无效选择，返回主菜单。" << std::endl;
        return;
    }
}


// 处理班次管理
// TODO:
// 班次管理功能需要进一步完善，例如删除班次、修改班次等功能，当前示例先实现添加班次的基本功能，此外班次显示应该显示指定的城市相关的班次，当前示例先显示所有班次以简化实现
void Menu::handleScheduleManagement() {
    clearScreen();
    std::cout << "===== 班次管理 =====" << std::endl;
    // 显示班次列表
    for (const auto& trip : consultController.getData().getAllTrips()) {  // 直接访问数据成员，简化示例
        std::cout << trip.trip_number_ << ". " << trip.type_ << ": " << trip.from_city_id_ << " → " << trip.to_city_id_
                  << ", 出发时间: " << minutesToTimeStr(trip.departure_time_)
                  << ", 到达时间: " << minutesToTimeStr(trip.arrival_time_) << ", 票价: " << trip.price_ << "元"
                  << std::endl;
    }
    // 选择操作：添加班次、删除班次
    int choice;
    std::cout << "请选择操作: 1. 添加班次 2. 删除班次: ";
    std::cin >> choice;
    if (choice == 1) {
        // 添加班次
        Trip new_trip;
        std::cout << "请输入要添加的班次信息。" << std::endl;
        std::cout << "班次号: ";
        std::cin >> new_trip.trip_number_;
        std::cout << "类型（火车/飞机）: ";
        std::string type_str;
        std::cin >> type_str;
        if (type_str == "火车") {
            new_trip.type_ = TransportType::TRAIN;
        } else if (type_str == "飞机") {
            new_trip.type_ = TransportType::PLANE;
        } else {
            std::cout << "无效的类型输入，默认使用火车。" << std::endl;
        }
        std::cout << "出发城市编号: ";
        std::cin >> new_trip.from_city_id_;
        std::cout << "到达城市编号: ";
        std::cin >> new_trip.to_city_id_;
        std::cout << "出发时间（HH:MM，例如 08:30）: ";
        std::string depart_time_str;
        std::cin >> depart_time_str;
        new_trip.departure_time_ = timeStrToMinutes(depart_time_str);
        std::cout << "到达时间（HH:MM，例如 12:45）: ";
        std::string arrival_time_str;
        std::cin >> arrival_time_str;
        new_trip.arrival_time_ = timeStrToMinutes(arrival_time_str);
        std::cout << "票价: ";
        std::cin >> new_trip.price_;
        // 后续需要加入输入验证等功能，当前示例先假设输入合法
        consultController.addTrip(new_trip);
        consultController.saveData();
        std::cout << "已添加班次: " << new_trip.trip_number_ << std::endl;
    } else if (choice == 2) {
        // 删除班次
        int trip_id;
        std::cout << "请输入要删除的班次编号: ";
        std::cin >> trip_id;
        // 后续需要检查班次是否存在等，当前示例先假设输入合法且可以删除
        consultController.removeTrip(trip_id);
        consultController.saveData();
        std::cout << "已删除班次编号: " << trip_id << std::endl;
    } else {
        std::cout << "无效选择。" << std::endl;
    }
    // 返回主菜单或继续管理班次
    std::cout << "按 0 返回主菜单，按 1 继续管理班次: ";
    std::cin >> choice;
    if (choice == 0) {
        return;  // 返回主菜单
    } else if (choice == 1) {
        handleScheduleManagement();  // 继续管理班次
    } else {
        std::cout << "无效选择，返回主菜单。" << std::endl;
        return;
    }
}

// 路径详情展示
void Menu::showPathDetails(const Path& path) {
    clearScreen();
    std::cout << "===== 路径详情 =====" << std::endl;
    for (const auto& segment : path.segments_) {
        std::cout << "班次号: " << segment.trip_.trip_number_ << ", 类型: " << segment.trip_.type_ << ", "
                  << "出发城市: " << segment.trip_.from_city_id_ << " → 到达城市: " << segment.trip_.to_city_id_
                  << std::endl;
        std::cout << "出发时间: " << minutesToTimeStr(segment.trip_.departure_time_)
                  << ", 到达时间: " << minutesToTimeStr(segment.trip_.arrival_time_) << std::endl;
        std::cout << "等待时间: " << (segment.wait_time_ == 0 ? "-" : std::to_string(segment.wait_time_) + "分钟")
                  << ", 本段票价: " << segment.trip_.price_ << "元" << std::endl;
        std::cout << "-----------------------------" << std::endl;
    }
    std::cout << "总结: 总耗时: " << path.total_time_ << "分钟, 总票价: " << path.total_price_
              << "元, 换乘次数: " << path.transfer_count_ << std::endl;
}

// 清屏
void Menu::clearScreen() {
#ifdef _WIN32  // Windows 系统
    system("cls");
#else  // Unix/Linux/MacOS 系统
    system("clear");
#endif
}