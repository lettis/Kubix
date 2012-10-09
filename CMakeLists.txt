cmake_minimum_required (VERSION 2.8)

project (Kubix)

set (Kubix_VERSION_MAJOR 1)
set (Kubix_VERSION_MINOR 0)

add_subdirectory(src)

add_custom_target(
  distclean
  COMMAND make clean
  COMMAND rm -r src/Kubix.dir
  COMMAND find -iname '*cmake*' -not -name CMakeLists.txt -exec rm -rf {} \\+
)

