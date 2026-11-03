#pragma once

// 成员 A：主窗口、角色页面组装、刷新和退出登录。
#include "Models.h"
#include "A/SeatManager.h"
#include "B/ReservationManager.h"
#include "C/AttendanceViolationManager.h"
#include "D/StatisticsManager.h"

#include <QMainWindow>

class JsonStore;

class MainWindow : public QMainWindow
{
public:
    MainWindow(const User &user, JsonStore &store);

    bool loggedOut = false;

private:
    bool refreshing = false;
    SeatManager seats;
    ReservationManager reservations;
    AttendanceViolationManager attendance;
    StatisticsManager statistics;
};
