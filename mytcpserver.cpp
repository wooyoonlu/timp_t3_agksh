#include "mytcpserver.h"
#include <QDebug>
#include <QCoreApplication>
#include <QString>

MyTcpServer::~MyTcpServer()
{
    mTcpServer->close();
}

MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent)
{
    mTcpServer = new QTcpServer(this);

    connect(mTcpServer, &QTcpServer::newConnection,
            this, &MyTcpServer::slotNewConnection);

    if(!mTcpServer->listen(QHostAddress::Any, 33333)){
        qDebug() << "server is not started";
    } else {
        qDebug() << "server is started";
    }
}

void MyTcpServer::slotNewConnection()
{
    mTcpSocket = mTcpServer->nextPendingConnection();

    mTcpSocket->write("Hello, World!!! I am echo server!\r\n");

    connect(mTcpSocket, &QTcpSocket::readyRead,
            this,&MyTcpServer::slotServerRead);

    connect(mTcpSocket,&QTcpSocket::disconnected,
            this,&MyTcpServer::slotClientDisconnected);
}

void MyTcpServer::slotServerRead()
{
    QString res = "";

    while(mTcpSocket->bytesAvailable()>0)
    {
        QByteArray array = mTcpSocket->readAll();

        qDebug() << array << "\n";

        res.append(array);
    }

    QString response = handleRequest(res);

    mTcpSocket->write(response.toUtf8());
}

void MyTcpServer::slotClientDisconnected()
{
    mTcpSocket->close();
}

QString MyTcpServer::handleRequest(QString request)
{
    QStringList parts = request.trimmed().split("|");

    if (parts.size() == 0 || parts[0].isEmpty()) {
        return "Error: empty request";
    }

    QString action = parts[0];

    if (action == "encrypt") {
        return "Stub: Vigenere encrypt";
    }
    else if (action == "decrypt") {
        return "Stub: Vigenere decrypt";
    }
    else if (action == "hash") {
        return "Stub: SHA-512 hash";
    }
    else if (action == "bisection") {
        return "Stub: bisection method";
    }
    else if (action == "shortest_path") {
        return "Stub: shortest path";
    }
    else {
        return "Error: unknown request";
    }
}