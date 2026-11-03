#include "B/ReservationManager.h"
#include "B/ReservationRules.h"
#include "C/AttendanceViolationManager.h"
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

const Reservation *findReservation(
    const AppData &data, const QString &reservationId)
{
    for (const Reservation &reservation : data.reservations) {
        if (reservation.id == reservationId)
            return &reservation;
    }
    return nullptr;
}

const Violation *findViolation(
    const AppData &data, const QString &reservationId)
{
    for (const Violation &violation : data.violations) {
        if (violation.reservationId == reservationId)
            return &violation;
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

    ReservationManager reservations(store);
    AttendanceViolationManager attendance(store);

    const QDate date = QDate::currentDate();
    const QDateTime dayStart(date, QTime(10, 0));

    const OperationResult first = reservations.createReservation(
        student1, 1, dayStart, dayStart.addSecs(30 * 60), dayStart);
    expect(first.ok, "create reservation for check-in");
    const QString firstId =
        reservations.listReservations(student1).value(0).id;

    const OperationResult earlyCheckIn = attendance.checkIn(
        student1, firstId, dayStart.addSecs(-60));
    expect(!earlyCheckIn.ok, "reject check-in before reservation start");

    const OperationResult validCheckIn = attendance.checkIn(
        student1, firstId, dayStart.addSecs(10 * 60));
    expect(validCheckIn.ok, "allow check-in inside reservation period");

    const OperationResult duplicateCheckIn = attendance.checkIn(
        student1, firstId, dayStart.addSecs(20 * 60));
    expect(!duplicateCheckIn.ok, "reject duplicate check-in");

    const QDateTime secondStart(date, QTime(11, 0));
    const OperationResult second = reservations.createReservation(
        student2, 2, secondStart, secondStart.addSecs(30 * 60), dayStart);
    expect(second.ok, "create reservation for manual violation");
    const QString secondId =
        reservations.listReservations(student2).value(0).id;

    const OperationResult violationTooEarly = attendance.addViolation(
        admin, secondId, secondStart.addSecs(-60));
    expect(!violationTooEarly.ok,
           "reject violation before reservation start");

    const OperationResult studentViolation = attendance.addViolation(
        student1, secondId, secondStart);
    expect(!studentViolation.ok,
           "reject violation registration by student");

    const OperationResult manualViolation = attendance.addViolation(
        admin, secondId, secondStart);
    expect(manualViolation.ok, "administrator can register violation");

    const Reservation *terminated =
        findReservation(store.data(), secondId);
    expect(terminated != nullptr
               && terminated->flag == ReservationFlag::Terminated,
           "manual violation terminates active reservation");
    expect(terminated != nullptr && terminated->closedAt.isValid(),
           "terminated reservation records close time");

    const OperationResult duplicateViolation = attendance.addViolation(
        admin, secondId, secondStart.addSecs(5 * 60));
    expect(!duplicateViolation.ok,
           "reject duplicate active violation");

    const Violation *secondViolation =
        findViolation(store.data(), secondId);
    expect(secondViolation != nullptr,
           "violation is stored for reservation");
    const QString secondViolationId =
        secondViolation == nullptr ? QString() : secondViolation->id;

    const OperationResult studentRevoke = attendance.revokeViolation(
        student1, secondViolationId);
    expect(!studentRevoke.ok, "reject violation revoke by student");

    const OperationResult missingRevoke =
        attendance.revokeViolation(admin, QStringLiteral("missing"));
    expect(!missingRevoke.ok, "reject missing violation revoke");

    const OperationResult revoke = attendance.revokeViolation(
        admin, secondViolationId);
    expect(revoke.ok, "administrator can revoke violation");

    const OperationResult duplicateRevoke = attendance.revokeViolation(
        admin, secondViolationId);
    expect(!duplicateRevoke.ok, "reject duplicate violation revoke");

    terminated = findReservation(store.data(), secondId);
    expect(terminated != nullptr
               && terminated->flag == ReservationFlag::Terminated,
           "revoke does not restore terminated reservation");

    const QDateTime autoStart(date, QTime(13, 0));
    const OperationResult autoCandidate = reservations.createReservation(
        student1, 3, autoStart, autoStart.addSecs(30 * 60), dayStart);
    expect(autoCandidate.ok, "create reservation for automatic violation");
    const QString autoId =
        store.data().reservations.constLast().id;

    const OperationResult notExpired = attendance.recordExpiredReservations(
        autoStart.addSecs(29 * 60));
    expect(notExpired.ok, "auto violation at end boundary is not early");
    expect(findViolation(store.data(), autoId) == nullptr,
           "reservation is not violated before end time");

    const OperationResult expired = attendance.recordExpiredReservations(
        autoStart.addSecs(30 * 60));
    expect(expired.ok, "record expired missed check-in");
    expect(findViolation(store.data(), autoId) != nullptr,
           "expired reservation receives violation");

    const int countAfterAuto = store.data().violations.size();
    const OperationResult repeatedAuto =
        attendance.recordExpiredReservations(
            autoStart.addSecs(60 * 60));
    expect(repeatedAuto.ok
               && store.data().violations.size() == countAfterAuto,
           "automatic violation is not duplicated");

    const QDateTime finalStart(date, QTime(15, 0));
    const OperationResult finalCandidate = reservations.createReservation(
        student2, 4, finalStart, finalStart.addSecs(30 * 60), dayStart);
    expect(finalCandidate.ok, "create second manual violation candidate");
    const QString finalId = store.data().reservations.constLast().id;

    const OperationResult finalViolation = attendance.addViolation(
        admin, finalId, finalStart);
    expect(finalViolation.ok, "register second manual violation");

    const QVector<Violation> student1Violations =
        attendance.listViolations(student1);
    expect(student1Violations.size() == 1,
           "student only queries own violation");

    const QVector<Violation> student2Violations =
        attendance.listViolations(student2);
    expect(student2Violations.size() == 2,
           "student queries own active and revoked violations");

    expect(attendance.listViolations(admin).size() == 3,
           "administrator queries all violations");

    const int finalCount = store.data().violations.size();
    expect(attendance.recordExpiredReservations(
               QDateTime(date, QTime(16, 0))).ok
               && store.data().violations.size() == finalCount,
           "refresh does not duplicate existing violations");

    std::cout << "MemberC_Test failures: " << failures << '\n';
    return failures == 0 ? 0 : 1;
}
