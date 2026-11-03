# 校园自习室预约管理系统

项目按四人分工拆分为：

- `A_UI`：主程序、登录、主窗口和座位模块。
- `B_Reservation`：预约规则和预约页面。
- `C_Attendance`：签到、违约管理和页面。
- `D_DataStatistics`：JSON 持久化、统计和统计页面。

## 最终解决方案

打开 `StudyRoomReservation.sln`，选择 `Debug | x64` 或 `Release | x64`。

依赖方向：

```text
D <- B <- C
       \  |
        A_UI
```

A 只引用 B/C/D 的公开头文件，不直接复制三个模块的 `.cpp`。

## 独立测试

```text
tests/MemberA_Test
tests/MemberB_Test
tests/MemberC_Test
```

## 运行账号

| 账号 | 密码 | 身份 |
| --- | --- | --- |
| admin | 123456 | 管理员 |
| student01 | 123456 | 学生一 |
| student02 | 123456 | 学生二 |

## 数据文件

程序数据保存在 EXE 同目录的：

```text
study_room_data.json
```
