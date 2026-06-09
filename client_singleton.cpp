#include "client_singleton.h"
#include <iostream>
#include <limits>

ClientSingleton::ClientSingleton() : socket_(new QTcpSocket()), connected_(false) {}

ClientSingleton::~ClientSingleton() {
    disconnectFromServer();
    delete socket_;
}

ClientSingleton& ClientSingleton::getInstance() {
    static ClientSingleton instance;
    return instance;
}

void ClientSingleton::connectToServer(const QString& host, quint16 port) {
    if (connected_) {
        std::cout << "[Клиент]: Уже подключен к серверу.\n";
        return;
    }

    // Говорим сокету подключиться к серверу
    socket_->connectToHost(host, port);

    // Ждем ответа от сети 5 секунд
    if (socket_->waitForConnected(5000)) {
        connected_ = true;
        std::cout << "[Клиент]: Успешно подключен к TCP серверу (" << host.toStdString() << ":" << port << ")\n";
    }
    else {
        std::cout << "[Клиент]: Ошибка подключения: " << socket_->errorString().toStdString() << "\n";
    }
}

void ClientSingleton::disconnectFromServer() {
    if (connected_) {
        socket_->disconnectFromHost();
        connected_ = false;
        std::cout << "[Клиент]: Соединение закрыто.\n";
    }
}

void ClientSingleton::sendCommand(const std::string& command, const std::string& argument) {
    if (!connected_) {
        std::cout << "[Клиент]: Ошибка! Сначала подключитесь к серверу.\n";
        return;
    }

    // Собираем строку под формат сервера: команда|аргумент\n
    std::string requestStr = command;
    if (!argument.empty()) {
        requestStr += "|" + argument;
    }
    requestStr += "\n";

    // Пишем строку в сокет
    socket_->write(requestStr.c_str());
    socket_->flush(); // Принудительно выталкиваем данные в сеть

    // Ждем, пока сервер пришлет ответ (таймаут 5 секунд)
    if (socket_->waitForReadyRead(5000)) {
        QByteArray response = socket_->readAll();
        std::cout << "\n--- Ответ от сервера ---\n" << response.constData() << "------------------------\n";
    }
    else {
        std::cout << "[Клиент]: Сервер не ответил вовремя.\n";
    }
}

// Связываем пункты меню с отправкой команд
void ClientSingleton::sendEncrypt(const std::string& text) { sendCommand("encrypt", text); }
void ClientSingleton::sendDecrypt(const std::string& text) { sendCommand("decrypt", text); }
void ClientSingleton::sendHash(const std::string& text) { sendCommand("hash", text); }
void ClientSingleton::sendBisection(const std::string& args) { sendCommand("bisection", args); }
void ClientSingleton::sendShortestPath(const std::string& args) { sendCommand("shortest_path", args); }
void ClientSingleton::sendShowDb() { sendCommand("show_db", ""); } // Добавили пустую строку ""
void ClientSingleton::sendDbInfo() { sendCommand("db_info", ""); }

void ClientSingleton::showMenu() {
    while (true) {
        std::cout << "\n=== Меню Клиента (Вариант 3) ===\n";
        std::cout << "1. Шифрование LEMON (encrypt)\n";
        std::cout << "2. Дешифрование LEMON (decrypt)\n";
        std::cout << "3. Хеширование строки (hash)\n";
        std::cout << "4. Алгоритм Дихотомии (bisection)\n";
        std::cout << "5. Кратчайший путь (shortest_path)\n";
        std::cout << "6. Посмотреть логи сервера (show_db)\n";
        std::cout << "7. Узнать путь к БД на сервере (db_info)\n";
        std::cout << "0. Выход\n";
        std::cout << "Выберите действие: ";

        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        if (choice == 0) break;

        std::string argument;
        if (choice >= 1 && choice <= 5) {
            std::cout << "Введите аргумент для команды: ";
            std::cin >> argument;
        }

        switch (choice) {
        case 1: sendEncrypt(argument); break;
        case 2: sendDecrypt(argument); break;
        case 3: sendHash(argument); break;
        case 4: sendBisection(argument); break;
        case 5: sendShortestPath(argument); break;
        case 6: sendShowDb(); break;
        case 7: sendDbInfo(); break;
        default: std::cout << "Неверный пункт.\n"; break;
        }
    }
}
