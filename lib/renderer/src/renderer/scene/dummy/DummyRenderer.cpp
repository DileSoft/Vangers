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
	return {0};
}

void DummyRenderer::map_destroy(HeightMap map_rid) {
	std::cout << "DummyRenderer::map_destroy"
			  << " map_rid: " << map_rid.id
	          << std::endl;
}

void DummyRenderer::map_update_data(HeightMap map_rid, const Rect& rect, uint8_t *height, uint8_t *meta) {
	std::cout << "DummyRenderer::map_update"
			  << " map_rid: " << map_rid.id
	          << ", rect: " << rect
			  << ", height: " << height
			  << ", meta: " << meta
	          << std::endl;
}

void DummyRenderer::render(int32_t viewport_width, int32_t viewport_height, int32_t camera_pos_x, int32_t camera_pos_y, int32_t camera_pos_z) {
	std::cout << "DummyRenderer::SokolRenderer"
	          << " viewport_width: " << viewport_width
	          << ", viewport_height: " << viewport_height
	          << std::endl;
}
