#include <iostream>
#include <cassert>

#include "SokolRenderer.h"
#include "HeightMap.h"

using namespace renderer;
using namespace renderer::scene;

HeightMap SokolRenderer::map_create(const MapDescription &map_description) {
	HeightMap map_rid =  map_storage.create(std::make_unique<sokol::HeightMap>(map_description));
	std::cout << "SokolRenderer::map_create() "
			  << "width="<<map_description.width
			  << ", height="<<map_description.height
			  << " => " << map_rid.id
	          << std::endl;

	return map_rid;
}

void SokolRenderer::map_destroy(HeightMap map_rid) {
	std::cout << "SokolRenderer::map_destroy"
			  << " map_rid: " << map_rid.id
	          << std::endl;
	auto& m = map_storage.getOrThrow(map_rid);
	m->destroy();
	map_storage.remove(map_rid);
}

void SokolRenderer::map_update_data(HeightMap map_rid, const Rect& rect, uint8_t *height, uint8_t *meta) {
	std::cout << "SokolRenderer::map_update"
			  << " map_rid: " << map_rid.id
	          << " , rect: " << rect
	          << std::endl;
	auto& m = map_storage.getOrThrow(map_rid);
	m->update_region(rect, height, meta);
}

void SokolRenderer::render(int32_t viewport_width, int32_t viewport_height, int32_t camera_pos_x, int32_t camera_pos_y, int32_t camera_pos_z) {
	std::cout << "SokolRenderer::SokolRenderer"
	<< " viewport_width: " << viewport_width
	<< " , viewport_height: " << viewport_height
	<< ", camera.x: " << camera_pos_x
	<< ", camera.y: " << camera_pos_y
	<< ", camera.z: " << camera_pos_z
	<< std::endl;
	for(auto& [rid, map]: map_storage){
		map->render(viewport_width, viewport_height, camera_pos_x, camera_pos_y, camera_pos_z);
	}
}

void SokolRenderer::map_update_palette(HeightMap map_rid, uint32_t *palette, int32_t palette_size)
{
	auto& m = map_storage.getOrThrow(map_rid);
	assert(palette_size == 256);
	m->update_palette(palette);
}

void SokolRenderer::map_query(HeightMap map_rid, int32_t *width, int32_t *height)
{
	auto& m = map_storage.getOrThrow(map_rid);
	const MapDescription& desc = m->map_decscription();
	if(width != nullptr){
		*width = desc.width;
	}

	if(height != nullptr){
		*height = desc.height;
	}
}

SokolRenderer::SokolRenderer()
	: map_storage(){
	std::cout << "SokolRenderer::SokolRenderer" << std::endl;
}

SokolRenderer::~SokolRenderer() = default;
