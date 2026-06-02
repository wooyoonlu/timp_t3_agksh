QT -= gui

QT += network sql

CONFIG += c++17 console
CONFIG -= app_bundle

SOURCES += \
    database.cpp \
    main.cpp \
    mytcpserver.cpp \
    variantfunctions.cpp

HEADERS += \
    database.h \
    mytcpserver.h \
    variantfunctions.h
