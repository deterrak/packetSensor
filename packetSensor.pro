QT += core
QT += network
QT -= gui


CONFIG += c++11 -pthread

TARGET = packetSensor
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    tcpdumpThread.cpp \
    httppost.cpp

HEADERS += \
    tcpdumpThread.h \
    httppost.h
