#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QDebug>

class Database
{
private:
    static Database* instance;

    Database();

public:
    static Database* getInstance();

    void connect();
    void saveRequest(QString request);
};

#endif
