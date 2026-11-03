# 成员 A 测试报告

测试程序：`tests/MemberA_Test`

环境：

```text
Visual Studio 2022
MSVC v143
x64
C++17
Qt 6.11.2 MSVC2022 64 位
```

## 运行方式

打开 `StudyRoomReservation.sln`，选择 `Debug | x64`：

- 运行 `MemberA_Test` 验证座位逻辑。
- 将 `A_UI` 设为启动项目并运行完整 GUI。

## 测试结果

| 编号 | 测试内容 | 预期 | 实际 | 结果 |
| --- | --- | --- | --- | --- |
| A01 | 加载空数据 | 成功 | 成功 | PASS |
| A02 | 三个演示账号 | 1 管理员 + 2 学生 | 符合预期 | PASS |
| A03 | 固定 12 个座位 | 编号 1-12 且名称唯一 | 符合预期 | PASS |
| A04 | 3x4 布局和开放时间 | 配置正确 | 符合预期 | PASS |
| A05 | SeatManager 固定座位查询 | 12 个座位 | 12 个座位 | PASS |
| A06 | 非半小时查询 | 返回空集合 | 返回空集合 | PASS |
| A07 | 已预约座位当前状态 | 已预约 | 已预约 | PASS |
| A08 | 所选时段占用 | 不可预约 | 不可预约 | PASS |
| A09 | 其他空闲座位 | 空闲且可预约 | 符合预期 | PASS |
| A10 | 签到后状态 | 使用中 | 使用中 | PASS |
| A11 | 当前状态与未来时段分离 | 当前空闲、所选占用 | 符合预期 | PASS |
| A12 | 取消释放座位 | 空闲且可预约 | 符合预期 | PASS |

## GUI 验收项

- 正确账号可登录，错误密码被拒绝。
- 管理员和学生看到的页面不同。
- 3x4 座位网格正常显示。
- 退出登录后返回登录窗口。
- 30 秒刷新和手动刷新正常。

## 结论

已使用 MSVC 19.44 手工编译并链接 D DLL、B DLL、C DLL、A 主程序和 `MemberA_Test`。

- `MemberA_Test` 实际运行 22 项断言，全部通过，失败数为 0。
- `StudyRoomReservation.exe` 使用 `QT_QPA_PLATFORM=offscreen` 启动冒烟测试成功，进程稳定运行后由测试脚本关闭。
- `dumpbin /dependents` 确认 EXE 依赖 `MemberB_Reservation.dll`、`MemberC_Attendance.dll` 和 `MemberD_Data.dll`。

尚未完成真实窗口下的完整登录、角色切换、三账号连续登录和人工交互验收。正式交付前应在正常 Qt MSBuild 环境中分别运行 Debug、Release GUI。
