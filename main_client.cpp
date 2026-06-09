#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv); // Запуск графического движка Qt
    MainWindow w;               // Создаем наше окно
    w.show();                   // Показываем окно на экране
    return a.exec();            // Держим программу открытой, пока окно не закроют
}
