#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "networkclient.h" // Подключаем синглтон

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. Получаем доступ к синглтону
    NetworkClient *client = NetworkClient::getInstance();

    // 2. Подписываемся на ответы от сервера
    connect(client, &NetworkClient::responseReceived, this, [this](const QString &response) {
        ui->textLog->append("<b>Сервер:</b> " + response);
    });

    connect(client, &NetworkClient::connectedToServer, this, [this]() {
        ui->textLog->append("<font color='green'>Подключено к серверу!</font>");
    });

    // 3. Авто-подключение при старте окна
   // client->connectToServer("127.0.0.1", 33333);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Слот нажатия на главную кнопку "Отправить" (справа)
void MainWindow::on_btnSend_clicked()
{
    QString operation = ui->cmbOperation->currentText();
    QString p1 = ui->lineEditP1->text();
    QString p2 = ui->lineEditP2->text();
    QString p3 = ui->lineEditP3->text();

    QString request;

    // Формируем запросы строго по спецификации сервера
    if (operation == "Шифр Виженера") {
        request = QString("encrypt|%1|%2").arg(p1, p2);
    }
    else if (operation == "SHA-512") {
        request = QString("hash|%1").arg(p1);
    }
    else if (operation == "Деление пополам") {
        request = QString("bisection|%1|%2|%3").arg(p1, p2, p3);
    }
    else if (operation == "Алгоритм Дейкстры") {
        request = QString("shortest_path|%1|%2|%3").arg(p1, p2, p3);
    }
    else if (operation == "Информация о БД") {
        request = "db_info";
    }

    if (!request.isEmpty()) {
        ui->textLog->append("Вы: " + request);
        NetworkClient::getInstance()->sendRequest(request);
    }
}

// Слот нажатия на кнопку "Подключиться" (слева)
void MainWindow::on_btnConnect_clicked()
{
    ui->textLog->append("Попытка подключения к серверу...");
    NetworkClient::getInstance()->connectToServer("127.0.0.1", 33333);
}
