QT -= gui
QT += network sql

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    mytcpserver.cpp \
    database.cpp

HEADERS += \
    mytcpserver.h \
    database.h
