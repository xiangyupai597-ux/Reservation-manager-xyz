#pragma once

// 成员 D：JSON 持久化，是其他业务模块共同依赖的底层模块。
#include "Models.h"
#include "D/Export.h"

#include <QObject>

class MEMBER_D_API JsonStore : public QObject
{
    Q_OBJECT

public:
    explicit JsonStore(QObject *parent = nullptr);

    OperationResult load();
    const AppData &data() const;
    OperationResult commit(const AppData &updatedData);

signals:
    void dataChanged();

private:
    AppData m_data;
};
