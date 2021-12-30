//
// Created by nikita on 30.11.2021.
//
#include <iostream>

#include "DummyRenderer.h"

using namespace renderer::scene;
using namespace renderer::scene::dummy;

void DummyRenderer::map_create(const MapDescription& map_description) {
	std::cout << "DummyRenderer::map_create"
	          << " => " << 0
	          << std::endl;
}

void DummyRenderer::map_destroy() {
	std::cout << "DummyRenderer::map_destroy"
	          << std::endl;
}

void DummyRenderer::map_request_update(const Rect& rect) {
	std::cout << "DummyRenderer::map_request_update"
	          << ", rect: " << rect
			  << std::endl;
}

void DummyRenderer::render(const renderer::Rect &viewport)
{

}

void DummyRenderer::camera_create(const CameraDescription &camera_description)
{

}

void DummyRenderer::camera_destroy()
{

}

void DummyRenderer::camera_set_transform(const Transform &transform)
{

}

void DummyRenderer::map_update_palette(uint32_t *palette, int32_t palette_size)
{

}

