include($$PWD/../types/quamodbusclient.pri)

INCLUDEPATH += $$PWD/

FORMS += \
    $$PWD/quamodbusclientwidget.ui \
    $$PWD/quamodbusclientwidgetedit.ui \
    $$PWD/quamodbusclientdialog.ui \
    $$PWD/quamodbusdatablockwidgetedit.ui \
    $$PWD/quamodbusvaluewidgetedit.ui \
    $$PWD/quamodbusdatablockwidgetstatus.ui \
    $$PWD/quamodbusvaluewidgetstatus.ui

HEADERS += \
    $$PWD/quamodbusclientwidget.h \
    $$PWD/quamodbusclientwidgetedit.h \
    $$PWD/quamodbusclientdialog.h \
    $$PWD/quamodbusdatablockwidgetedit.h \
    $$PWD/quamodbusvaluewidgetedit.h \
    $$PWD/quamodbusdatablockwidgetstatus.h \
    $$PWD/quamodbusvaluewidgetstatus.h

SOURCES += \
    $$PWD/quamodbusclientwidget.cpp \
    $$PWD/quamodbusclientwidgetedit.cpp \
    $$PWD/quamodbusclientdialog.cpp \
    $$PWD/quamodbusdatablockwidgetedit.cpp \
    $$PWD/quamodbusvaluewidgetedit.cpp \
    $$PWD/quamodbusdatablockwidgetstatus.cpp \
    $$PWD/quamodbusvaluewidgetstatus.cpp

