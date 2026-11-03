# 成员 C：签到与违约模块接口说明

## 模块职责

- 学生签到。
- 管理员登记违约。
- 管理员撤销违约。
- 到期未签到自动登记违约。
- 学生查询本人违约，管理员查询全部违约。

## 依赖

```text
shared/Models.h
B/ReservationRules.h
D/JsonStore.h
MemberB_Reservation.lib/.dll
MemberD_Data.lib/.dll
```

## 公开接口

```cpp
class MEMBER_C_API AttendanceViolationManager {
public:
    explicit AttendanceViolationManager(JsonStore &store);
    OperationResult checkIn(
        const User &user, const QDateTime &now);
    OperationResult checkIn(
        const User &user, const QString &reservationId,
        const QDateTime &now);
    OperationResult addViolation(
        const User &admin, const QString &reservationId,
        const QDateTime &now);
    OperationResult revokeViolation(
        const User &admin, const QString &violationId);
    OperationResult recordExpiredReservations(
        const QDateTime &now);
    QVector<Violation> listViolations(
        const User &user) const;
};
```

页面类：

```cpp
AttendanceViolationPage(
    AttendanceViolationManager &manager,
    const JsonStore &store,
    const User &user,
    QWidget *parent = nullptr);
```

## 构建输出

```text
bin/x64/Debug/MemberC_Attendance.dll
bin/x64/Debug/MemberC_Attendance.lib
bin/x64/Release/MemberC_Attendance.dll
bin/x64/Release/MemberC_Attendance.lib
```

## 交付规则

A 只接收：

```text
include/C/*.h
MemberC_Attendance.lib
MemberC_Attendance.dll
```

不接收 `src/*.cpp`。
