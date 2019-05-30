include($$PWD/../types/quamodbusclient.pri)

INCLUDEPATH += $$PWD/

FORMS += \
    $$PWD/quamodbusclientwidget.ui \
    $$PWD/quamodbusclientwidgetedit.ui \
    $$PWD/quamodbusclientdialog.ui \
    $$PWD/quamodbusdatablockwidgetedit.ui

HEADERS += \
    $$PWD/quamodbusclientwidget.h \
    $$PWD/quamodbusclientwidgetedit.h \
    $$PWD/quamodbusclientdialog.h \
    $$PWD/quamodbusdatablockwidgetedit.h

SOURCES += \
    $$PWD/quamodbusclientwidget.cpp \
    $$PWD/quamodbusclientwidgetedit.cpp \
    $$PWD/quamodbusclientdialog.cpp \
    $$PWD/quamodbusdatablockwidgetedit.cpp

