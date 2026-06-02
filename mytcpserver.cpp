#include "mytcpserver.h"

#include "database.h"
#include "variantfunctions.h"

#include <QByteArray>
#include <QDebug>
#include <QPointer>
#include <QTimer>

MyTcpServer::~MyTcpServer()
{
    mTcpServer->close();
}

MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent)
{
    mTcpServer = new QTcpServer(this);

    Database::instance().open();

    connect(mTcpServer, &QTcpServer::newConnection,
            this, &MyTcpServer::slotNewConnection);

    if (!mTcpServer->listen(QHostAddress::Any, 33333)) {
        qDebug() << "server is not started";
    } else {
        qDebug() << "server is started";
    }
}

void MyTcpServer::slotNewConnection()
{
    while (mTcpServer->hasPendingConnections()) {
        QTcpSocket *socket = mTcpServer->nextPendingConnection();
        socket->setProperty("requestBuffer", QByteArray());
        socket->setProperty("flushScheduled", false);
        socket->write("OK|connected\n");

        connect(socket, &QTcpSocket::readyRead,
                this, &MyTcpServer::slotServerRead);

        connect(socket, &QTcpSocket::disconnected,
                this, &MyTcpServer::slotClientDisconnected);
    }
}

void MyTcpServer::slotServerRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) {
        return;
    }

    QByteArray buffer = socket->property("requestBuffer").toByteArray();
    buffer.append(socket->readAll());
    socket->setProperty("requestBuffer", buffer);

    processBufferedRequests(socket, false);
    if (!socket->property("requestBuffer").toByteArray().isEmpty()) {
        scheduleBufferFlush(socket);
    }
}

void MyTcpServer::processBufferedRequests(QTcpSocket *socket,
                                          bool flushPartial)
{
    QByteArray buffer = socket->property("requestBuffer").toByteArray();
    qsizetype newlineIndex = buffer.indexOf('\n');

    while (newlineIndex >= 0) {
        QByteArray request = buffer.left(newlineIndex);
        buffer.remove(0, newlineIndex + 1);
        if (request.endsWith('\r')) {
            request.chop(1);
        }
        processRequest(socket, request);
        newlineIndex = buffer.indexOf('\n');
    }

    if (flushPartial && !buffer.isEmpty()) {
        processRequest(socket, buffer);
        buffer.clear();
    }

    socket->setProperty("requestBuffer", buffer);
}

void MyTcpServer::scheduleBufferFlush(QTcpSocket *socket)
{
    if (socket->property("flushScheduled").toBool()) {
        return;
    }

    socket->setProperty("flushScheduled", true);
    const QPointer<QTcpSocket> guardedSocket(socket);
    QTimer::singleShot(30, socket, [this, guardedSocket]() {
        if (!guardedSocket) {
            return;
        }
        guardedSocket->setProperty("flushScheduled", false);
        processBufferedRequests(guardedSocket, true);
    });
}

void MyTcpServer::processRequest(QTcpSocket *socket, const QByteArray &request)
{
    const QString requestText = QString::fromUtf8(request);
    if (requestText.trimmed().isEmpty()) {
        return;
    }

    qDebug() << "Request:" << requestText;
    const QString response = handleRequest(requestText);
    const QString storedResponse =
        requestText.trimmed().compare("show_db", Qt::CaseInsensitive) == 0
                                       ? "Database contents returned"
                                       : response;
    Database::instance().saveRequest(requestText, storedResponse);

    socket->write(response.toUtf8());
    socket->write("\n");
}

void MyTcpServer::slotClientDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (socket) {
        socket->deleteLater();
    }
}

QString MyTcpServer::handleRequest(const QString &request)
{
    if (request.trimmed().isEmpty()) {
        return "Error: empty request";
    }

    const QStringList parts = request.split('|', Qt::KeepEmptyParts);
    const QString action = parts.at(0).trimmed().toLower();

    if (action == "show_db") {
        if (parts.size() != 1) {
            return "Error: usage: show_db";
        }
        return Database::instance().getAllRequests();
    }

    if (action == "db_info") {
        if (parts.size() != 1) {
            return "Error: usage: db_info";
        }
        return "OK|database_path=" + Database::instance().databasePath();
    }

    if (action == "encrypt") {
        if (parts.size() < 3) {
            return "Error: usage: encrypt|key|text";
        }
        QString error;
        const QString cipher = VariantFunctions::vigenereEncrypt(
            parts.mid(2).join('|'), parts.at(1), &error);
        return error.isEmpty() ? "OK|cipher=" + cipher : "Error: " + error;
    }

    if (action == "decrypt") {
        if (parts.size() < 3) {
            return "Error: usage: decrypt|key|text";
        }
        QString error;
        const QString text = VariantFunctions::vigenereDecrypt(
            parts.mid(2).join('|'), parts.at(1), &error);
        return error.isEmpty() ? "OK|text=" + text : "Error: " + error;
    }

    if (action == "hash") {
        if (parts.size() < 2) {
            return "Error: usage: hash|text";
        }
        return "OK|sha512=" + VariantFunctions::sha512(parts.mid(1).join('|'));
    }

    if (action == "bisection") {
        if (parts.size() != 4) {
            return "Error: usage: bisection|left|right|epsilon";
        }

        bool leftOk = false;
        bool rightOk = false;
        bool epsilonOk = false;
        const double left = parts.at(1).toDouble(&leftOk);
        const double right = parts.at(2).toDouble(&rightOk);
        const double epsilon = parts.at(3).toDouble(&epsilonOk);

        if (!leftOk || !rightOk || !epsilonOk) {
            return "Error: boundaries and epsilon must be numbers";
        }

        const VariantFunctions::BisectionResult result =
            VariantFunctions::bisection(left, right, epsilon);
        if (!result.success) {
            return "Error: " + result.error;
        }

        return QString("OK|root=%1|iterations=%2")
            .arg(result.root, 0, 'g', 15)
            .arg(result.iterations);
    }

    if (action == "shortest_path") {
        if (parts.size() != 4) {
            return "Error: usage: shortest_path|start|finish|"
                   "from,to,weight;from,to,weight";
        }

        const QStringList edges =
            parts.at(3).split(';', Qt::SkipEmptyParts);
        const VariantFunctions::ShortestPathResult result =
            VariantFunctions::shortestPath(parts.at(1).trimmed(),
                                           parts.at(2).trimmed(),
                                           edges);
        if (!result.success) {
            return "Error: " + result.error;
        }

        return QString("OK|distance=%1|path=%2")
            .arg(result.distance, 0, 'g', 15)
            .arg(result.path.join("->"));
    }

    return "Error: unknown request";
}
