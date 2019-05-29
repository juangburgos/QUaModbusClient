QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = widget
TEMPLATE = app


SOURCES += main.cpp\
        quamodbusclientwidgettest.cpp

HEADERS  += quamodbusclientwidgettest.h

FORMS    += quamodbusclientwidgettest.ui

include($$PWD/../../src/widget/quamodbusclientwidget.pri)
include($$PWD/../../libs/QDeferred.git/src/qlambdathreadworker.pri)
include($$PWD/../../libs/QUaServer.git/src/wrapper/quaserver.pri)
include($$PWD/../../libs/QUaServer.git/src/helper/add_qt_path_win.pri)