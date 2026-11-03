# 成员 B 测试报告

测试程序：`tests/MemberB_Test`

环境：

```text
Visual Studio 2022
MSVC v143
x64
C++17
Qt 6.11.2 MSVC2022 64 位
```

## 运行方式

在 Visual Studio 中打开 `B_DataReservation.sln`，选择 `Debug | x64`，将 `MemberB_Test` 设为启动项目并运行。

## 测试结果

| 编号 | 测试内容 | 预期 | 实际 | 结果 |
| --- | --- | --- | --- | --- |
| B01 | 正常创建预约 | 成功 | 成功 | PASS |
| B02 | 学生查询本人预约 | 只能得到本人记录 | 符合预期 | PASS |
| B03 | 同一座位时间冲突 | 拒绝 | 已拒绝 | PASS |
| B04 | 同一学生时间冲突 | 拒绝 | 已拒绝 | PASS |
| B05 | 首尾相接预约 | 允许 | 已允许 | PASS |
| B06 | 非半小时时间 | 拒绝 | 已拒绝 | PASS |
| B07 | 过去时间 | 拒绝 | 已拒绝 | PASS |
| B08 | 跨日预约 | 拒绝 | 已拒绝 | PASS |
| B09 | 他人取消预约 | 拒绝 | 已拒绝 | PASS |
| B10 | 管理员取消预约 | 拒绝 | 已拒绝 | PASS |
| B11 | 本人取消预约 | 成功 | 成功 | PASS |
| B12 | 取消后保留历史 | 记录仍存在 | 符合预期 | PASS |
| B13 | 管理员查询全部 | 返回全部记录 | 符合预期 | PASS |
| B14 | 取消后释放座位 | 新预约成功 | 成功 | PASS |
| B15 | 时间归一化 | 去掉秒和毫秒 | 符合预期 | PASS |

## 失败记录

暂无。

## 结论

已使用 MSVC 19.44 手工编译并链接 D DLL、B DLL 和测试程序，实际运行 17 项检查，全部通过，失败数为 0。

当前验证不包含 Qt VS Tools qmake 整体构建和 A/C 主程序集成；正式交付前仍需在正常的 Qt MSBuild 环境中运行 `B_DataReservation.sln` 并分别验证 Debug、Release。
