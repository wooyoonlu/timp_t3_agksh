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
        socket->setProperty("loggedIn", false);
        socket->setProperty("userLogin", QString());
        socket->write("OK|connected\n");

        connect(socket, &QTcpSocket::readyRead,
                this, &MyTcpServer::slotServerRead);

        connect(socket, &QTcpSocket::disconnected,
                this, &MyTcpServer::slotClientDisconnected);
        
        qDebug() << "New client connected";
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
    const QString response = handleRequest(requestText, socket);
    
    // сохраняем запрос в бд (скрываем пароли)
    QString safeRequest = requestText;
    if (safeRequest.startsWith("reg|", Qt::CaseInsensitive) ||
        safeRequest.startsWith("login|", Qt::CaseInsensitive)) {
        safeRequest = safeRequest.split('|').mid(0, 2).join('|') + "|***";
    }
    Database::instance().saveRequest(safeRequest, response);

    socket->write(response.toUtf8());
    socket->write("\n");
}

void MyTcpServer::slotClientDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (socket) {
        qDebug() << "Client disconnected";
        socket->deleteLater();
    }
}

QString MyTcpServer::handleRequest(const QString &request, QTcpSocket *clientSocket)
{
    if (request.trimmed().isEmpty()) {
        return "Error: empty request";
    }

    const QStringList parts = request.split('|', Qt::KeepEmptyParts);
    if (parts.isEmpty()) {
        return "Error: invalid request format";
    }
    
    const QString action = parts.at(0).trimmed().toLower();

    // команды без авторизации
    if (action == "reg") {
        if (parts.size() < 3) {
            return "Error: usage: reg|login|password";
        }
        QString login = parts.at(1).trimmed();
        QString password = parts.at(2).trimmed();
        
        if (login.isEmpty() || password.isEmpty()) {
            return "Error: login and password cannot be empty";
        }
        
        if (login.length() > 20) {
            return "Error: login too long (max 20 characters)";
        }
        
        if (Database::instance().userExists(login)) {
            return "Error: user already exists";
        }
        
        if (Database::instance().addUser(login, password, "user")) {
            return "OK|registration successful";
        } else {
            return "Error: registration failed";
        }
    }

    if (action == "login") {
        if (parts.size() < 3) {
            return "Error: usage: login|login|password";
        }
        QString login = parts.at(1).trimmed();
        QString password = parts.at(2).trimmed();
        
        if (!Database::instance().checkUser(login, password)) {
            return "Error: invalid login or password";
        }
        
        clientSocket->setProperty("loggedIn", true);
        clientSocket->setProperty("userLogin", login);
        
        QString role = Database::instance().getUserRole(login);
        return "OK|login successful|role=" + role;
    }

    // проверка статуса (отладка)
    if (action == "status") {
        bool loggedIn = clientSocket->property("loggedIn").toBool();
        QString login = clientSocket->property("userLogin").toString();
        if (loggedIn) {
            return "OK|logged in as " + login;
        } else {
            return "OK|not logged in";
        }
    }

    if (action == "db_info") {
        if (parts.size() != 1) {
            return "Error: usage: db_info";
        }
        return "OK|database_path=" + Database::instance().databasePath();
    }

    // просмотр истории запросов (только админ)
    if (action == "show_db") {
        bool loggedIn = clientSocket->property("loggedIn").toBool();
        QString login = clientSocket->property("userLogin").toString();
        
        if (!loggedIn) {
            return "Error: not logged in";
        }
        
        QString role = Database::instance().getUserRole(login);
        if (role != "admin") {
            return "Error: admin rights required";
        }
        
        return Database::instance().getAllRequests();
    }

    // проверка авторизации для остальных команд
    bool loggedIn = clientSocket->property("loggedIn").toBool();
    if (!loggedIn && action != "reg" && action != "login" && 
        action != "db_info" && action != "status") {
        return "Error: not logged in. Please use 'login|login|password' first";
    }

    // выход из системы
    if (action == "logout") {
        clientSocket->setProperty("loggedIn", false);
        clientSocket->setProperty("userLogin", QString());
        return "OK|logged out successfully";
    }

    // админ: список всех пользователей
    if (action == "admin_list_users") {
        QString login = clientSocket->property("userLogin").toString();
        QString role = Database::instance().getUserRole(login);
        
        if (role != "admin") {
            return "Error: admin rights required";
        }
        
        return Database::instance().getAllUsersFormatted();
    }

    // админ: изменить роль пользователя
    if (action == "admin_set_role") {
        if (parts.size() < 3) {
            return "Error: usage: admin_set_role|login|role";
        }
        
        QString adminLogin = clientSocket->property("userLogin").toString();
        QString adminRole = Database::instance().getUserRole(adminLogin);
        
        if (adminRole != "admin") {
            return "Error: admin rights required";
        }
        
        QString targetLogin = parts.at(1).trimmed();
        QString newRole = parts.at(2).trimmed().toLower();
        
        if (newRole != "user" && newRole != "admin") {
            return "Error: invalid role. Must be 'user' or 'admin'";
        }
        
        if (!Database::instance().userExists(targetLogin)) {
            return "Error: user does not exist";
        }
        
        if (Database::instance().setUserRole(targetLogin, newRole)) {
            return "OK|role changed: " + targetLogin + " is now " + newRole;
        } else {
            return "Error: failed to change role";
        }
    }

    // админ: удалить пользователя
    if (action == "admin_delete_user") {
        if (parts.size() < 2) {
            return "Error: usage: admin_delete_user|login";
        }
        
        QString adminLogin = clientSocket->property("userLogin").toString();
        QString adminRole = Database::instance().getUserRole(adminLogin);
        
        if (adminRole != "admin") {
            return "Error: admin rights required";
        }
        
        QString targetLogin = parts.at(1).trimmed();
        
        if (targetLogin == adminLogin) {
            return "Error: cannot delete your own account";
        }
        
        if (!Database::instance().userExists(targetLogin)) {
            return "Error: user does not exist";
        }
        
        if (Database::instance().deleteUser(targetLogin)) {
            return "OK|user deleted: " + targetLogin;
        } else {
            return "Error: failed to delete user";
        }
    }

    // основной функционал темы 3

    // шифрование виженера
    if (action == "encrypt") {
        if (parts.size() < 3) {
            return "Error: usage: encrypt|key|text";
        }
        QString error;
        const QString cipher = VariantFunctions::vigenereEncrypt(
            parts.mid(2).join('|'), parts.at(1), &error);
        return error.isEmpty() ? "OK|cipher=" + cipher : "Error: " + error;
    }

    // дешифрование виженера
    if (action == "decrypt") {
        if (parts.size() < 3) {
            return "Error: usage: decrypt|key|text";
        }
        QString error;
        const QString text = VariantFunctions::vigenereDecrypt(
            parts.mid(2).join('|'), parts.at(1), &error);
        return error.isEmpty() ? "OK|text=" + text : "Error: " + error;
    }

    // sha-512 хеш
    if (action == "hash") {
        if (parts.size() < 2) {
            return "Error: usage: hash|text";
        }
        return "OK|sha512=" + VariantFunctions::sha512(parts.mid(1).join('|'));
    }

    // метод деления пополам
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

    // поиск кратчайшего пути в графе
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

    // справка
    if (action == "help") {
        return "OK|Available commands:\n"
               "  reg|login|password - register new user\n"
               "  login|login|password - authenticate\n"
               "  logout - end session\n"
               "  status - check login status\n"
               "  encrypt|key|text - Vigenere encryption\n"
               "  decrypt|key|cipher - Vigenere decryption\n"
               "  hash|text - SHA-512 hash\n"
               "  bisection|left|right|eps - find root of x^2-4\n"
               "  shortest_path|start|end|edges - Dijkstra algorithm\n"
               "  admin_list_users - show all users (admin only)\n"
               "  admin_set_role|login|role - change user role (admin only)\n"
               "  admin_delete_user|login - delete user (admin only)\n"
               "  show_db - show request history (admin only)\n"
               "  db_info - show database path\n"
               "  help - show this message";
    }

    return "Error: unknown request. Try 'help' for available commands.";
}
