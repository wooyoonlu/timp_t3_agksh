QT -= gui
QT += testlib sql

CONFIG += c++17 console testcase cmdline
CONFIG -= app_bundle

TARGET = ServerParseUnitTest

SOURCES += \
    ../database.cpp \
    ../variantfunctions.cpp \
    tst_serverparse.cpp

HEADERS += \
    ../database.h \
    ../variantfunctions.h
