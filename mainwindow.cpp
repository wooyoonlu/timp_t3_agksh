#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "client_singleton.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnSend_clicked()
{
    // 1. Забираем текст из нижней белой полоски (куда ты будешь печатать команду)
    // Если ты назвала поле "inputMessage", то код такой:
    QString textMessage = ui->inputMessage->text();

    // 2. Если поле было пустым, ничего не отправляем, просто выходим
    if (textMessage.isEmpty()) {
        return;
    }

    // 3. Берем твой готовый клиент (Синглтон)
    ClientSingleton& client = ClientSingleton::getInstance();

    // 4. Отправляем текст на сервер
    // ВНИМАНИЕ: замени "sendMessage" на то, как у тебя называется функция отправки!
    client.sendCommand(textMessage.toStdString());

    // 5. Печатаем этот же текст в большой белый квадрат посередине (textLogs),
    // чтобы ты видела, что именно отправила
    ui->textLogs->append("Вы: " + textMessage);

    // 6. Очищаем нижнюю белую полоску ввода, чтобы было удобно писать следующее сообщение
    ui->inputMessage->clear();
}


void MainWindow::on_btnShowDb_clicked()
{
    ClientSingleton::getInstance().sendCommand("show_db");
    ui->textLogs->append("Команда: show_db");
}


void MainWindow::on_btnDbInfo_clicked()
{
    ClientSingleton::getInstance().sendCommand("db_info");
    ui->textLogs->append("Команда: db_info");
}


void MainWindow::on_btnBisection_clicked()
{
    ClientSingleton::getInstance().sendCommand("bisection");
    ui->textLogs->append("Команда: bisection");
}


void MainWindow::on_btnShortestPath_clicked()
{
    ClientSingleton::getInstance().sendCommand("shortest_path");
    ui->textLogs->append("Команда: shortest_path");
}


void MainWindow::on_btnEncrypt_clicked()
{
    ClientSingleton::getInstance().sendCommand("encrypt");
    ui->textLogs->append("Команда: encrypt");
}

