set(PACKAGE_VERSION "4.4.0")

if(PACKAGE_VERSION VERSION_LESS PACKAGE_FIND_VERSION)
  set(PACKAGE_VERSION_COMPATIBLE FALSE)
else()
  string(REGEX REPLACE "^([0-9]+).*" "\\1" _toml11_major "${PACKAGE_VERSION}")
  if(_toml11_major STREQUAL PACKAGE_FIND_VERSION_MAJOR)
    set(PACKAGE_VERSION_COMPATIBLE TRUE)
  endif()
  if(PACKAGE_FIND_VERSION STREQUAL PACKAGE_VERSION)
    set(PACKAGE_VERSION_EXACT TRUE)
  endif()
endif()

# No CMAKE_SIZEOF_VOID_P check here, unlike the vcpkg-generated version: this
# package is architecture independent by construction (headers only).
