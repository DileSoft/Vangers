#include <iostream>
#include <cassert>

#include "SokolRenderer.h"
#include "SokolHeightMap.h"

using namespace renderer;
using namespace renderer::scene;

HeightMap SokolRenderer::map_create(const MapDescription &map_description) {
	HeightMap map_rid =  map_storage.create(std::make_unique<sokol::SokolHeightMap>(map_description));
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

void SokolRenderer::map_request_update(HeightMap map_rid, const Rect& region) {
	std::cout << "SokolRenderer::map_update"
			  << " map_rid: " << map_rid.id
			  << " , rect: " << region
	          << std::endl;
	auto& m = map_storage.getOrThrow(map_rid);
	m->request_update_region(region);
}

void SokolRenderer::render(const Rect &viewport, Camera camera_rid) {
	std::cout << "SokolRenderer::SokolRenderer"
	<< " viewport: " << viewport
	<< ", camera: " << camera_rid.id
	<< std::endl;

	auto& camera = camera_storage.getOrThrow(camera_rid);

	for(auto& [rid, map]: map_storage){
		const MapDescription& desc = map->map_decscription();
		hmm_mat4 transform = camera->transform_mat(desc);
		map->render(viewport, transform);
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

Camera SokolRenderer::camera_create(const CameraDescription &desc)
{
	return camera_storage.create(std::make_unique<sokol::SokolCamera>(desc));
}

void SokolRenderer::camera_destroy(Camera camera)
{
	camera_storage.remove(camera);
}

void SokolRenderer::camera_set_transform(Camera camera, const Transform &transform)
{
	auto& cam = camera_storage.getOrThrow(camera);
	cam->set_transform(transform);
}

SokolRenderer::~SokolRenderer() = default;
