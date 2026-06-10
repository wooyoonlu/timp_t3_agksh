#include "networkclient.h"

// Инициализируем указатель нулем
NetworkClient* NetworkClient::instance = nullptr;

NetworkClient::NetworkClient(QObject *parent) : QObject(parent)
{
    socket = new QTcpSocket(this);

    connect(socket, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    connect(socket, &QTcpSocket::connected, this, &NetworkClient::connectedToServer);
}

NetworkClient* NetworkClient::getInstance()
{
    if (!instance) {
        instance = new NetworkClient();
    }
    return instance;
}

void NetworkClient::connectToServer(const QString &host, quint16 port)
{
    if (socket->state() == QAbstractSocket::UnconnectedState) {
        socket->connectToHost(host, port);
    }
}

void NetworkClient::sendRequest(const QString &request)
{
    if (socket->state() == QAbstractSocket::ConnectedState) {
        QString safeRequest = request;
        // Решение проблемы D-001 из вашего файла:
        // Всегда добавляем \n в конец, если его там нет
        if (!safeRequest.endsWith('\n')) {
            safeRequest += '\n';
        }
        socket->write(safeRequest.toUtf8());
    }
}

void NetworkClient::onReadyRead()
{
    // Читаем ответы сервера построчно
    while (socket->canReadLine()) {
        QByteArray data = socket->readLine();
        emit responseReceived(QString::fromUtf8(data).trimmed());
    }
}
