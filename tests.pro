TEMPLATE = subdirs
CONFIG += debug_and_release build_all
# names
SUBDIRS = \
open62541 \
qadvanceddocking \
01_console \
02_widget \
03_access_control
# directories
open62541.subdir         = $$PWD/libs/QUaServer.git/src/amalgamation
qadvanceddocking.subdir  = $$PWD/libs/QUaAccessControl.git/libs/QAdvancedDocking.git/src
01_console.subdir        = $$PWD/tests/01_console
02_widget.subdir         = $$PWD/tests/02_widget
03_access_control.subdir = $$PWD/tests/03_access_control
# dependencies
01_console.depends         = open62541
02_widget.depends          = open62541
03_access_control.depends  = open62541 qadvanceddocking
