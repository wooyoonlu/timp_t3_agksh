#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QTcpSocket>

class NetworkClient : public QObject
{
    Q_OBJECT
private:
    // Приватный конструктор запрещает создавать объекты напрямую
    explicit NetworkClient(QObject *parent = nullptr);
    static NetworkClient* instance; // Тот самый единственный экземпляр
    QTcpSocket *socket;

public:
    // Глобальная точка доступа к клиенту
    static NetworkClient* getInstance();

    void connectToServer(const QString &host, quint16 port);
    void sendRequest(const QString &request);

signals:
    void responseReceived(const QString &response);
    void connectedToServer();

private slots:
    void onReadyRead();
};

#endif // NETWORKCLIENT_H
