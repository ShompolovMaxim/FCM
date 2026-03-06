#pragma once
#include <QSqlDatabase>

class MigrationManager {
public:
    static void migrate(QSqlDatabase& db);
};
