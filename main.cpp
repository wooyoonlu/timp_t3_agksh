#include <QCoreApplication>

#include "mytcpserver.h"

/**
 * @brief Starts the TCP server event loop.
 * @param argc Argument count.
 * @param argv Argument values.
 * @return Qt application exit code.
 */
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setOrganizationName("ProgrammingTechnologyCourse");
    QCoreApplication::setApplicationName("Variant3TcpServer");
    MyTcpServer myserv;
    return a.exec();
}
