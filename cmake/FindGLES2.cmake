find_library(GLES2_LIBRARY
	NAMES
	    GLESv2
		libGLESv2.dll
	PATHS
	    ${GLES2_ROOT}/lib
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

IF(GLES2_LIBRARY)
    SET(GLES2_FOUND TRUE)
ENDIF(GLES2_LIBRARY)

IF(GLES2_FOUND)
    IF(NOT GLES2_FIND_QUIETLY)
	    MESSAGE(STATUS "Found GLES2: ${GLES2_LIBRARY} ${GLES2_INCLUDE_DIR}")
	ENDIF(NOT GLES2_FIND_QUIETLY)
ELSE(GLES2_FOUND)
    IF(GLES2_FIND_REQUIRED)
	    MESSAGE(FATAL_ERROR "Could not find GLES2")
	ENDIF(GLES2_FIND_REQUIRED)
ENDIF(GLES2_FOUND)

