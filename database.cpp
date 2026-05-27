#include "database.h"

Database* Database::instance = nullptr;

Database::Database()
{
    qDebug() << "Database object created";
}

Database* Database::getInstance()
{
    if (instance == nullptr) {
        instance = new Database();
    }

    return instance;
}

void Database::connect()
{
    qDebug() << "Database connected";
}

void Database::saveRequest(QString request)
{
    qDebug() << "Request saved:" << request;
}
