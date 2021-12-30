#include <iostream>
#include <cassert>

#include "SokolRenderer.h"
#include "SokolHeightMap.h"

using namespace renderer;
using namespace renderer::scene;

void SokolRenderer::map_create(const MapDescription &map_description) {
	map = std::make_unique<sokol::SokolHeightMap>(map_description);
	std::cout << "SokolRenderer::map_create() "
			  << "width="<<map_description.width
			  << ", height="<<map_description.height
	          << std::endl;
}

void SokolRenderer::map_destroy() {
	std::cout << "SokolRenderer::map_destroy"
	          << std::endl;
	map.reset();
}

void SokolRenderer::map_request_update(const Rect& region) {
	std::cout << "SokolRenderer::map_update"
			  << " , rect: " << region
	          << std::endl;
	map->request_update_region(region);
}

void SokolRenderer::render(const Rect &viewport) {
	std::cout << "SokolRenderer::SokolRenderer"
	<< " viewport: " << viewport
	<< std::endl;

	const MapDescription& desc = map->map_decscription();
	hmm_mat4 transform = camera->transform_mat(desc);
	map->render(viewport, transform);
}

void SokolRenderer::map_update_palette(uint32_t *palette, int32_t palette_size)
{
	assert(palette_size == 256);
	map->update_palette(palette);
}

SokolRenderer::SokolRenderer(){
	std::cout << "SokolRenderer::SokolRenderer" << std::endl;
}

void SokolRenderer::camera_create(const CameraDescription &desc)
{
	camera = std::make_unique<sokol::SokolCamera>(desc);
}

void SokolRenderer::camera_destroy()
{
	camera.reset();
}

void SokolRenderer::camera_set_transform(const Transform &transform)
{
	camera->set_transform(transform);
}

SokolRenderer::~SokolRenderer() = default;
