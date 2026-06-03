#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QString>

/**
 * @brief TCP server that parses requests and dispatches variant 3 functions.
 */
class MyTcpServer : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Starts a TCP server on port 33333.
     * @param parent Optional Qt parent object.
     */
    explicit MyTcpServer(QObject *parent = nullptr);
    ~MyTcpServer() override;

public slots:
    /**
     * @brief Registers all pending client connections.
     */
    void slotNewConnection();

    /**
     * @brief Deletes a socket after its client disconnects.
     */
    void slotClientDisconnected();

    /**
     * @brief Reads and processes complete newline-delimited requests.
     */
    void slotServerRead();

    /**
     * @brief Parses one request and invokes the corresponding function.
     * @param request Request in `command|argument|...` format.
     * @param clientSocket Socket of the client making the request.
     * @return Text response for the client.
     */
    QString handleRequest(const QString &request, QTcpSocket *clientSocket);

private:
    void processBufferedRequests(QTcpSocket *socket, bool flushPartial);
    void processRequest(QTcpSocket *socket, const QByteArray &request);
    void scheduleBufferFlush(QTcpSocket *socket);

    QTcpServer *mTcpServer;
};

#endif // MYTCPSERVER_H
