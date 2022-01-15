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
	src/renderer/compositor/AbstractCompositor.h
	src/renderer/compositor/CompositorException.cpp
	src/renderer/compositor/CompositorException.h
	src/renderer/compositor/sokol/SokolCompositor.cpp
	src/renderer/compositor/sokol/SokolCompositor.h
	src/renderer/compositor/sokol/SokolTexture.cpp
	src/renderer/compositor/sokol/SokolTexture.h
	src/renderer/compositor/gles3/GLES3Compositor.cpp
	src/renderer/compositor/gles3/GLES3Compositor.h
	src/renderer/compositor/gles3/Shader.cpp
	src/renderer/compositor/gles3/Shader.h
	src/renderer/compositor/gles3/QuadVertexArray.cpp
	src/renderer/compositor/gles3/QuadVertexArray.h
	src/renderer/compositor/gles3/GLES3Texture.cpp
	src/renderer/compositor/gles3/GLES3Texture.h
	src/renderer/compositor/sdl_ext/SDL_extensions.cpp
	src/renderer/compositor/sdl_ext/SDL_extensions.h
	src/renderer/scene/AbstractRenderer.h
	src/renderer/scene/RenderingContext.cpp
	src/renderer/scene/RenderingContext.h
	src/renderer/scene/sokol/SokolRenderer.cpp
	src/renderer/scene/sokol/SokolRenderer.h
	src/renderer/scene/sokol/SokolHeightMap.cpp
	src/renderer/scene/sokol/SokolHeightMap.h
	src/renderer/scene/sokol/SokolCamera.h
	src/renderer/scene/sokol/SokolCamera.cpp
	src/renderer/scene/sokol/MapUpdater.h
	src/renderer/scene/sokol/MapUpdater.cpp
	src/renderer/scene/dummy/DummyRenderer.cpp
	src/renderer/scene/dummy/DummyRenderer.h
	src/renderer/scene/rust/RustRenderer.cpp
	src/renderer/scene/rust/RustRenderer.h
	src/renderer/scene/rust/vange_rs.h
	)


ADD_LIBRARY(renderer STATIC ${renderer_SRCS})

TARGET_INCLUDE_DIRECTORIES(
	renderer 
	INTERFACE "${PROJECT_SOURCE_DIR}/lib/glad/include"
)

SET(RENDERER_LIBS
	${RUSTVANGERS_LIBRARY}
	glad
	)

IF(UNIX)
    SET(RENDERER_LIBS
		${RENDERER_LIBS}
		dl
		pthread
		)
ENDIF(UNIX)
IF(MINGW)
SET(RENDERER_LIBS
	${RENDERER_LIBS}
		userenv
		bcrypt
		d3d12
		d3dcompiler
		ws2_32
		)
ENDIF(MINGW)

TARGET_LINK_LIBRARIES(
	renderer
	${RENDERER_LIBS}
)
