TEMPLATE = subdirs

# names
SUBDIRS = \
amalgamation \
qadvanceddocking \
main \

# directories
amalgamation.subdir     = $$PWD/libs/QUaServer.git/src/amalgamation
qadvanceddocking.subdir = $$PWD/libs/QAdvancedDocking.git/src
main.subdir             = $$PWD/main

# dependencies
main.depends = amalgamation qadvanceddocking