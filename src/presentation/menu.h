#pragma once

#include "application/consult_controller.h"
#include <iostream>

class Menu {
public:
    Menu(ConsultController& consultController);
    // 主菜单循环
    void showMainMenu();
    // 处理用户选择
    void handleTrafficConsultation();
    // 处理城市管理
    void handleCityManagement();
    // 处理班次管理
    void handleScheduleManagement();
    // 路径详情展示
    void showPathDetails(const Path& path);

    // 辅助函数：清屏
    static void clearScreen();
private:
    ConsultController& consultController;   // 引用控制器，进行业务逻辑交互
};