#include "database.h"

#include <QDebug>
#include <QSqlError>
#include <QCryptographicHash>
#include <QSqlRecord>

Database* Database::m_instance = nullptr;
QMutex Database::m_instanceMutex;

Database& Database::instance()
{
    if (!m_instance) {
        QMutexLocker locker(&m_instanceMutex);
        if (!m_instance) {
            m_instance = new Database();
        }
    }
    return *m_instance;
}

Database::~Database()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool Database::open(const QString& path)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_db.isOpen()) {
        return true;
    }
    
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);
    
    if (!m_db.open()) {
        qDebug() << "Database error:" << m_db.lastError().text();
        return false;
    }
    
    createTables();
    qDebug() << "Database opened at" << path;
    return true;
}

void Database::close()
{
    QMutexLocker locker(&m_mutex);
    if (m_db.isOpen()) {
        m_db.close();
    }
}

QString Database::databasePath() const
{
    QMutexLocker locker(&m_mutex);
    return m_db.databaseName();
}

void Database::createTables()
{
    // Таблица пользователей
    QSqlQuery query;
    if (!query.exec("CREATE TABLE IF NOT EXISTS users ("
                    "login VARCHAR(20) PRIMARY KEY, "
                    "password VARCHAR(128) NOT NULL, "
                    "role VARCHAR(20) NOT NULL DEFAULT 'user', "
                    "created_at DATETIME DEFAULT CURRENT_TIMESTAMP)")) {
        qDebug() << "Failed to create users table:" << query.lastError().text();
    }
    
    // Таблица истории запросов
    if (!query.exec("CREATE TABLE IF NOT EXISTS requests ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                    "request TEXT NOT NULL, "
                    "response TEXT NOT NULL, "
                    "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP)")) {
        qDebug() << "Failed to create requests table:" << query.lastError().text();
    }
    
    // Добавляем тестового администратора, если нет пользователей
    query.exec("SELECT COUNT(*) FROM users");
    if (query.next() && query.value(0).toInt() == 0) {
        addUser("admin", "admin123", "admin");
        qDebug() << "Created default admin user: admin / admin123";
    }
}

QString Database::hashPassword(const QString& password)
{
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha512);
    return hash.toHex();
}

bool Database::addUser(const QString& login, const QString& password, const QString& role)
{
    QMutexLocker locker(&m_mutex);
    
    QSqlQuery query;
    query.prepare("INSERT INTO users (login, password, role) VALUES (:login, :password, :role)");
    query.bindValue(":login", login);
    query.bindValue(":password", hashPassword(password));
    query.bindValue(":role", role);
    
    if (!query.exec()) {
        qDebug() << "addUser failed:" << query.lastError().text();
        return false;
    }
    return true;
}

bool Database::checkUser(const QString& login, const QString& password)
{
    QMutexLocker locker(&m_mutex);
    
    QSqlQuery query;
    query.prepare("SELECT password FROM users WHERE login = :login");
    query.bindValue(":login", login);
    
    if (!query.exec() || !query.next()) {
        return false;
    }
    
    return query.value(0).toString() == hashPassword(password);
}

bool Database::userExists(const QString& login)
{
    QMutexLocker locker(&m_mutex);
    
    QSqlQuery query;
    query.prepare("SELECT login FROM users WHERE login = :login");
    query.bindValue(":login", login);
    
    return query.exec() && query.next();
}

QString Database::getUserRole(const QString& login)
{
    QMutexLocker locker(&m_mutex);
    
    QSqlQuery query;
    query.prepare("SELECT role FROM users WHERE login = :login");
    query.bindValue(":login", login);
    
    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return "user";
}

bool Database::setUserRole(const QString& login, const QString& newRole)
{
    QMutexLocker locker(&m_mutex);
    
    QSqlQuery query;
    query.prepare("UPDATE users SET role = :role WHERE login = :login");
    query.bindValue(":role", newRole);
    query.bindValue(":login", login);
    
    return query.exec();
}

bool Database::deleteUser(const QString& login)
{
    QMutexLocker locker(&m_mutex);
    
    // Нельзя удалить последнего админа
    QSqlQuery countQuery;
    countQuery.exec("SELECT COUNT(*) FROM users WHERE role = 'admin'");
    if (countQuery.next() && countQuery.value(0).toInt() == 1) {
        QSqlQuery adminCheck;
        adminCheck.prepare("SELECT role FROM users WHERE login = :login");
        adminCheck.bindValue(":login", login);
        if (adminCheck.exec() && adminCheck.next() && adminCheck.value(0).toString() == "admin") {
            qDebug() << "Cannot delete the last admin user";
            return false;
        }
    }
    
    QSqlQuery query;
    query.prepare("DELETE FROM users WHERE login = :login");
    query.bindValue(":login", login);
    
    return query.exec();
}

QMap<QString, QString> Database::getAllUsers() const
{
    QMutexLocker locker(&m_mutex);
    
    QMap<QString, QString> users;
    QSqlQuery query("SELECT login, role FROM users ORDER BY login");
    
    while (query.next()) {
        users[query.value(0).toString()] = query.value(1).toString();
    }
    return users;
}

QString Database::getAllUsersFormatted() const
{
    QString result = "USERS:\n";
    QSqlQuery query("SELECT login, role FROM users ORDER BY login");
    
    while (query.next()) {
        result += "  " + query.value(0).toString() + " (" + query.value(1).toString() + ")\n";
    }
    return result.trimmed();
}

void Database::saveRequest(const QString& request, const QString& response)
{
    QMutexLocker locker(&m_mutex);
    
    QSqlQuery query;
    query.prepare("INSERT INTO requests (request, response) VALUES (:request, :response)");
    query.bindValue(":request", request);
    query.bindValue(":response", response);
    
    if (!query.exec()) {
        qDebug() << "Failed to save request:" << query.lastError().text();
    }
}

QString Database::getAllRequests() const
{
    QMutexLocker locker(&m_mutex);
    
    QString result = "REQUEST HISTORY:\n";
    QSqlQuery query("SELECT id, request, response, timestamp FROM requests ORDER BY id DESC LIMIT 50");
    
    while (query.next()) {
        result += QString("[%1] %2\n  -> %3\n")
            .arg(query.value(3).toDateTime().toString("hh:mm:ss"))
            .arg(query.value(1).toString())
            .arg(query.value(2).toString());
    }
    return result;
}

void Database::clearRequests()
{
    QMutexLocker locker(&m_mutex);
    QSqlQuery query("DELETE FROM requests");
}
