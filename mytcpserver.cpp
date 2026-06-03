#include "mytcpserver.h"
#include "database.h"
#include <QDebug>

MyTcpServer::~MyTcpServer()
{
    mTcpServer->close();
    qDeleteAll(m_clients);
}

MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent)
{
    mTcpServer = new QTcpServer(this);

    connect(mTcpServer, &QTcpServer::newConnection,
            this, &MyTcpServer::slotNewConnection);

    if(!mTcpServer->listen(QHostAddress::Any, 33333)){
        qDebug() << "server is not started";
    } else {
        qDebug() << "server is started on port 33333";
    }
}

void MyTcpServer::slotNewConnection()
{
    QTcpSocket* clientSocket = mTcpServer->nextPendingConnection();
    m_clients.append(clientSocket);

    connect(clientSocket, &QTcpSocket::readyRead,
            this, &MyTcpServer::slotServerRead);
    connect(clientSocket, &QTcpSocket::disconnected,
            this, &MyTcpServer::slotClientDisconnected);

    qDebug() << "New client connected. Total clients:" << m_clients.size();
    clientSocket->write("Connected to server. Use commands: REG|login|password, LOGIN|login|password, ENCRYPT|text|key, etc.\r\n");
}

void MyTcpServer::slotServerRead()
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    QByteArray data = client->readAll();
    QString request = QString::fromUtf8(data).trimmed();
    qDebug() << "Received from" << client->socketDescriptor() << ":" << request;

    QString response = handleRequest(request, client);
    client->write(response.toUtf8());
    client->flush();
}

void MyTcpServer::slotClientDisconnected()
{
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    m_clients.removeAll(client);
    m_sessions.remove(client);
    client->deleteLater();

    qDebug() << "Client disconnected. Remaining clients:" << m_clients.size();
}

QString MyTcpServer::handleRequest(const QString& request, QTcpSocket* client)
{
    QStringList parts = request.split('|');
    if (parts.isEmpty()) {
        return "ERROR: empty request";
    }

    QString action = parts[0].toUpper();

    // Регистрация
    if (action == "REG") {
        if (parts.size() < 3) return "ERROR: need login and password";
        QString login = parts[1];
        QString password = parts[2];
        if (Database::getInstance()->addUser(login, password)) {
            return "REG_OK: user registered";
        } else {
            return "REG_FAIL: user already exists";
        }
    }
    // Логин
    else if (action == "LOGIN") {
        if (parts.size() < 3) return "ERROR: need login and password";
        QString login = parts[1];
        QString password = parts[2];
        if (Database::getInstance()->checkUser(login, password)) {
            m_sessions[client] = login;
            return "LOGIN_OK: welcome " + login + " (role=" + Database::getInstance()->getUserRole(login) + ")";
        } else {
            return "LOGIN_FAIL: invalid credentials";
        }
    }
    // Заглушки
    else if (action == "ENCRYPT") {
        if (!m_sessions.contains(client)) return "ERROR: not logged in";
        if (parts.size() < 3) return "ERROR: need text and key";
        QString text = parts[1];
        QString key = parts[2];
        return "ENCRYPT_STUB: would encrypt '" + text + "' with key '" + key + "'";
    }
    else if (action == "DECRYPT") {
        if (!m_sessions.contains(client)) return "ERROR: not logged in";
        if (parts.size() < 3) return "ERROR: need text and key";
        QString text = parts[1];
        QString key = parts[2];
        return "DECRYPT_STUB: would decrypt '" + text + "' with key '" + key + "'";
    }
    else if (action == "HASH") {
        if (!m_sessions.contains(client)) return "ERROR: not logged in";
        if (parts.size() < 2) return "ERROR: need input string";
        QString input = parts[1];
        return "HASH_STUB: SHA-512 of '" + input + "' would be here";
    }
    else if (action == "BISECTION") {
        if (!m_sessions.contains(client)) return "ERROR: not logged in";
        // пример: BISECTION|0|10|0.001
        if (parts.size() < 4) return "ERROR: need a, b, epsilon";
        double a = parts[1].toDouble();
        double b = parts[2].toDouble();
        double eps = parts[3].toDouble();
        return "BISECTION_STUB: root of f(x)=0 on [" + QString::number(a) + "," + QString::number(b) + "] with eps " + QString::number(eps);
    }
    else if (action == "SHORTEST_PATH") {
        if (!m_sessions.contains(client)) return "ERROR: not logged in";
        // формат: SHORTEST_PATH|4|0-1:5;0-2:3;1-3:2;2-3:1|0|3
        if (parts.size() < 5) return "ERROR: need N, edges, start, end";
        // заглушка
        return "SHORTEST_PATH_STUB: would compute distance from " + parts[3] + " to " + parts[4];
    }
    // команды админа
    else if (action == "ADMIN_LIST_USERS") {
        if (!m_sessions.contains(client)) return "ERROR: not logged in";
        QString login = m_sessions[client];
        if (Database::getInstance()->getUserRole(login) != "admin")
            return "ERROR: admin rights required";
        auto users = Database::getInstance()->getAllUsers();
        QString result = "USERS: ";
        for (auto it = users.begin(); it != users.end(); ++it) {
            result += it.key() + "(" + it.value() + ") ";
        }
        return result;
    }
    else if (action == "ADMIN_SET_ROLE") {
        if (!m_sessions.contains(client)) return "ERROR: not logged in";
        QString login = m_sessions[client];
        if (Database::getInstance()->getUserRole(login) != "admin")
            return "ERROR: admin rights required";
        if (parts.size() < 3) return "ERROR: need target_login and new_role";
        QString target = parts[1];
        QString newRole = parts[2];
        if (Database::getInstance()->setUserRole(target, newRole))
            return "ROLE_CHANGED: " + target + " now " + newRole;
        else
            return "ROLE_CHANGE_FAIL";
    }
    else {
        return "ERROR: unknown command. Available: REG, LOGIN, ENCRYPT, HASH, BISECTION, SHORTEST_PATH, ADMIN_LIST_USERS, ADMIN_SET_ROLE";
    }
}
