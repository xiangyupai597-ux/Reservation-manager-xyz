#pragma once

// 成员 B：供 SeatManager 和 AttendanceViolationManager 共同使用的时间及状态规则。
#include "Models.h"
#include "B/Export.h"

#include <QDateTime>
#include <QString>
#include <QVector>

MEMBER_B_API QDateTime toMinute(const QDateTime &value);
MEMBER_B_API bool isHalfHourAligned(const QDateTime &value);
MEMBER_B_API bool overlaps(const QDateTime &startA, const QDateTime &endA,
                           const QDateTime &startB, const QDateTime &endB);
MEMBER_B_API ReservationViewState stateOf(const Reservation &reservation,
                                          const QDateTime &now);
MEMBER_B_API QString displayTime(const QDateTime &value);
MEMBER_B_API QString reservationStateText(
    const Reservation &reservation, const QDateTime &now,
    const QVector<Violation> &violations = {});
