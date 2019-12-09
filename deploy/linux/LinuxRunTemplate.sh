#!/bin/sh
# get exec dir (not called symlink dir)
dirname="$(dirname "$(readlink -f "$0")")"
# make executable
chmod +x $dirname/LinuxRunner.sh
# execute
$dirname/