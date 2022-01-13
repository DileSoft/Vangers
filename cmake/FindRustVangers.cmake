FIND_LIBRARY(RUST_VANGERS_LIBRARY
  NAMES
    vangers_ffi
  PATHS
    ${RUST_VANGERS_ROOT}/lib
    /usr/lib
	/usr/local/lib
	/opt/local/lib
	/sw/lib
	/local/lib
	/mingw/lib
	/local/bin
	/mingw/bin
	/usr/local/lib
)
IF(RUST_VANGERS_LIBRARY)
   SET(RUST_VANGERS_FOUND TRUE)
ENDIF(RUST_VANGERS_LIBRARY)

IF(RUST_VANGERS_FOUND)
  IF(NOT RUST_VANGERS_FIND_QUIETLY)
    MESSAGE(STATUS "Found vange-rs ffi library: ${RUST_VANGERS_LIBRARY}")
  ENDIF(NOT RUST_VANGERS_FIND_QUIETLY)
ELSE(RUST_VANGERS_FOUND)
  IF(RUST_VANGERS_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find vange-rs ffi library")
  ENDIF(RUST_VANGERS_FIND_REQUIRED)
ENDIF(RUST_VANGERS_FOUND)
