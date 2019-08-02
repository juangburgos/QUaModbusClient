QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = 03_access_control
TEMPLATE = app

# modbus
include($$PWD/../../src/widget/quamodbusclientwidget.pri)
include($$PWD/../../libs/QDeferred.git/src/qlambdathreadworker.pri)
include($$PWD/../../libs/QUaServer.git/src/wrapper/quaserver.pri)
include($$PWD/../../src/docks/quamodbusdocking.pri)
# access control
include($$PWD/../../libs/QUaAccessControl.git/src/types/quaaccesscontrol.pri)
include($$PWD/../../libs/QUaAccessControl.git/src/widgets/quaaccesscontrolwidgets.pri)
include($$PWD/../../libs/QUaAccessControl.git/src/docks/quaacdocking.pri)

SOURCES += \
main.cpp \        
    quamodbusaccesscontrol.cpp

HEADERS  += \ 
    quamodbusaccesscontrol.h

FORMS    += \ 
    quamodbusaccesscontrol.ui

include($$PWD/../../libs/QUaServer.git/src/helper/add_qt_path_win.pri)
