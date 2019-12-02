TEMPLATE = subdirs

# names
SUBDIRS = \
amalgamation \
main \

# directories
amalgamation.subdir = $$PWD/libs/QUaServer.git/src/amalgamation
main.subdir         = $$PWD/main

# dependencies
main.depends         = amalgamation