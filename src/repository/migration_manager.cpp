#include "migration_manager.h"
#include <QSqlQuery>
#include <QFile>
#include <QSqlError>
#include <stdexcept>

void MigrationManager::migrate(QSqlDatabase& db)
{
    if (db.tables().contains("schema_version"))
        return;

    QFile file(":/migrations/001-init.sql");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        throw std::runtime_error("Cannot open 001-init.sql");

    QString sql = file.readAll();
    file.close();

    QSqlQuery query(db);

    for (const QString &statement : sql.split(';', Qt::SkipEmptyParts)) {
        QString trimmed = statement.trimmed();
        if (trimmed.isEmpty())
            continue;

        if (!query.exec(trimmed)) {
            throw std::runtime_error(
                query.lastError().text().toStdString()
                );
        }
    }
}
