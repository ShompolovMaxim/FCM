#pragma once
#include <QSqlDatabase>

class MigrationManager {
public:
    static bool migrate(QSqlDatabase& db);
};
