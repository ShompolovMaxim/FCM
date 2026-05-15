#include "migration_manager.h"

#include <QDir>
#include <QFile>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QRegularExpression>

namespace {
bool migrationVersion(const QString &fileName, int &version) {
    static const QRegularExpression re(R"(^(\d+).+\.sql$)");
    const auto match = re.match(fileName);
    if (!match.hasMatch()) {
        qDebug() << "Invalid migration file name:" << fileName;
        return false;
    }

    version = match.captured(1).toInt();
    return true;
}

bool currentSchemaVersion(QSqlDatabase &db, int &version) {
    if (!db.tables().contains("schema_version")) {
        version = 0;
        return true;
    }

    QSqlQuery query(db);
    if (!query.exec("SELECT version FROM schema_version LIMIT 1")) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return false;
    }

    if (!query.next()) {
        version = 0;
        return true;
    }

    version = query.value(0).toInt();
    return true;
}

bool setSchemaVersion(QSqlDatabase &db, int version) {
    QSqlQuery query(db);
    if (!query.exec("DELETE FROM schema_version")) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return false;
    }

    query.prepare("INSERT INTO schema_version (version) VALUES (:version)");
    query.bindValue(":version", version);
    if (!query.exec()) {
        qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << query.lastQuery();
        return false;
    }

    return true;
}

bool executeMigrationFile(QSqlDatabase &db, const QString &resourcePath) {
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Cannot open migration file:" << resourcePath;
        return false;
    }

    const QString sql = file.readAll();
    file.close();

    QSqlQuery query(db);
    for (const QString &statement : sql.split(';', Qt::SkipEmptyParts)) {
        const QString trimmed = statement.trimmed();
        if (trimmed.isEmpty()) {
            continue;
        }

        if (!query.exec(trimmed)) {
            qDebug() << "SQL Error:" << query.lastError().text() << "Query:" << trimmed;
            return false;
        }
    }

    return true;
}
}

bool MigrationManager::migrate(QSqlDatabase& db) {
    QDir migrationsDir(":/migrations");
    const QStringList migrationFiles = migrationsDir.entryList({"*.sql"}, QDir::Files, QDir::Name);

    int appliedVersion = 0;
    if (!currentSchemaVersion(db, appliedVersion)) {
        return false;
    }

    for (const QString &migrationFile : migrationFiles) {
        int fileVersion = 0;
        if (!migrationVersion(migrationFile, fileVersion)) {
            return false;
        }
        if (fileVersion <= appliedVersion) {
            continue;
        }

        if (!db.transaction()) {
            qDebug() << "Failed to start migration transaction";
            return false;
        }

        if (!executeMigrationFile(db, ":/migrations/" + migrationFile) ||
            !setSchemaVersion(db, fileVersion)) {
            db.rollback();
            return false;
        }

        if (!db.commit()) {
            qDebug() << "SQL Error:" << db.lastError().text();
            db.rollback();
            return false;
        }

        appliedVersion = fileVersion;
    }

    return true;
}
