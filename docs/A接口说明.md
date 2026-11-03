# 成员 A：界面与座位模块接口说明

## 模块职责

- 固定演示账号和 12 个座位。
- 登录窗口和角色切换。
- 学生、管理员页面组装。
- 3x4 座位网格。
- 当前座位状态和所选时段可预约情况。
- 手动刷新、数据变化刷新和 30 秒定时刷新。
- 退出登录及程序入口。

## 依赖

```text
shared/Models.h
shared/SystemConfig.h
B/ReservationManager.h
B/ReservationPage.h
B/ReservationRules.h
C/AttendanceViolationManager.h
C/AttendanceViolationPage.h
D/JsonStore.h
D/StatisticsManager.h
D/StatisticsPage.h
```

## SeatManager 接口

```cpp
class SeatManager {
public:
    explicit SeatManager(const JsonStore &store);
    QVector<Seat> listSeats() const;
    QVector<SeatStateView> getSeatStates(
        const QDateTime &start,
        const QDateTime &end,
        const QDateTime &now) const;
};
```

## 最终程序输出

```text
bin/x64/Debug/StudyRoomReservation.exe
bin/x64/Release/StudyRoomReservation.exe
```

运行时需要同目录包含：

```text
MemberB_Reservation.dll
MemberC_Attendance.dll
MemberD_Data.dll
Qt6Widgetsd.dll / Qt6Cored.dll
platforms/
```

## 主程序集成

A 只包含 B/C/D 的公开头文件，不复制它们的 `.cpp`：

```cpp
#include "B/ReservationManager.h"
#include "C/AttendanceViolationManager.h"
#include "D/JsonStore.h"
```

主窗口通过 `JsonStore::dataChanged` 和定时器统一刷新所有页面。
