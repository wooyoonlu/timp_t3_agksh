#ifndef CLIENT_SINGLETON_H
#define CLIENT_SINGLETON_H

#include <string>
#include <QTcpSocket> // Подключаем сокеты Qt вместо базы данных сервера

class ClientSingleton {
public:
    static ClientSingleton& getInstance(); // это статическая функция доступа
    // Метод, который сам собирает строку вида "команда|аргумент\n" и шлет в сеть
    void sendCommand(const std::string& command, const std::string& argument = "");

    // Настоящее подключение по TCP к серверу
    void connectToServer(const QString& host = "127.0.0.1", quint16 port = 33333);
    void disconnectFromServer();

    // Методы под протокол Варианта 3 (из ТЗ сервера)
    void sendEncrypt(const std::string& text);
    void sendDecrypt(const std::string& text);
    void sendHash(const std::string& text);
    void sendBisection(const std::string& args);
    void sendShortestPath(const std::string& args);
    void sendShowDb();
    void sendDbInfo();

    void showMenu();

private:
    ClientSingleton();
    ~ClientSingleton();
    ClientSingleton(const ClientSingleton&) = delete;
    ClientSingleton& operator=(const ClientSingleton&) = delete;


    QTcpSocket* socket_; // Наш сетевой сокет
    bool connected_;     // Флаг: подключены или нет
};

#endif // CLIENT_SINGLETON_H
