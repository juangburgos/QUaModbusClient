#!/bin/bash
# Usage : ./CopyFiles.sh
# No arguments

app_name="QUaModbusClient"

# script dir (not calling or symlink dir)
dirname="$(dirname "$(readlink -f "$0")")"

# check os
uname_os="$(uname -s)"
case "${uname_os}" in
    Linux*)     machine=Linux;;
    Darwin*)    machine=Mac;;
    CYGWIN*)    machine=Win;;
    MINGW*)     machine=Win;;
	MSYS*)      machine=Win;;
    *)          machine="UNKNOWN:${uname_os}"
esac
echo "[INFO] os is ${machine}."

# check bitness
bits="64"
uname_bit="$(uname -m)"
if [[ $uname_bit -ne "x86_64" ]]; then
	bits="32"
fi
bitness=${bits}bits
echo "[INFO] bits is ${bitness}."

if [[ $machine == "Win" ]]; then
	PATH="${PATH}:/c/Program Files/Git/bin"
	if [[ $bits == "64" ]]; then
		PATH="${PATH}:/c/Qt/Qt5.13.2/5.13.2/msvc2017_64/bin"
		path_source_app="${dirname}/../build/win64/bin/release/"
		path_source_lib="${dirname}/../libs/QAdvancedDocking.git/lib/qtadvanceddocking.dll"
	else
		#PATH="${PATH}:/c/Qt/Qt5.6.3/5.6.3/msvc2013/bin"
		#path_source_app="${dirname}/../build/win32/bin/release/"
		echo "[TODO] 32bits not supported"
		exit 1
	fi
	PATH="${PATH}:/c/Qt/QtIFW-2.0.5/bin"
	PATH="${PATH}:/c/Qt/QtIFW-3.1.1/bin"
else
	if [[ $bits == "64" ]]; then
		PATH="/opt/Qt5.12.1/5.12.1/gcc_64/bin:${PATH}"
		PATH="/opt/Qt5.13.2/5.13.2/gcc_64/bin:${PATH}"
		path_source_app="${dirname}/../build/linux64/bin/release/"
		path_source_lib="${dirname}/../libs/QAdvancedDocking.git/lib/"
		path_source_app_icons="${dirname}/../res/logo/"
	else
		#path_source_app="${dirname}/../build/Linux32/bin/release/"
		echo "[TODO] 32bits not supported"
		exit 1
	fi
	PATH="${PATH}:/home/${USER}/Qt/QtIFW-2.0.5/bin"
	PATH="${PATH}:/home/${USER}/Qt/QtIFW-3.1.1/bin"
	PATH="${PATH}:/root/Qt/QtIFW-3.1.1/bin"
fi
echo "[INFO] will copy binaries from ${path_source_app}"

# check path_source_app exists
if [[ ! -d ${path_source_app} ]]; then
    printf "\n[ERROR] source binaries path :\n%s \ndoes not exist" ${path_source_app}
    exit 1
fi

# function checkDependencies ***************************************************************
checkDependencies() {
	# check at least one arg
	[[ $# -lt 1 ]] && { printf "\n[ERROR] missing arguments in checkDependencies\n\n" >&2; exit 1; }
	# get args as array
	list_args=("$@")
	# check arg is array
	#declare -p list_args 2> /dev/null | grep -q '^declare \-a' && echo array || echo notarray
	# loop
	for depend in ${!list_args[@]}; do
	    curr_depend=${list_args[$depend]}
	    #printf "%s\n" $curr_depend
	    if ! type $curr_depend > /dev/null 2>&1; then
	    	printf "\n[ERROR] %s dependency not found.\n\n" $curr_depend
			exit 1
		fi
	done
}

# function checkAndCopy *************************************************************
# Check and Copy Function
checkAndCopy() {
	# Check arguments
	[[ $# -ne 2 ]] && { printf "\n[ERROR] missing arguments in checkAndCopy\n\n" >&2; exit 1; }
	# Check source file exists
	if [[ ! -e $1 ]]; then
	    printf "\n[ERROR] source file :\n%s \ndoes not exist!\n\n" $1
	    exit 1
	fi
	# Copy
	yes | cp -rf $1 $2
	# Success
    printf "[INFO] copied :\n%s to \n%s\n" $1 $2 
}

# function main *****************************************************************************
# create list of necessary dependencies
declare -a list_depends
list_depends+=( "git"          )
list_depends+=( "binarycreator")
list_depends+=( "qmake"        )
if [[ $machine == "Win" ]]; then
	list_depends+=( "windeployqt"   )
else
	list_depends+=( "linuxdeployqt" )
fi

# check they exist in system
checkDependencies "${list_depends[@]}"

# target paths
if [[ $machine == "Win" ]]; then
	path_target_root="${dirname}/win/packages/${app_name}/data/"
	path_target_bin="${path_target_root}bin/"
else
	path_target_root="${dirname}/linux/packages/${app_name}/data/"
	path_target_bin="${path_target_root}bin/"
	path_target_icons="${path_target_root}icons/"
	# need to add target path to library path so linuxdeployqt finds portal deps
	LD_LIBRARY_PATH=$path_target_bin:$LD_LIBRARY_PATH
	export LD_LIBRARY_PATH
fi

# clean untracked files
git clean -f -x -d

# copy style files
echo "[INFO] searching style files."
# get list of styles in source
list_styles=( $(find ${dirname}/../ -name '*.qss') )	
# copy all styles files to target
for f in ${!list_styles[@]}; do
    curr_fname=${list_styles[$f]}
    curr_bname="$(basename "${curr_fname}")"
    curr_tname="$path_target_bin$curr_bname"
    checkAndCopy $curr_fname $curr_tname
done
echo "[INFO] finished copying style files"

# get list of app files in source
if [[ $machine == "Win" ]]; then
	# *.exe files
	list_apps=( $(find ${path_source_app} -name '*.exe') )	
else
	# files marked as executable
	list_apps=( $( file $(find ${path_source_app} -type f) | grep "LSB executable" | grep -o -P '.*(?=:)' | grep -v ".pdb") )
fi
# copy all app files to target
for f in ${!list_apps[@]}; do
    curr_fname=${list_apps[$f]}
    curr_bname="$(basename "${curr_fname}")"
    curr_tname="$path_target_bin$curr_bname"
    # copy
	if [[ $machine == "Win" ]]; then
    	checkAndCopy $curr_fname $curr_tname
	else
		# note : asume binaries with no symbols, else need to strip them
		#strip --strip-debug --strip-unneeded $curr_fname -o $curr_tname
		checkAndCopy $curr_fname $curr_tname
		# copy run scripts
		checkAndCopy "${path_source_app}Run_${curr_bname}" "${path_target_bin}Run_${curr_bname}"
	fi
done

# Version file (About dialog)
curr_fname=${path_source_app}version.txt
curr_bname="$(basename "${curr_fname}")"
curr_tname="$path_target_bin$curr_bname"
checkAndCopy $curr_fname $curr_tname

# linux only
if [[ $machine == "Linux" ]]; then
	# copy bootstrap script
	checkAndCopy ${dirname}/linux/LinuxBootstrap.sh ${path_target_root}LinuxBootstrap.sh
	chmod +x ${path_target_root}LinuxBootstrap.sh
	# copy linux runner script to /bin
	checkAndCopy ${dirname}/linux/LinuxRunner.sh ${path_target_bin}LinuxRunner.sh
	chmod +x ${path_target_bin}LinuxRunner.sh
	# copy logo
	checkAndCopy "${path_source_app_icons}logo.png" "${path_target_icons}${app_name}.png"
	# copy shortcut template files
	checkAndCopy ${dirname}/linux/Template.desktop ${path_target_root}Template.desktop
	checkAndCopy ${dirname}/linux/Template.directory ${path_target_root}Template.directory
fi

# copy dependencies
if [[ $machine == "Win" ]]; then
	# *.dll files
	curr_fname="${path_source_lib}"
    curr_bname="$(basename "${curr_fname}")"
    curr_tname="$path_target_bin$curr_bname"
    checkAndCopy $curr_fname $curr_tname
else
	# *.so files
	list_sos=( $(find ${path_source_lib}/../ -name '*qtadvanceddocking.so*') )	
	# copy all styles files to target
	for f in ${!list_sos[@]}; do
	    curr_fname=${list_sos[$f]}
	    curr_bname="$(basename "${curr_fname}")"
	    curr_tname="$path_target_bin$curr_bname"
	    checkAndCopy $curr_fname $curr_tname
	done
fi

# get list of app and lib files in target
if [[ $machine == "Win" ]]; then
	list_apps=( $(find ${path_target_bin} -name '*.exe') )
	list_libs=( $(find ${path_target_bin} -name '*.dll') )
else
	list_apps=( $( file $(find ${path_target_bin} -type f) | grep "LSB executable" | grep -o -P '.*(?=:)' | grep -v ".pdb") )
	list_libs=( $(find ${path_target_bin} -name '*.so') )
fi
# join lists
list_bins=( "${list_apps[@]}" "${list_libs[@]}")
length_bins=${#list_bins[@]}
# get dependencies for all binaries using windeployqt/linuxdeployqt
# it is one big statement containing all binaries
if [[ $machine == "Win" ]]; then
	deploy_cmd="windeployqt --release --no-styless --no-compiler-runtime"
else
	deploy_cmd="linuxdeployqt"
fi
for f in ${!list_bins[@]}; do
	# construct one big statement
    curr_fname=${list_bins[$f]}
    if [[ $machine == "Win" ]]; then
    	deploy_cmd="$deploy_cmd $curr_fname"
	else
		if [ $f -eq "0" ]; then
		    deploy_cmd="$deploy_cmd $curr_fname"
		else
		    deploy_cmd="$deploy_cmd -executable=$curr_fname"
		fi
	fi
done
# execute big statement
if [[ $machine == "Win" ]]; then
	echo $deploy_cmd
	$deploy_cmd
else
	deploy_cmd="$deploy_cmd -no-styless -bundle-non-qt-libs -extra-plugins=imageformats/libqsvg.so,iconengines/libqsvgicon.so"
	echo $deploy_cmd
	$deploy_cmd
	# cleanup
	rm $(find ${PWD} -name '*AppRun*')
fi
echo "[INFO] finished getting binary dependencies"

# target path for package_info.json
path_package_info="${path_target_root}package_info.json"
# get package creator
package_creator=$(git config user.name)
# get current branch
package_branch=$(git rev-parse --abbrev-ref HEAD)
# get current commit
package_commit=$(git rev-parse --short=8 HEAD)
# get current date
package_date=$(date +%Y%m%d_%H%M)
# get qt info (split by line, get second line)
qt_info=$(qmake -v)
IFS=$'\n' list_qt_info=($qt_info)
package_qtver=${list_qt_info[1]}
# remove json file if exists
if [[ -e $path_package_info ]]; then
	rm -f $path_package_info
fi
# create json to be printed in file
printf '{\n\t"package_creator": "%s",\n\t"package_branch": "%s",\n\t"package_commit": "%s",\n\t"package_date": "%s",\n\t"package_qtver": "%s",\n\t"package_arch": "%s"\n}\n' "$package_creator" "$package_branch" "$package_commit" "$package_date" "$package_qtver" "$bitness" >> $path_package_info
# create installer appending current commit
echo "[INFO] creating installer please wait..."
if [[ $machine == "Win" ]]; then
	install_config=${dirname}/win/config/config.xml
	install_package=${dirname}/win/packages
	installer_name=${app_name}_${machine}${bitness}_${package_commit}.exe
else
	install_config=${dirname}/linux/config/config.xml
	install_package=${dirname}/linux/packages
	installer_name=${app_name}_${machine}${bitness}_${package_commit}.run
fi
binarycreator --offline-only -c ${install_config} -p ${install_package} ${installer_name}
#clear;
if [[ $machine == "Linux" ]]; then
	chmod +x ${installer_name}
fi
echo "[INFO] successfully created ${installer_name}"
