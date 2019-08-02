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
open62541.subdir         = $$PWD/../libs/QUaServer.git/src/amalgamation/open62541.pro
qadvanceddocking.subdir  = $$PWD/../libs/QUaAccessControl.git/libs/QAdvancedDocking.git/src/src.pro
01_console.subdir        = $$PWD/01_console/01_console.pro
02_widget.subdir         = $$PWD/02_widget/02_widget.pro
03_access_control.subdir = $$PWD/03_access_control/03_access_control.pro
# dependencies
01_console.depends         = open62541
02_widget.depends          = open62541
03_access_control.depends  = open62541 qadvanceddocking
