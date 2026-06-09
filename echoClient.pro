QT += core gui widgets network

CONFIG += c++17 console
CONFIG -= app_bundle

SOURCES += main_client.cpp \
           client_singleton.cpp \
           mainwindow.cpp

HEADERS += client_singleton.h \
    mainwindow.h

FORMS += \
    mainwindow.ui
