//
// Created by caiiiycuk on 25.06.2021.
//

#ifndef VANGERS_XBMP_H
#define VANGERS_XBMP_H

#include <SDL.h>
#include <renderer/core/AbstractCoreRenderer.h>

renderer::core::Texture BMP_CreateTexture(const char *file, renderer::core::AbstractCoreRenderer *renderer);

#endif // VANGERS_XBMP_H
