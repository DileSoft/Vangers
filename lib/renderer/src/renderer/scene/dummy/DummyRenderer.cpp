//
// Created by nikita on 30.11.2021.
//
#include <iostream>

#include "DummyRenderer.h"

using namespace renderer::scene;
using namespace renderer::scene::dummy;

HeightMap DummyRenderer::map_create(const MapDescription& map_description) {
	std::cout << "DummyRenderer::map_create"
	          << " => " << 0
	          << std::endl;
	return HeightMap{0};
}

void DummyRenderer::map_destroy(HeightMap map_rid) {
	std::cout << "DummyRenderer::map_destroy"
			  << " map_rid: " << map_rid.id
	          << std::endl;
}

void DummyRenderer::map_request_update(HeightMap map_rid, const Rect& rect) {
	std::cout << "DummyRenderer::map_request_update"
			  << " map_rid: " << map_rid.id
	          << ", rect: " << rect
			  << std::endl;
}

void DummyRenderer::render(const renderer::Rect &viewport, Camera camera)
{

}

Camera DummyRenderer::camera_create(const CameraDescription &camera_description)
{
	return Camera{0};
}

void DummyRenderer::camera_destroy(Camera camera)
{

}

void DummyRenderer::camera_set_transform(Camera camera, const Transform &transform)
{

}

void DummyRenderer::map_query(HeightMap map, int32_t *width, int32_t *height)
{

}

void DummyRenderer::map_update_palette(HeightMap map, uint32_t *palette, int32_t palette_size)
{

}

