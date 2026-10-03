SET(CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "Minimum macOS deployment version")
CMAKE_MINIMUM_REQUIRED(VERSION 4.0.0)

# root
PROJECT(vangers)

INCLUDE(CTest)

# modules
SET(RUSTYVANGERS_ROOT "${PROJECT_SOURCE_DIR}/vange-rs")
SET(RUSTYVANGERS_LOCK_PATCH "${PROJECT_SOURCE_DIR}/patches/vange-rs-local-wgpu-lock.patch")
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
include(ExternalDependency)

# options
OPTION(BINARY_SCRIPT "Use binary script for interface" ON)
OPTION(DIAGEN_TEST "Dialog generator and test subsystem" OFF)
# hide optons
OPTION(ACTINT "Actint user interface in road game" ON)

OPTION(FFMPEG "Build with ffmpeg" ON)
OPTION(ENABLE_ASAN "Native: compile and link with AddressSanitizer (Clang/GCC)" OFF)

SET(CMAKE_CONFIGURATION_TYPES "Release;Debug;MinSizeRel;RelWithDebInfo")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

ADD_DEFINITIONS(-DRV_API_3)

# depends
IF(EMSCRIPTEN)
	ADD_DEFINITIONS(-DEMSCRIPTEN)
	SET(FFMPEG OFF)
	SET(SDL3_LIBRARY "-sUSE_SDL=3")
	SET(SDL3_NET_LIBRARY "-sUSE_SDL_NET=3")
	SET(OGG_LIBRARY "-sUSE_OGG=1")
	SET(VORBIS_LIBRARY "-sUSE_VORBIS=1")
	SET(VORBISFILE_LIBRARY "-sUSE_VORBIS=1")
	SET(ZLIB_LIBRARIES "-sUSE_ZLIB=1")
	SET(RUSTYVANGERS_LIBRARY "${RUSTYVANGERS_ROOT}/target/wasm32-unknown-emscripten/release/librusty_vangers.a" CACHE FILEPATH "rusty-vangers Emscripten static library" FORCE)
	ADD_CUSTOM_TARGET(rusty_vangers
		COMMAND ${CMAKE_COMMAND} -DWORKING_DIRECTORY=${RUSTYVANGERS_ROOT} -DPATCH_FILE=${RUSTYVANGERS_LOCK_PATCH} -P ${PROJECT_SOURCE_DIR}/cmake/ApplyGitPatch.cmake
		COMMAND cargo build --manifest-path ${RUSTYVANGERS_ROOT}/Cargo.toml -p rusty-vangers --target wasm32-unknown-emscripten --release --config patch.crates-io.naga.path='${PROJECT_SOURCE_DIR}/wgpu/naga' --config patch.crates-io.wgpu-hal.path='${PROJECT_SOURCE_DIR}/wgpu/wgpu-hal' --config patch.crates-io.wgpu-naga-bridge.path='${PROJECT_SOURCE_DIR}/wgpu/wgpu-naga-bridge' --config patch.crates-io.wgpu-types.path='${PROJECT_SOURCE_DIR}/wgpu/wgpu-types'
		BYPRODUCTS ${RUSTYVANGERS_LIBRARY}
		WORKING_DIRECTORY ${RUSTYVANGERS_ROOT}
		COMMENT "Building rusty-vangers with cargo for wasm32-unknown-emscripten"
		VERBATIM
	)
	string(REPLACE "/cmake/Modules/Platform/Emscripten.cmake" "/cache/sysroot" EMSCRIPTEN_SYSROOT "${CMAKE_TOOLCHAIN_FILE}")
	SET(CLUNK_CMAKE_ARGS
		-DSDL3_INCLUDE_DIR=${EMSCRIPTEN_SYSROOT}/include/SDL3
		-DSDL3_LIBRARY_TEMP=${SDL3_LIBRARY}
		-DCMAKE_C_FLAGS=${SDL3_LIBRARY}
		-DCMAKE_CXX_FLAGS=${SDL3_LIBRARY}
	)
	add_compile_options(${SDL3_LIBRARY} ${SDL3_NET_LIBRARY} ${OGG_LIBRARY} ${VORBIS_LIBRARY} ${ZLIB_LIBRARIES} "-fwasm-exceptions")
ELSE(EMSCRIPTEN)
	FIND_PACKAGE(SDL3 3.2 CONFIG REQUIRED COMPONENTS SDL3)
	SET(SDL3_LIBRARY SDL3::SDL3)
	FIND_PACKAGE(SDL3_net 3.2 CONFIG REQUIRED)
	SET(SDL3_NET_LIBRARY SDL3::SDL3_net)
	FIND_PACKAGE(Ogg REQUIRED)
	FIND_PACKAGE(Vorbis REQUIRED)
	FIND_PACKAGE(VorbisFile REQUIRED)
	FIND_PACKAGE(ZLIB REQUIRED)
	SET(RUSTYVANGERS_LIBRARY "${RUSTYVANGERS_ROOT}/target/release/librusty_vangers.a" CACHE FILEPATH "rusty-vangers static library" FORCE)
	ADD_CUSTOM_TARGET(rusty_vangers
			COMMAND ${CMAKE_COMMAND} -DWORKING_DIRECTORY=${RUSTYVANGERS_ROOT} -DPATCH_FILE=${RUSTYVANGERS_LOCK_PATCH} -P ${PROJECT_SOURCE_DIR}/cmake/ApplyGitPatch.cmake
			COMMAND cargo build --manifest-path ${RUSTYVANGERS_ROOT}/Cargo.toml -p rusty-vangers --release --config patch.crates-io.naga.path='${PROJECT_SOURCE_DIR}/wgpu/naga' --config patch.crates-io.wgpu-hal.path='${PROJECT_SOURCE_DIR}/wgpu/wgpu-hal' --config patch.crates-io.wgpu-naga-bridge.path='${PROJECT_SOURCE_DIR}/wgpu/wgpu-naga-bridge' --config patch.crates-io.wgpu-types.path='${PROJECT_SOURCE_DIR}/wgpu/wgpu-types'
			BYPRODUCTS ${RUSTYVANGERS_LIBRARY}
			WORKING_DIRECTORY ${RUSTYVANGERS_ROOT}
			COMMENT "Building rusty-vangers with cargo"
			VERBATIM
	)
ENDIF(EMSCRIPTEN)

ExternalDependency(Clunk
	GIT_REPOSITORY https://github.com/stalkerg/clunk.git
	GIT_TAG d7d94324c7c2e869dc504ae6bd31dc2a07ccca2f
	TARGET Clunk::clunk
	CMAKE_ARGS -DWITH_SSE=OFF ${CLUNK_CMAKE_ARGS}
)

IF(FFMPEG)
	FIND_PACKAGE(FFMPEG REQUIRED)
ENDIF(FFMPEG)

IF(MSVC)
	# The committed vangers.res was produced by windres as a 32-bit COFF
	# object, which MSVC's x64 linker rejects (LNK1112). The icon is optional,
	# so MSVC builds without an embedded resource for now.
	SET(WINDOWS_RES "")
ELSE()
	set(WINDOWS_RES "../vangers.res")
ENDIF()

IF(BINARY_SCRIPT)
	ADD_DEFINITIONS(-D_BINARY_SCRIPT_)
ENDIF(BINARY_SCRIPT)

IF(ACTINT)
	ADD_DEFINITIONS(-DACTINT)
ENDIF(ACTINT)

IF(DIAGEN_TEST)
	ADD_DEFINITIONS(-DDIAGEN_TEST)
ENDIF(DIAGEN_TEST)

# globals
INCLUDE_DIRECTORIES(SYSTEM
	${ZLIB_INCLUDE_DIR}
	${FFMPEG_INCLUDE_DIRS}
	${CLUNK_INCLUDE_DIR}
	${OGG_INCLUDE_DIR}
	${VORBIS_INCLUDE_DIR}
	${VORBISFILE_INCLUDE_DIR}
	${STEAM_INCLUDE_DIR}
)

INCLUDE(CheckCXXCompilerFlag)
SET(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -Os -ftree-vectorize")
CHECK_CXX_COMPILER_FLAG("-msse2" HAVE_SIMDSSE)
IF(HAVE_SIMDSSE)
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -msse2")
ENDIF()
# For modern gcc
add_definitions(-D_USE_MATH_DEFINES)

IF(MSVC)
	# The sources contain UTF-8 (Russian) string literals. GCC/clang use UTF-8 as
	# the execution charset by default, so tell MSVC to do the same.
	add_compile_options("$<$<COMPILE_LANGUAGE:C,CXX>:/utf-8>")
	# Report the real __cplusplus value instead of the legacy 199711L.
	add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:/Zc:__cplusplus>")
	# A few translation units (actint, univang) are large enough to exceed the
	# default object file section limit otherwise.
	add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:/bigobj>")
	add_compile_options("$<$<COMPILE_LANGUAGE:C,CXX>:/W3>")
	# The visual-backend code (renderer::Transform / MapDescription call sites in
	# 3dobject.cpp, road.cpp, vmap.cpp) uses nested designated initializers.
	# GCC/clang accept them in C++17 as an extension; MSVC requires C++20.
	set(CMAKE_CXX_STANDARD 20)
	# Keep the POSIX CRT names (strdup, ...) and the classic CRT functions usable
	# without deprecation noise, and stop windows.h from defining min/max macros.
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
	add_definitions(-DNOMINMAX)
	# clunk's export_clunk.h marks every symbol __declspec(dllimport) whenever
	# _MSC_VER is defined, but the project links clunk as a static library, so
	# the linker would look for __imp_* entries. Override the macro with an
	# empty definition; the header then leaves CLUNKAPI as-is.
	add_definitions(-DCLUNKAPI=)
ELSE()
	add_compile_options(-Werror=shift-negative-value)
	add_compile_options(-Werror=dangling-else)
	add_compile_options(-Werror=parentheses)
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Werror=delete-non-virtual-dtor>)
	add_compile_options(-Werror=write-strings)
	add_compile_options(-fsigned-char)

	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		add_compile_options(-Werror=mismatched-new-delete )
	endif()
ENDIF()

IF(ENABLE_ASAN AND NOT EMSCRIPTEN)
	IF(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
		add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
		add_link_options(-fsanitize=address)
	ELSE()
		message(WARNING "ENABLE_ASAN: only Clang/GNU are wired; skipping for ${CMAKE_CXX_COMPILER_ID}")
	ENDIF()
ENDIF()

IF(WIN32)
	SET(WIN_DBG
		dbghelp
	)
ENDIF(WIN32)

# Get the current working branch
IF(NOT($ENV{GITHUB_REF_NAME}))
	execute_process(
		COMMAND git rev-parse --abbrev-ref HEAD
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_BRANCH
		COMMAND_ECHO STDOUT
		ECHO_OUTPUT_VARIABLE
		ECHO_ERROR_VARIABLE
		OUTPUT_STRIP_TRAILING_WHITESPACE)
ENDIF()

# Get the latest commit hash
IF(NOT($ENV{GITHUB_SHA}))
	execute_process(
		COMMAND git rev-parse HEAD
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_COMMIT_HASH
		COMMAND_ECHO STDOUT
		ECHO_OUTPUT_VARIABLE
		ECHO_ERROR_VARIABLE
		OUTPUT_STRIP_TRAILING_WHITESPACE)
ENDIF()

# subdirs
ADD_SUBDIRECTORY("src")
ADD_SUBDIRECTORY("lib/glad")
ADD_SUBDIRECTORY("lib/renderer")
ADD_SUBDIRECTORY("lib/xgraph")
ADD_SUBDIRECTORY("lib/xsound")
ADD_SUBDIRECTORY("lib/xtool")

if(NOT ANDROID)
	IF(BUILD_TESTING)
		ADD_SUBDIRECTORY("tests")
	ENDIF()
endif()
