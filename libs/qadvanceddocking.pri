#CONFIG += c++14

# shared
win32 {
    CONFIG(debug, debug|release) {
        LIBS += -L$$PWD/QAdvancedDocking.git/lib
        LIBS += -lqtadvanceddockingd
    } else {
        LIBS += -L$$PWD/QAdvancedDocking.git/lib
        LIBS += -lqtadvanceddocking
    }
}

mac {
    LIBS += -L$${PWD}/QAdvancedDocking.git/lib/
    LIBS += -lqtadvanceddocking_debug
}
linux-g++ {
    LIBS += -L$${PWD}/QAdvancedDocking.git/lib/
    LIBS += -lqtadvanceddocking
}

INCLUDEPATH += $$PWD/QAdvancedDocking.git/src
DEPENDPATH  += $$PWD/QAdvancedDocking.git/src
