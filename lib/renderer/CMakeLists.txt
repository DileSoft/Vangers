FIND_PACKAGE(OpenGL REQUIRED)
# For sdl_ext only
FIND_PACKAGE(SDL2 REQUIRED)


SET(renderer_SRCS
	src/renderer/lib/HandmadeMath.h
	src/renderer/lib/HandmadeMath_impl.c
	src/renderer/lib/sokol_gfx.h
	src/renderer/lib/sokol_gfx_impl.cpp
	src/renderer/lib/sokol_gfx_settings.h
	src/renderer/lib/sokol_gfx_ext.h
	src/renderer/ResourceStorage.h
	src/renderer/ResourceId.h
	src/renderer/common.h
	src/renderer/common.cpp
	src/renderer/core/AbstractCoreRenderer.h
	src/renderer/core/sokol/SokolCoreRenderer.cpp
	src/renderer/core/sokol/SokolCoreRenderer.h
	src/renderer/core/sokol/SokolTexture.cpp
	src/renderer/core/sokol/SokolTexture.h
	src/renderer/core/sdl_ext/SDL_extensions.cpp
	src/renderer/core/sdl_ext/SDL_extensions.h
	src/renderer/scene/AbstractRenderer.h
	src/renderer/scene/RenderingContext.cpp
	src/renderer/scene/RenderingContext.h
	src/renderer/scene/util/MapUpdater.h
	src/renderer/scene/util/MapUpdater.cpp
	src/renderer/scene/sokol/SokolRenderer.cpp
	src/renderer/scene/sokol/SokolRenderer.h
	src/renderer/scene/sokol/HeightMap.cpp
	src/renderer/scene/sokol/HeightMap.h
	src/renderer/scene/sokol/SokolCamera.h
	src/renderer/scene/sokol/SokolCamera.cpp
	src/renderer/scene/dummy/DummyRenderer.cpp
	src/renderer/scene/dummy/DummyRenderer.h
	)


ADD_LIBRARY(renderer STATIC ${renderer_SRCS})
TARGET_LINK_LIBRARIES(
	renderer
	${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${SDL2_LIBRARY}
)
