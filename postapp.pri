win32 {
  # nothing to do here
}
# in case of linux, create deployment scripts
linux-g++ {
  # in case of linux, add post build commands to separate debug symbols
  load(resolve_target)
  TARGET_FULL = $$basename(QMAKE_RESOLVED_TARGET)
  TARGET_PATH = $$dirname(QMAKE_RESOLVED_TARGET)
  TARGET_PDB  = $${TARGET_FULL}.pdb
  QMAKE_POST_LINK += $$quote( cd "$${TARGET_PATH}"; objcopy --only-keep-debug "$${TARGET_FULL}" "$${TARGET_PDB}" $$escape_expand(\n\t))
  QMAKE_POST_LINK += $$quote( cd "$${TARGET_PATH}"; strip --strip-debug --strip-unneeded "$${TARGET_FULL}" $$escape_expand(\n\t))
  QMAKE_POST_LINK += $$quote( cd "$${TARGET_PATH}"; objcopy --add-gnu-debuglink="$${TARGET_PDB}" "$${TARGET_FULL}" $$escape_expand(\n\t))
}