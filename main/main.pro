include($$PWD/../preapp.pri)

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

APP_NAME = QUaModbusClient
TEMP_DIR = $$BUILD_TEMP_DIR/$${APP_NAME}

MOC_DIR = $$TEMP_DIR
UI_DIR  = $$TEMP_DIR
RCC_DIR = $$TEMP_DIR

CONFIG(debug, debug|release) {
	TARGET = $${APP_NAME}d
	OBJECTS_DIR = $$TEMP_DIR/debug
} else {
	TARGET = $${APP_NAME}
	OBJECTS_DIR = $$TEMP_DIR/release
}

include($$PWD/../libs/QUaServerWidgets.git/src/quaserverwidgets.pri)
include($$PWD/../src/widget/quamodbusclientwidget.pri)
include($$PWD/../libs/QDeferred.git/src/qlambdathreadworker.pri)
include($$PWD/../libs/QUaServer.git/src/wrapper/quaserver.pri)
include($$PWD/../libs/qadvanceddocking.pri)

SOURCES += \
main.cpp \
quamodbus.cpp

HEADERS += \
quamodbus.h

FORMS += \
quamodbus.ui

RESOURCES += $$PWD/../res/res.qrc

win32 {
	RC_ICONS = $$PWD/../res/logo/logo.ico
}
include($$PWD/../libs/add_qad_path_win.pri)

include($$PWD/../postapp.pri)