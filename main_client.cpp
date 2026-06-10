#include "mainwindow.h"
#include <QApplication>

#ifdef _WIN32
#include <windows.h> // Подключаем библиотеку для работы с Windows API
#endif

int main(int argc, char *argv[])
{
#ifdef _WIN32
    // Настраиваем консоль на работу с UTF-8 (кириллица и латиница)
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
#endif

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
