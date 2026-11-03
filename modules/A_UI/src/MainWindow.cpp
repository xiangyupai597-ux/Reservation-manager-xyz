// 成员 A：主窗口、角色页面组装、刷新和退出登录。
#include "A/MainWindow.h"
#include "A/DemoData.h"
#include "A/SeatPage.h"
#include "B/ReservationPage.h"
#include "B/ReservationRules.h"
#include "C/AttendanceViolationPage.h"
#include "D/JsonStore.h"
#include "D/StatisticsPage.h"

#include <QtWidgets>

MainWindow::MainWindow(const User &user, JsonStore &store)
    : seats(store), reservations(store), attendance(store),
      statistics(store)
{
    setWindowTitle(QStringLiteral("校园自习室预约管理系统"));
    resize(1060, 720);
    auto *central = new QWidget;
    setCentralWidget(central);
    auto *root = new QVBoxLayout(central);
    auto *header = new QHBoxLayout;
    const bool admin = user.role == UserRole::Admin;
    header->addWidget(new QLabel(
        QStringLiteral("%1（%2） - %3")
            .arg(user.name, user.account,
                 admin ? QStringLiteral("管理员")
                       : QStringLiteral("学生"))));
    header->addStretch();
    auto *clock = new QLabel;
    header->addWidget(clock);
    auto *refreshButton = new QPushButton(QStringLiteral("刷新"));
    auto *logoutButton = new QPushButton(QStringLiteral("退出登录"));
    header->addWidget(refreshButton);
    header->addWidget(logoutButton);
    root->addLayout(header);
    auto *automaticMessage = new QLabel;
    automaticMessage->setWordWrap(true);
    root->addWidget(automaticMessage);
    auto *tabs = new QTabWidget;
    root->addWidget(tabs);

    const QVector<User> knownUsers = DemoData::users();
    auto *seatPage = new SeatPage(seats);
    auto *reservationPage =
        new ReservationPage(reservations, store, user, knownUsers);
    auto *attendancePage =
        new AttendanceViolationPage(attendance, store, user);
    auto *statisticsPage =
        admin ? new StatisticsPage(statistics, user) : nullptr;
    ReservationPage *historyReservations = nullptr;
    if (admin) {
        tabs->addTab(seatPage, QStringLiteral("座位查询"));
        tabs->addTab(reservationPage,
                     QStringLiteral("预约与签到查询"));
        tabs->addTab(attendancePage, QStringLiteral("违约管理"));
        tabs->addTab(statisticsPage, QStringLiteral("统计"));
    } else {
        auto *bookingPage = new QWidget;
        auto *bookingLayout = new QVBoxLayout(bookingPage);
        bookingLayout->addWidget(seatPage, 1);
        auto *actions = new QHBoxLayout;
        actions->addWidget(reservationPage, 1);
        actions->addWidget(attendancePage, 1);
        bookingLayout->addLayout(actions);
        tabs->addTab(bookingPage, QStringLiteral("座位预约"));

        auto *historyPage = new QWidget;
        auto *historyLayout = new QVBoxLayout(historyPage);
        historyReservations = new ReservationPage(
            reservations, store, user, knownUsers, historyPage, true);
        historyLayout->addWidget(new QLabel(
            QStringLiteral("预约、签到与违约记录")));
        historyLayout->addWidget(historyReservations, 1);
        tabs->addTab(historyPage, QStringLiteral("预约记录"));
    }
    connect(seatPage, &SeatPage::selectionChanged,
            reservationPage, &ReservationPage::setSelection);

    auto refresh = [=] {
        if (refreshing)
            return;
        refreshing = true;
        const QDateTime now =
            toMinute(QDateTime::currentDateTime());
        const OperationResult result =
            attendance.recordExpiredReservations(now);
        automaticMessage->setText(
            result.ok
            ? QString()
            : QStringLiteral("自动登记违约失败：%1")
                  .arg(result.message));
        automaticMessage->setVisible(!result.ok);
        clock->setText(
            QStringLiteral("当前时间：%1").arg(displayTime(now)));
        seatPage->refresh(now);
        reservationPage->refresh();
        attendancePage->refresh();
        if (historyReservations)
            historyReservations->refresh();
        if (statisticsPage)
            statisticsPage->refreshStatistics();
        refreshing = false;
    };
    connect(&store, &JsonStore::dataChanged, this, refresh);
    connect(refreshButton, &QPushButton::clicked, this, refresh);
    connect(logoutButton, &QPushButton::clicked, this, [this] {
        loggedOut = true;
        close();
    });
    auto *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, refresh);
    timer->start(30000);
    refresh();
}
