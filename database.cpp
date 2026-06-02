#include "database.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QSet>
#include <QVariant>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace {
const char kConnectionName[] = "server_database_connection";

QString projectDirectory()
{
    const QStringList candidates = {
        QDir::currentPath(),
        QCoreApplication::applicationDirPath()
    };

    for (const QString &candidate : candidates) {
        QDir directory(candidate);
        do {
            if (directory.exists("echoServer.pro")) {
                return directory.absolutePath();
            }
        } while (directory.cdUp());
    }

    return QDir::currentPath();
}

QString sqliteCompatiblePath(const QString &path)
{
#ifdef Q_OS_WIN
    const QFileInfo fileInfo(path);
    const QString nativeDirectory =
        QDir::toNativeSeparators(fileInfo.absolutePath());

    const DWORD size = GetShortPathNameW(
        reinterpret_cast<LPCWSTR>(nativeDirectory.utf16()), nullptr, 0);
    if (size > 0) {
        std::wstring shortDirectory(size, L'\0');
        const DWORD written = GetShortPathNameW(
            reinterpret_cast<LPCWSTR>(nativeDirectory.utf16()),
            shortDirectory.data(),
            size);
        if (written > 0 && written < size) {
            return QDir(QString::fromWCharArray(shortDirectory.data(), written))
                .filePath(fileInfo.fileName());
        }
    }
#endif

    return path;
}
}

Database::Database()
{
}

Database::~Database()
{
    if (!mDatabase.isValid()) {
        return;
    }

    const QString connectionName = mDatabase.connectionName();
    mDatabase.close();
    mDatabase = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName);
}

Database &Database::instance()
{
    static Database database;
    return database;
}

bool Database::open()
{
    mLastError.clear();

    if (mDatabase.isOpen()) {
        return true;
    }

    if (QSqlDatabase::contains(kConnectionName)) {
        mDatabase = QSqlDatabase::database(kConnectionName);
    } else {
        mDatabase = QSqlDatabase::addDatabase("QSQLITE", kConnectionName);
    }

    QString databasePath = qEnvironmentVariable("SERVER_DATABASE_PATH");
    if (databasePath.isEmpty()) {
        mDisplayDatabasePath = QDir(projectDirectory())
                                   .absoluteFilePath("server_database.db");
        databasePath = sqliteCompatiblePath(mDisplayDatabasePath);
    } else {
        mDisplayDatabasePath = databasePath;
    }

    mDatabase.setDatabaseName(databasePath);

    if (!mDatabase.open()) {
        mLastError = mDatabase.lastError().text();
        qDebug() << "Database error:" << mLastError;
        return false;
    }

    if (!ensureSchema()) {
        mDatabase.close();
        return false;
    }

    qDebug() << "Database connected:" << databasePath;
    return true;
}

bool Database::ensureSchema()
{
    QSqlQuery query(mDatabase);
    if (!query.exec("CREATE TABLE IF NOT EXISTS requests ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "created_at TEXT NOT NULL DEFAULT '', "
                    "request TEXT NOT NULL, "
                    "response TEXT NOT NULL DEFAULT '')")) {
        mLastError = query.lastError().text();
        qDebug() << "Schema error:" << mLastError;
        return false;
    }

    QSet<QString> columns;
    if (!query.exec("PRAGMA table_info(requests)")) {
        mLastError = query.lastError().text();
        qDebug() << "Schema inspection error:" << mLastError;
        return false;
    }

    while (query.next()) {
        columns.insert(query.value("name").toString());
    }

    if (!columns.contains("created_at")
        && !query.exec("ALTER TABLE requests "
                       "ADD COLUMN created_at TEXT NOT NULL DEFAULT ''")) {
        mLastError = query.lastError().text();
        qDebug() << "Schema migration error:" << mLastError;
        return false;
    }

    if (!columns.contains("response")
        && !query.exec("ALTER TABLE requests "
                       "ADD COLUMN response TEXT NOT NULL DEFAULT ''")) {
        mLastError = query.lastError().text();
        qDebug() << "Schema migration error:" << mLastError;
        return false;
    }

    return true;
}

bool Database::saveRequest(const QString &request, const QString &response)
{
    if (!isOpen()) {
        qDebug() << "Save error: database is not open";
        return false;
    }

    QSqlQuery query(mDatabase);
    query.prepare("INSERT INTO requests (created_at, request, response) "
                  "VALUES (:created_at, :request, :response)");
    query.bindValue(":created_at",
                    QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    query.bindValue(":request", request);
    query.bindValue(":response", response);

    if (!query.exec()) {
        qDebug() << "Save error:" << query.lastError().text();
        return false;
    }

    qDebug() << "Request saved:" << request;
    return true;
}

QString Database::getAllRequests() const
{
    if (!isOpen()) {
        return mLastError.isEmpty()
                   ? "Error: database is not open"
                   : "Error: database is not open: " + mLastError;
    }

    QString result;
    QSqlQuery query(mDatabase);

    if (!query.exec("SELECT id, created_at, request, response "
                    "FROM requests ORDER BY id")) {
        return "Error: cannot read database: " + query.lastError().text();
    }

    while (query.next()) {
        QString request = query.value("request").toString();
        QString response = query.value("response").toString();
        request.replace('\n', "\\n");
        response.replace('\n', "\\n");

        result += QString("%1 | %2 | request=%3 | response=%4\n")
                      .arg(query.value("id").toString(),
                           query.value("created_at").toString(),
                           request,
                           response);
    }

    if (result.isEmpty()) {
        result = "Database is empty\n";
    }

    return result;
}

QString Database::databasePath() const
{
    return mDisplayDatabasePath;
}

bool Database::isOpen() const
{
    return mDatabase.isValid() && mDatabase.isOpen();
}
