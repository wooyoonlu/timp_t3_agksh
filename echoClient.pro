QT += core gui widgets network

CONFIG += c++17 console
CONFIG -= app_bundle

SOURCES += main_client.cpp \
           client_singleton.cpp \
           mainwindow.cpp \
           networkclient.cpp

HEADERS += client_singleton.h \
    mainwindow.h \
    networkclient.h

FORMS += \
    mainwindow.ui
