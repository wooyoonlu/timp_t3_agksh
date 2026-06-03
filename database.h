#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMutex>
#include <QMap>
#include <QCryptographicHash>

class Database
{
private:
    static Database* instance;
    static QMutex mutex;
    QSqlDatabase db;

    Database();
    QString hashPassword(const QString& password);

public:
    static Database* getInstance();

    bool addUser(const QString& login, const QString& password, const QString& role = "user");
    bool checkUser(const QString& login, const QString& password);
    QString getUserRole(const QString& login);
    bool setUserRole(const QString& login, const QString& newRole);
    QMap<QString, QString> getAllUsers();
};

#endif 
