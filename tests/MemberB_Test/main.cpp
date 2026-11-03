#include "B/ReservationManager.h"
#include "B/ReservationRules.h"
#include "D/JsonStore.h"

#include <QCoreApplication>
#include <QDate>
#include <QFile>
#include <QTime>

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

    const User student1{
        QStringLiteral("student01"), QStringLiteral("Student One"),
        QStringLiteral("123456"), UserRole::Student
    };
    const User student2{
        QStringLiteral("student02"), QStringLiteral("Student Two"),
        QStringLiteral("123456"), UserRole::Student
    };
    const User admin{
        QStringLiteral("admin"), QStringLiteral("Administrator"),
        QStringLiteral("123456"), UserRole::Admin
    };

    ReservationManager manager(store);
    const QDateTime now(QDate::currentDate(), QTime(10, 0));
    const QDateTime start = now;
    const QDateTime end = now.addSecs(30 * 60);

    const OperationResult first =
        manager.createReservation(student1, 1, start, end, now);
    expect(first.ok, "create normal reservation");

    QVector<Reservation> studentReservations =
        manager.listReservations(student1);
    expect(studentReservations.size() == 1,
           "student can query own reservation");
    const QString firstId = studentReservations.isEmpty()
        ? QString() : studentReservations.first().id;

    const OperationResult seatConflict =
        manager.createReservation(student2, 1, start, end, now);
    expect(!seatConflict.ok, "reject seat conflict");

    const OperationResult studentConflict =
        manager.createReservation(student1, 2, start, end, now);
    expect(!studentConflict.ok, "reject same-student time conflict");

    const OperationResult adjacent = manager.createReservation(
        student1, 2, end, end.addSecs(30 * 60), now);
    expect(adjacent.ok, "allow adjacent reservation");

    const OperationResult invalidMinute = manager.createReservation(
        student2, 3, now.addSecs(15 * 60), now.addSecs(45 * 60), now);
    expect(!invalidMinute.ok, "reject non-half-hour time");

    const OperationResult past = manager.createReservation(
        student2, 3, now.addSecs(-60 * 60), now.addSecs(-30 * 60), now);
    expect(!past.ok, "reject past reservation");

    const OperationResult crossDay = manager.createReservation(
        student2, 3, now.addDays(1), now.addDays(1).addSecs(30 * 60), now);
    expect(!crossDay.ok, "reject cross-day reservation");

    const OperationResult wrongOwner =
        manager.cancelReservation(student2, firstId, now);
    expect(!wrongOwner.ok, "reject cancellation by another student");

    const OperationResult adminCancel =
        manager.cancelReservation(admin, firstId, now);
    expect(!adminCancel.ok, "reject cancellation by administrator");

    const OperationResult cancelOwn =
        manager.cancelReservation(student1, firstId, now);
    expect(cancelOwn.ok, "student can cancel own reservation");

    studentReservations = manager.listReservations(student1);
    expect(studentReservations.size() == 2,
           "cancelled reservation remains in history");

    const OperationResult studentQuery =
        manager.createReservation(student2, 4, now, end, now);
    expect(studentQuery.ok, "create reservation for student two");

    expect(manager.listReservations(admin).size() == 3,
           "administrator can query all reservations");

    const OperationResult reuseSeat =
        manager.createReservation(student1, 1, start, end, now);
    expect(reuseSeat.ok, "cancelled reservation releases seat");

    const QDateTime normalized = toMinute(
        QDateTime(QDate::currentDate(), QTime(10, 0, 45, 123)));
    expect(normalized.time() == QTime(10, 0),
           "toMinute removes seconds and milliseconds");

    std::cout << "MemberB_Test failures: " << failures << '\n';
    return failures == 0 ? 0 : 1;
}
