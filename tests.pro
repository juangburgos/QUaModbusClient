TEMPLATE = subdirs
CONFIG += debug_and_release build_all
# names
SUBDIRS = \
amalgamation \
qadvanceddocking \
01_console \
02_widget \
03_access_control
# directories
amalgamation.subdir      = $$PWD/libs/QUaServer.git/src/amalgamation
qadvanceddocking.subdir  = $$PWD/libs/QAdvancedDocking.git/src
01_console.subdir        = $$PWD/tests/01_console
02_widget.subdir         = $$PWD/tests/02_widget
03_access_control.subdir = $$PWD/tests/03_access_control
# dependencies
01_console.depends         = amalgamation
02_widget.depends          = amalgamation
03_access_control.depends  = amalgamation qadvanceddocking
