QT += core gui

CONFIG += c++11

# N/A already included elsewhere
#include($$PWD/../../libs/QUaAccessControl.git/libs/qadvanceddocking.pri)

INCLUDEPATH += $$PWD/

HEADERS += \
    $$PWD/quamodbusdockwidgets.hpp
