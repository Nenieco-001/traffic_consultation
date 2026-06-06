#include <iostream>

#include "application/auth_service.h"
#include "application/consult_controller.h"
#include "presentation/menu.h"

int main() {
    // 初始化应用层：交通控制器 + 认证服务
    ConsultController controller;
    controller.loadData();  // 从 data/ 加载城市和班次数据

    AuthService authService;
    authService.loadData();  // 从 data/user.dat 加载用户，首次运行自动创建 admin

    std::cout << "Loaded " << controller.getData().getAllCities().size() << " cities, "
              << controller.getData().getAllTrips().size() << " trips, " << "system ready.\n"
              << std::endl;

    // Menu 依赖 ConsultController（查询/CRUD）和 AuthService（登录注册）
    Menu menu(controller, authService);
    menu.showMainMenu();  // 事件循环结束才返回

    // 退出前保存所有变更
    controller.saveData();
    authService.saveData();
    return 0;
}
