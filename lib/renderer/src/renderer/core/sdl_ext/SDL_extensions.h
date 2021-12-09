#ifndef LIB_RENDERER_SRC_RENDERER_CORE_SDL_EXT_SDL_EXTENSIONS
#define LIB_RENDERER_SRC_RENDERER_CORE_SDL_EXT_SDL_EXTENSIONS

#include <SDL.h>

#include "../../common.h"
#include "../AbstractCoreRenderer.h"

namespace renderer::core::sdl_ext {
	Texture texture_load_bmp(AbstractCoreRenderer& renderer, const char* filepath, TextureType texture_type, BlendMode blend_mode);
	Texture texture_from_sdl_surface(AbstractCoreRenderer& renderer, SDL_Surface* surface, TextureType texture_type, BlendMode blend_mode);
}
#endif /* LIB_RENDERER_SRC_RENDERER_CORE_SDL_EXT_SDL_EXTENSIONS */
