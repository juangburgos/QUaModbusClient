QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

CONFIG(debug, debug|release) {
	TARGET = QUaModbusClientd
} else {
	TARGET = QUaModbusClient
}

SOURCES += \
main.cpp \
quamodbus.cpp

HEADERS += \
quamodbus.h

FORMS += \
quamodbus.ui

include($$PWD/../src/widget/quamodbusclientwidget.pri)
include($$PWD/../libs/QDeferred.git/src/qlambdathreadworker.pri)
include($$PWD/../libs/QUaServer.git/src/wrapper/quaserver.pri)
include($$PWD/../libs/QUaServer.git/src/helper/add_qt_path_win.pri)