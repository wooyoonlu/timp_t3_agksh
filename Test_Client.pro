QT += core testlib network

CONFIG += c++17 testcase
CONFIG -= app_bundle

TARGET = tst_client
TEMPLATE = app

# Файл самого теста (Qt Creator создаст его сам, проверь точное имя!)
SOURCES += tst_testclient.cpp \
           ../echoClient/networkclient.cpp

HEADERS += ../echoClient/networkclient.h

# Показываем компилятору, где искать заголовочные файлы основного проекта
INCLUDEPATH += ../echoClient
