QT += core serialbus serialport network
QT -= gui

CONFIG += c++11

TARGET = QUaModbusClient
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += $$PWD/

SOURCES += main.cpp

HEADERS += quamodbusclient.h
SOURCES += quamodbusclient.cpp

include($$PWD/../libs/QUaServer.git/src/wrapper/quaserver.pri)
include($$PWD/../libs/QUaServer.git/src/helper/add_qt_path_win.pri)