#!/bin/bash

app_name="QUaModbusClient"

# script dir (not calling or symlink dir)
dirname="$(dirname "$(readlink -f "$0")")"
# NOTE : $USER does not work if script execd with sudo
name_curruser=$(logname)

# function replaceAndCreateFile ***************************************************************
# replace text in file and write in another file
replaceAndCreateFile() {
	# Check arguments
	[[ $# -ne 4 ]] && { printf "\n[ERROR] missing arguments in replaceAndCreateFile\n\n" >&2; exit 1; }
	# Check source file exists
	if [[ ! -e $1 ]]; then
	    printf "\n[ERROR] source file for replaceAndCreateFile :\n%s \ndoes not exist!\n\n" $1
	    exit 1
	fi
	# Copy args
	filename_src=$1
	filename_targ=$2
	str_to_match=$3
	str_to_write=$4
	# Substitute
	while read a ; do 
	    echo ${a//$str_to_match/$str_to_write} ; 
	done < $filename_src > $filename_targ ; 
	# Success
    printf "[INFO] created :\n%s\n" $2 
}

# function replaceInFile ***************************************************************
# replace text in file and overwrite it
replaceInFile() {
	# Check arguments
	[[ $# -ne 3 ]] && { printf "\n[ERROR] missing arguments in replaceInFile\n\n" >&2; exit 1; }
	# Check source file exists
	if [[ ! -e $1 ]]; then
	    printf "\n[ERROR] source file for replaceInFile :\n%s \ndoes not exist!\n\n" $1
	    exit 1
	fi
	# Copy args
	filename_src=$1
	str_to_match=$2
	str_to_write=$3
	# Substitute
	while read a ; do 
	    echo ${a//$str_to_match/$str_to_write} ; 
	done < $filename_src > ${filename_src}.t ; 
	# Replace
	rm -f $filename_src
	mv ${filename_src}.t $filename_src
	# Success
    printf "[INFO] updated :\n%s\n" $1
}

# generate list of run script files
bin_dir="${dirname}/bin"
list_run=( $(find $bin_dir -name 'Run_*') )
# create shortcuts
targ_shortcuts=${dirname}/menu
for f in ${!list_run[@]}; do
	# get app name
	curr_runname=${list_run[$f]}
	curr_runname=${curr_runname//$bin_dir\//}  # strip $bin_dir/
	curr_appname=${curr_runname//Run_/} # strip Run_
	## generate desktop file
	curr_shortcut="$targ_shortcuts/$curr_appname.desktop"
	replaceAndCreateFile $dirname/Template.desktop $curr_shortcut "{UNKNOWN_BIN}" $curr_appname
    replaceInFile $curr_shortcut "{UNKNOWN_DIR}" $dirname
    chmod +x $curr_shortcut
    gio set $curr_shortcut "metadata::trusted" yes
done
# create shortcut folder (/menu) symlink to current user desktop 
ln -s ${targ_shortcuts} /home/$name_curruser/Desktop/${app_name}
ln -s ${targ_shortcuts} ~/Desktop/${app_name}
# create applications menu entries
menu_dirfile=$dirname/${app_name}.directory
replaceAndCreateFile $dirname/Template.directory $menu_dirfile "{UNKNOWN_DIR}" $dirname
replaceInFile $menu_dirfile "{UNKNOWN_BIN}" ${app_name}
# register all shortcuts
for f in ${!list_run[@]}; do
	# get shortcut name
	curr_runname=${list_run[$f]}
	curr_runname=${curr_runname//$bin_dir\//}  # strip $bin_dir/
	curr_appname=${curr_runname//Run_/} # strip Run_
	curr_shortcut="$targ_shortcuts/$curr_appname.desktop"
	xdg-desktop-menu install --novendor $menu_dirfile $curr_shortcut
done
