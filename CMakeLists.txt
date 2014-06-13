cmake_minimum_required (VERSION 2.8)

project (Kubix)

set (Kubix_VERSION_MAJOR 0)
set (Kubix_VERSION_MINOR 1)

INCLUDE(InstallRequiredSystemLibraries)

#### enable compiler warnings	
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic -std=c++11")
add_subdirectory(src)

add_custom_target(
  distclean
  COMMAND make clean
  COMMAND rm -r src/Kubix.dir
  COMMAND find -iname '*cmake*' -not -name CMakeLists.txt -exec rm -rf {} \\+
  )

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Kubix")
SET(CPACK_PACKAGE_VENDOR "Florian Sittel, Carsten Burgard")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
SET(CPACK_PACKAGE_VERSION_MAJOR "${Kubix_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${Kubix_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "0")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
IF(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backslashes.
#  SET(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\Kubix.exe")
  SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} Kubix")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\github.com/lettis/Kubix")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\github.com/lettis/Kubix")
  SET(CPACK_NSIS_CONTACT "sittel@lettis.net")
  SET(CPACK_NSIS_MODIFY_PATH ON)
ELSE(WIN32 AND NOT UNIX)
  SET(CPACK_STRIP_FILES "Kubix")
  SET(CPACK_SOURCE_STRIP_FILES "")
ENDIF(WIN32 AND NOT UNIX)
SET(CPACK_PACKAGE_EXECUTABLES "Kubix")
INCLUDE(CPack)

