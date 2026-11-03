#include "A/DemoData.h"
#include "A/SeatManager.h"
#include "B/ReservationManager.h"
#include "C/AttendanceViolationManager.h"
#include "D/JsonStore.h"
#include "SystemConfig.h"

#include <QCoreApplication>
#include <QDate>
#include <QFile>
#include <QSet>
#include <QTime>

#include <algorithm>
#include <iostream>

namespace {
int failures = 0;

void expect(bool condition, const char *name)
{
    if (condition) {
        std::cout << "[PASS] " << name << '\n';
    } else {
        std::cout << "[FAIL] " << name << '\n';
        ++failures;
    }
}

QString testDataPath()
{
    return QCoreApplication::applicationDirPath()
        + QStringLiteral("/study_room_data.json");
}

const SeatStateView *findSeat(
    const QVector<SeatStateView> &states, int seatId)
{
    for (const SeatStateView &state : states) {
        if (state.seat.id == seatId)
            return &state;
    }
    return nullptr;
}

class DataFileBackup
{
public:
    DataFileBackup()
        : m_path(testDataPath()), m_existed(QFile::exists(m_path))
    {
        if (m_existed) {
            QFile input(m_path);
            if (input.open(QIODevice::ReadOnly))
                m_original = input.readAll();
        }
        QFile::remove(m_path);
    }

    ~DataFileBackup()
    {
        if (m_existed) {
            QFile output(m_path);
            if (output.open(QIODevice::WriteOnly))
                output.write(m_original);
        } else {
            QFile::remove(m_path);
        }
    }

private:
    QString m_path;
    bool m_existed = false;
    QByteArray m_original;
};
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    DataFileBackup backup;

    JsonStore store;
    expect(store.load().ok, "load empty data store");

    const QVector<User> users = DemoData::users();
    expect(users.size() == 3, "provide three demo users");
    expect(std::count_if(users.begin(), users.end(),
                         [](const User &user) {
                             return user.role == UserRole::Admin;
                         }) == 1,
           "provide one administrator");
    expect(std::count_if(users.begin(), users.end(),
                         [](const User &user) {
                             return user.role == UserRole::Student;
                         }) == 2,
           "provide two students");

    const QVector<Seat> seats = DemoData::seats();
    expect(seats.size() == DemoData::seatCount,
           "seat constant matches seat list");
    expect(seats.size() == 12, "provide twelve seats");
    expect(!seats.isEmpty()
               && seats.first().id == 1
               && seats.last().id == 12,
           "seat ids cover one through twelve");

    QSet<QString> seatNames;
    for (const Seat &seat : seats)
        seatNames.insert(seat.name);
    expect(seatNames.size() == seats.size(), "seat names are unique");
    expect(DemoData::seatRows == 3
               && DemoData::seatColumns == 4,
           "seat layout is three by four");
    expect(DemoData::openingTime() == QTime(8, 0)
               && DemoData::closingTime() == QTime(23, 0),
           "opening hours are correct");

    SeatManager seatManager(store);
    expect(seatManager.listSeats().size() == 12,
           "seat manager returns fixed seat list");

    const QDate date = QDate::currentDate();
    const QDateTime now(date, QTime(10, 0));
    expect(seatManager.getSeatStates(
               now.addSecs(15 * 60),
               now.addSecs(45 * 60),
               now).isEmpty(),
           "reject non-half-hour seat query");

    const User student1{
        QStringLiteral("student01"), QStringLiteral("Student One"),
        QStringLiteral("123456"), UserRole::Student
    };
    const User student2{
        QStringLiteral("student02"), QStringLiteral("Student Two"),
        QStringLiteral("123456"), UserRole::Student
    };

    ReservationManager reservations(store);
    AttendanceViolationManager attendance(store);

    const OperationResult first = reservations.createReservation(
        student1, 1, now, now.addSecs(30 * 60), now);
    expect(first.ok, "create reservation for seat state test");
    const QString firstId =
        reservations.listReservations(student1).value(0).id;

    QVector<SeatStateView> states = seatManager.getSeatStates(
        now, now.addSecs(30 * 60), now);
    const SeatStateView *seat1 = findSeat(states, 1);
    const SeatStateView *seat2 = findSeat(states, 2);
    expect(seat1 != nullptr
               && seat1->currentState == SeatCurrentState::Reserved,
           "show reserved seat before check-in");
    expect(seat1 != nullptr
               && !seat1->availableForSelectedPeriod,
           "mark selected occupied seat unavailable");
    expect(seat2 != nullptr
               && seat2->currentState == SeatCurrentState::Free
               && seat2->availableForSelectedPeriod,
           "show another seat free and available");

    expect(attendance.checkIn(
               student1, firstId, now.addSecs(10 * 60)).ok,
           "check in reserved seat");
    states = seatManager.getSeatStates(
        now, now.addSecs(30 * 60), now.addSecs(10 * 60));
    seat1 = findSeat(states, 1);
    expect(seat1 != nullptr
               && seat1->currentState == SeatCurrentState::InUse,
           "show checked-in seat in use");

    const QDateTime futureStart(date, QTime(13, 0));
    expect(reservations.createReservation(
               student2, 3, futureStart,
               futureStart.addSecs(30 * 60), now).ok,
           "create future reservation");
    states = seatManager.getSeatStates(
        futureStart, futureStart.addSecs(30 * 60), now);
    const SeatStateView *seat3 = findSeat(states, 3);
    expect(seat3 != nullptr
               && seat3->currentState == SeatCurrentState::Free
               && !seat3->availableForSelectedPeriod,
           "separate current state from future availability");

    expect(reservations.cancelReservation(
               student1, firstId, now.addSecs(20 * 60)).ok,
           "cancel checked-in reservation before end");
    states = seatManager.getSeatStates(
        now, now.addSecs(30 * 60), now.addSecs(20 * 60));
    seat1 = findSeat(states, 1);
    expect(seat1 != nullptr
               && seat1->currentState == SeatCurrentState::Free
               && seat1->availableForSelectedPeriod,
           "cancelled reservation releases seat");

    std::cout << "MemberA_Test failures: " << failures << '\n';
    return failures == 0 ? 0 : 1;
}
