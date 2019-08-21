#CONFIG += c++14

# shared
CONFIG(debug, debug|release) {
    win32 {
        LIBS += -L$$PWD/QAdvancedDocking.git/lib
        LIBS += -lqtadvanceddockingd
    }
    mac {
        SO_FIND = 1
        DOCKLIB_PATH = $$system(dirname $(readlink -f $(find $${OUT_PWD}/../../ -type f -name "*docking.so*")), ,SO_FIND)
        equals(SO_FIND, 0) {
            message("Found docking library in $${DOCKLIB_PATH}.")
        }
        else {
            error("Failed to find docking library (qadvanceddocking.pri).")
        }
        LIBS += -L$${DOCKLIB_PATH}
        LIBS += -lqtadvanceddocking_debug
    }
    linux-g++ {
        SO_FIND = 1
        DOCKLIB_PATH = $$system(dirname $(readlink -f $(find $${OUT_PWD}/../../ -type f -name "*docking.so*")), ,SO_FIND)
        equals(SO_FIND, 0) {
            message("Found docking library in $${DOCKLIB_PATH}.")
        }
        else {
            error("Failed to find docking library (qadvanceddocking.pri).")
        }
        LIBS += -L$${DOCKLIB_PATH}
        LIBS += -lqtadvanceddocking
    }
} else {
    LIBS += -lqtadvanceddocking
}

INCLUDEPATH += $$PWD/QAdvancedDocking.git/src
DEPENDPATH  += $$PWD/QAdvancedDocking.git/src
