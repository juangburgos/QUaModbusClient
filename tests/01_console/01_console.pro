QT += core
QT -= gui

TARGET  = 01_console
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += $$PWD/

SOURCES += main.cpp

include($$PWD/../../src/types/quamodbusclient.pri)
include($$PWD/../../libs/QDeferred.git/src/qlambdathreadworker.pri)
include($$PWD/../../libs/QUaServer.git/src/wrapper/quaserver.pri)
include($$PWD/../../libs/QUaServer.git/src/helper/add_qt_path_win.pri)