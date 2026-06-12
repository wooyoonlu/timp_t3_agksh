#include <QtTest>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSignalSpy>
#include "networkclient.h"

class TestClient : public QObject
{
    Q_OBJECT

private slots:
    // Тест 1: Проверка Singleton
    void testSingletonInstance()
    {
        NetworkClient* client1 = NetworkClient::getInstance();
        NetworkClient* client2 = NetworkClient::getInstance();
        QCOMPARE(client1, client2);
        QVERIFY(client1 != nullptr);
    }

    // Тест 2: Проверка автоматического добавления \n к запросу
    void testRequestFormatting()
    {
        NetworkClient* client = NetworkClient::getInstance();

        // Создаем локальный сервер для теста
        QTcpServer mockServer;
        QVERIFY(mockServer.listen(QHostAddress::LocalHost, 33333));

        // Подключаемся к нему
        client->connectToServer("127.0.0.1", 33333);

        // ИСПРАВЛЕНО: Шпионим за публичным сигналом самого клиента, а не за приватным сокетом!
        QSignalSpy spyConnected(client, &NetworkClient::connectedToServer);
        spyConnected.wait(1000);

        // Если сервер принял подключение
        if (mockServer.waitForNewConnection(1000)) {
            QTcpSocket* serverSideSocket = mockServer.nextPendingConnection();
            if (serverSideSocket) {
                QString rawRequest = "hash|hello";
                client->sendRequest(rawRequest);

                // Ждем данные на стороне сервера
                serverSideSocket->waitForReadyRead(1000);
                QByteArray receivedData = serverSideSocket->readAll();

                // Проверяем, добавился ли \n автоматически (Решение проблемы D-001)
                QCOMPARE(receivedData, QByteArray("hash|hello\n"));
                serverSideSocket->close();
            }
        }
        mockServer.close();
    }
};

QTEST_MAIN(TestClient)
#include "tst_testclient.moc"
