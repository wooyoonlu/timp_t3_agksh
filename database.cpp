#include "database.h"

Database* Database::instance = nullptr;
QMutex Database::mutex;

Database::Database()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("server.db");
    if (!db.open()) {
        qDebug() << "Database open error:" << db.lastError().text();
    } else {
        qDebug() << "Database opened successfully";
    }

    // Создаём таблицу пользователей
    QSqlQuery query;
    if (!query.exec("CREATE TABLE IF NOT EXISTS users ("
                    "login VARCHAR(20) PRIMARY KEY, "
                    "password VARCHAR(128) NOT NULL, "
                    "role VARCHAR(20) NOT NULL DEFAULT 'user')")) {
        qDebug() << "Table creation error:" << query.lastError().text();
    } else {
        qDebug() << "Table 'users' ready";
    }

    // Добавляем тестового админа, если нет
    if (!checkUser("admin", "admin123")) {
        addUser("admin", "admin123", "admin");
        qDebug() << "Test admin user created (login: admin, password: admin123)";
    }
}

QString Database::hashPassword(const QString& password)
{
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha512);
    return hash.toHex();
}

Database* Database::getInstance()
{
    if (!instance) {
        QMutexLocker locker(&mutex);
        if (!instance) {
            instance = new Database();
        }
    }
    return instance;
}

bool Database::addUser(const QString& login, const QString& password, const QString& role)
{
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
    QSqlQuery query;
    query.prepare("SELECT password FROM users WHERE login = :login");
    query.bindValue(":login", login);
    if (!query.exec() || !query.next())
        return false;
    QString storedHash = query.value(0).toString();
    return storedHash == hashPassword(password);
}

QString Database::getUserRole(const QString& login)
{
    QSqlQuery query;
    query.prepare("SELECT role FROM users WHERE login = :login");
    query.bindValue(":login", login);
    if (query.exec() && query.next())
        return query.value(0).toString();
    return "";
}

bool Database::setUserRole(const QString& login, const QString& newRole)
{
    QSqlQuery query;
    query.prepare("UPDATE users SET role = :role WHERE login = :login");
    query.bindValue(":role", newRole);
    query.bindValue(":login", login);
    return query.exec();
}

QMap<QString, QString> Database::getAllUsers()
{
    QMap<QString, QString> users;
    QSqlQuery query("SELECT login, role FROM users");
    while (query.next()) {
        users[query.value(0).toString()] = query.value(1).toString();
    }
    return users;
}
