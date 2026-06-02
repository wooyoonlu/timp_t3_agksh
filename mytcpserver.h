#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QMap>

class MyTcpServer : public QObject
{
    Q_OBJECT

public:
    explicit MyTcpServer(QObject *parent = nullptr);
    ~MyTcpServer();

public slots:
    void slotNewConnection();
    void slotClientDisconnected();
    void slotServerRead();

    QString handleRequest(const QString& request, QTcpSocket* client);

private:
    QTcpServer *mTcpServer;
    QList<QTcpSocket*> m_clients;               // все подключенные клиенты
    QMap<QTcpSocket*, QString> m_sessions;     // сокет -> логин авторизованного пользователя
};

#endif // MYTCPSERVER_H
