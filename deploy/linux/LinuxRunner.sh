#!/bin/sh

# get exec dir (not called symlink dir)
dirname="$(dirname "$(readlink -f "$0")")"
# define required env vars
PATH=${PATH}:$dirname
export PATH
LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:$dirname
LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:$dirname/lib
LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:$dirname/..
LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:$dirname/../lib
export LD_LIBRARY_PATH
# execute application
cd $dirname
# make executable
chmod +x $dirname/"$1"
$dirname/"$1"