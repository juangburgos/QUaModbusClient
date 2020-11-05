# get os info
win32 {
	contains(QT_ARCH, i386) {
	    OS_VER = win32
	} else {
	    OS_VER = win64
	}

	# Fix error [*tried to link win x86 libs to x64 target] in VS2017 (corrupted paths)
	INC_FIX = "$$(INCLUDE)"
	INC_FIX = $$split(INC_FIX, ;)
	INCLUDEPATH += $$PWD $$INC_FIX

	LIB_FIX = "$$(LIB)"
	LIB_FIX = $$split(LIB_FIX, ;)
	for(LIB_FIX_PATH, LIB_FIX) {
		LIBS += -L$$LIB_FIX_PATH
	}

}

linux-g++ {
	contains(QT_ARCH, i386) {
	    OS_VER = linux32
	} else {
	    OS_VER = linux64
	}
}

mac {
	OS_VER = mac64
}

BUILD_DIR      = $$PWD/build/$$OS_VER
BUILD_BIN_DIR  = $$BUILD_DIR/bin
BUILD_TEMP_DIR = $$BUILD_DIR/temp

CONFIG(debug, debug|release) {
    DESTDIR = $$BUILD_BIN_DIR/debug
} else {
    DESTDIR = $$BUILD_BIN_DIR/release
}

CONFIG += debug_and_release build_all plugin skip_target_version_ext exceptions
CONFIG -= flat

# create debug symbols for release builds
# NOTE : need to disable optimization, else dump files will point to incorrect source code lines
CONFIG *= force_debug_info
QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO -= -O2

# version variables
APP_VERSION = "1.0"
APP_VERSION_FILE = "$${DESTDIR}/version.txt"

# get git commit
win32 {
	# look for git
	GIT_BIN = $$system(where git)
	isEmpty(GIT_BIN) {
		error("Git not found. Cannot get current commit.")
	}
	else {
		message("Git found.")
	}
}
linux-g++ {
	# look for git
	GIT_BIN = $$system(which git)
	isEmpty(GIT_BIN) {
		error("Git not found. Cannot get current commit.")
	}
	else {
		message("Git found.")
	}
}
# get current commit
APP_BUILD = $$system(git rev-parse --short=8 HEAD)
isEmpty(APP_BUILD) {
	error("Could not get git commit.")
}
else {
	message("Git commit is $${APP_BUILD}.")
}

# version info as csv (xml wont work inside xml in vcxproj files)
APP_VERSION_INFO_1 = "VERSION=$${APP_VERSION}"
APP_VERSION_INFO_2 = "BUILD=$${APP_BUILD}"
# write to file
QMAKE_POST_LINK += $$quote(echo $${APP_VERSION_INFO_1} >  $${APP_VERSION_FILE}$$escape_expand(\n\t))
QMAKE_POST_LINK += $$quote(echo $${APP_VERSION_INFO_2} >> $${APP_VERSION_FILE}$$escape_expand(\n\t))
