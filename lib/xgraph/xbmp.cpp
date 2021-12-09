//
// Created by caiiiycuk on 25.06.2021.
//

#include <fstream>

#include <renderer/core/sdl_ext/SDL_extensions.h>

#include "xbmp.h"

renderer::core::Texture BMP_CreateTexture(const char *file, renderer::core::AbstractCoreRenderer *renderer) {
	return renderer::core::sdl_ext::texture_load_bmp(
		*renderer, 
		file, 
		renderer::core::TextureType::RGBA32, 
		renderer::core::BlendMode::Alpha
	);
}
