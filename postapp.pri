win32 {
  # nothing to do here
}
# in case of linux, create deployment scripts
linux-g++ {
  LINUX_TARGET     = $$TARGET
  LINUX_DEPLOYPATH = $$DESTDIR

  # make deploy dir if does not exists
  !exists( $$LINUX_DEPLOYPATH ) {
    !system(mkdir $$LINUX_DEPLOYPATH) {
      error(Cant create a directory $$LINUX_DEPLOYPATH)
    }
  }

  # copy linux runner to deploy dir if does not exist
  LINUX_RUNNERSCRIPT = LinuxRunner.sh
  LINUX_SCRIPTS_PATH = $$PWD/deploy/linux
  !exists( $$LINUX_DEPLOYPATH/$$LINUX_RUNNERSCRIPT ) {
    # test first if original script exists
    !exists( $$LINUX_SCRIPTS_PATH/$$LINUX_RUNNERSCRIPT ) {
      error(Linux runner script missing $$LINUX_SCRIPTS_PATH/$$LINUX_RUNNERSCRIPT)
    }
    # copy it to deploy dir
    !system(cp $$LINUX_SCRIPTS_PATH/$$LINUX_RUNNERSCRIPT $$LINUX_DEPLOYPATH/$$LINUX_RUNNERSCRIPT) {
      error(Cant copy $$LINUX_SCRIPTS_PATH/$$LINUX_RUNNERSCRIPT to $$LINUX_DEPLOYPATH/$$LINUX_RUNNERSCRIPT)
    }
  }

  # copy linux run script template
  LINUX_RUNTEMPLATE = LinuxRunTemplate.sh
  LINUX_RUNFILENAME = Run_$$LINUX_TARGET
  # test first if original template exists
  !exists( $$LINUX_SCRIPTS_PATH/$$LINUX_RUNTEMPLATE ) {
    error(Linux run template script missing $$LINUX_SCRIPTS_PATH/$$LINUX_RUNTEMPLATE)
  }
  # copy it to deploy dir
  !system(cp $$LINUX_SCRIPTS_PATH/$$LINUX_RUNTEMPLATE $$LINUX_DEPLOYPATH/$$LINUX_RUNFILENAME) {
    error(Cant copy $$LINUX_SCRIPTS_PATH/$$LINUX_RUNTEMPLATE to $$LINUX_DEPLOYPATH/$$LINUX_RUNFILENAME)
  }
  # append line to run application
  !system(printf "%s\ %s" $$LINUX_RUNNERSCRIPT $$LINUX_TARGET >> $$LINUX_DEPLOYPATH/$$LINUX_RUNFILENAME) {
    error(Cant append script line to $$LINUX_RUNFILENAME)
  }
  # finally make executable
  !system(chmod +x $$LINUX_DEPLOYPATH/$$LINUX_RUNFILENAME) {
    error(Cant make $$LINUX_RUNFILENAME executable)
  }
  # success message
  #message(Created Linux Run Script : $$LINUX_RUNFILENAME)

  # in case of linux, add post build commands to separate debug symbols
  load(resolve_target)
  TARGET_FULL = $$basename(QMAKE_RESOLVED_TARGET)
  TARGET_PATH = $$dirname(QMAKE_RESOLVED_TARGET)
  TARGET_PDB  = $${TARGET_FULL}.pdb
  QMAKE_POST_LINK += $$quote( cd "$${TARGET_PATH}"; objcopy --only-keep-debug "$${TARGET_FULL}" "$${TARGET_PDB}" $$escape_expand(\n\t))
  QMAKE_POST_LINK += $$quote( cd "$${TARGET_PATH}"; strip --strip-debug --strip-unneeded "$${TARGET_FULL}" $$escape_expand(\n\t))
  QMAKE_POST_LINK += $$quote( cd "$${TARGET_PATH}"; objcopy --add-gnu-debuglink="$${TARGET_PDB}" "$${TARGET_FULL}" $$escape_expand(\n\t))
}