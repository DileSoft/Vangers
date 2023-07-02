INCLUDE_DIRECTORIES(AFTER
	"${PROJECT_SOURCE_DIR}/lib/xtool"
	"${PROJECT_SOURCE_DIR}/src"
)
	
SET(server_SRCS
	kdsplus.cpp
	${PROJECT_SOURCE_DIR}/src/xsocket.cpp
	${PROJECT_SOURCE_DIR}/lib/xtool/lang.cpp
	${PROJECT_SOURCE_DIR}/lib/xgraph/xgraph_dummy.cpp
	server.cpp
#win32f.cpp -- candidat to remove
)

ADD_EXECUTABLE(vangers_server ${server_SRCS})

IF(WIN32)
	SET(WIN_LIB 
		# wsock32
		ws2_32
	)
ENDIF(WIN32)

if(UNIX AND NOT APPLE)
	set(LINUX TRUE)
endif()

IF(NOT LINUX)
	SET(ICONV_LINK "-liconv")
ENDIF()

TARGET_LINK_LIBRARIES(vangers_server
	${SDL2_LIBRARY}
	${SDL2_NET_LIBRARY}
	${WIN_LIB}
	${ICONV_LINK}
	xtool
)
