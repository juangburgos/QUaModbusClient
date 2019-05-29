QT += core serialbus serialport network xml

CONFIG += c++11

INCLUDEPATH += $$PWD/

HEADERS += \
	$$PWD/quamodbusclientlist.h \
	$$PWD/quamodbusclient.h \
	$$PWD/quamodbustcpclient.h \
	$$PWD/quamodbusrtuserialclient.h \
	$$PWD/quamodbusdatablocklist.h \
	$$PWD/quamodbusdatablock.h \
	$$PWD/quamodbusvaluelist.h \
	$$PWD/quamodbusvalue.h

SOURCES += \
	$$PWD/quamodbusclientlist.cpp \
	$$PWD/quamodbusclient.cpp \
	$$PWD/quamodbustcpclient.cpp \
	$$PWD/quamodbusrtuserialclient.cpp \
	$$PWD/quamodbusdatablocklist.cpp \
	$$PWD/quamodbusdatablock.cpp \
	$$PWD/quamodbusvaluelist.cpp \
	$$PWD/quamodbusvalue.cpp
