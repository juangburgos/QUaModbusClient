QT += core serialbus serialport network
QT -= gui

CONFIG += c++11

TARGET = QUaModbusClient
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

INCLUDEPATH += $$PWD/

SOURCES += main.cpp

HEADERS += \
	quamodbusclientlist.h \
	quamodbusclient.h \
	quamodbustcpclient.h \
	quamodbusrtuserialclient.h \
	quamodbusdatablocklist.h \
	quamodbusdatablock.h

SOURCES += \
	quamodbusclientlist.cpp \
	quamodbusclient.cpp \
	quamodbustcpclient.cpp \
	quamodbusrtuserialclient.cpp \
	quamodbusdatablocklist.cpp \
	quamodbusdatablock.cpp

include($$PWD/../libs/QDeferred.git/src/qlambdathreadworker.pri)
include($$PWD/../libs/QUaServer.git/src/wrapper/quaserver.pri)
include($$PWD/../libs/QUaServer.git/src/helper/add_qt_path_win.pri)