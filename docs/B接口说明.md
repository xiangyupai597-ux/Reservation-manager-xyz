# 成员 B：预约模块接口说明

## 模块职责

- `ReservationManager`：创建、取消和查询预约。
- `ReservationPage`：学生预约、我的预约和管理员预约查询页面。
- `ReservationRules`：公共时间及预约状态规则。

## 依赖

```text
shared/Models.h
shared/SystemConfig.h
D/JsonStore.h
MemberD_Data.lib
MemberD_Data.dll
```

## 公开接口

### ReservationManager

```cpp
class MEMBER_B_API ReservationManager {
public:
    explicit ReservationManager(JsonStore &store);
    OperationResult createReservation(
        const User &user, int seatId,
        const QDateTime &start, const QDateTime &end,
        const QDateTime &now);
    OperationResult cancelReservation(
        const User &user, const QString &reservationId,
        const QDateTime &now);
    QVector<Reservation> listReservations(const User &user) const;
};
```

### 公共规则

```cpp
MEMBER_B_API QDateTime toMinute(const QDateTime &value);
MEMBER_B_API bool isHalfHourAligned(const QDateTime &value);
MEMBER_B_API bool overlaps(
    const QDateTime &startA, const QDateTime &endA,
    const QDateTime &startB, const QDateTime &endB);
MEMBER_B_API ReservationViewState stateOf(
    const Reservation &reservation, const QDateTime &now);
MEMBER_B_API QString displayTime(const QDateTime &value);
MEMBER_B_API QString reservationStateText(
    const Reservation &reservation, const QDateTime &now,
    const QVector<Violation> &violations = {});
```

### ReservationPage

```cpp
ReservationPage(
    ReservationManager &manager,
    const JsonStore &store,
    const User &user,
    const QVector<User> &knownUsers,
    QWidget *parent = nullptr,
    bool historyOnly = false);
```

## 构建输出

```text
bin/x64/Debug/MemberB_Reservation.dll
bin/x64/Debug/MemberB_Reservation.lib
bin/x64/Release/MemberB_Reservation.dll
bin/x64/Release/MemberB_Reservation.lib
```

## 交付规则

A/C 只接收：

```text
include/B/*.h
MemberB_Reservation.lib
MemberB_Reservation.dll
```

不接收 `src/*.cpp`。
