TEMPLATE = subdirs
CONFIG += debug_and_release build_all
# names
SUBDIRS = \
open62541 \
console \
widget
# directories
open62541.subdir = $$PWD/../libs/QUaServer.git/src/amalgamation/open62541.pro
console.subdir   = $$PWD/console/console.pro
widget.subdir    = $$PWD/widget/widget.pro
# dependencies
console.depends = open62541
widget.depends  = open62541



