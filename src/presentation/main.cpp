#include <iostream>

#include "application/consult_controller.h"
#include "presentation/menu.h"

int main() {
        // 初始化控制器并加载数据
    ConsultController controller;
    controller.loadData();
    // 显示加载的数据量
    std::cout << "Loaded " << controller.getData().getAllCities().size() << " cities, "
              << controller.getData().getAllTrips().size() << " trips\n";
    // 启动菜单
    Menu menu(controller);
    // 显示主菜单
    menu.showMainMenu();

    // 退出前保存数据
    controller.saveData();
    return 0;
}
