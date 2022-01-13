//
// Created by nikita on 05.12.2021.
//

#include <cassert>

// TODO: manage library includes and defines properly
#include "../../lib/sokol_gfx.h"

#include "SokolCompositor.h"
#include "SokolTexture.h"

using namespace renderer;
using namespace renderer::compositor;
using namespace renderer::compositor::sokol;


Texture SokolCompositor::texture_create(int32_t width, int32_t height, TextureType texture_type, BlendMode blend_mode) {
	return _texture_storage.create(SokolTexture::create(texture_type, blend_mode, width, height));
}

void SokolCompositor::texture_set_data(Texture texture, uint8_t *data) {
	auto& pTexture = _texture_storage.getOrThrow(texture);
	pTexture->set_data(data);
}

void SokolCompositor::texture_render(Texture texture, const Rect &src_rect, const Rect &dst_rect) {
	auto& pTexture = _texture_storage.getOrThrow(texture);
	pTexture->render(_screen_width, _screen_height, src_rect, dst_rect);
}



void SokolCompositor::texture_destroy(Texture texture) {
	auto& t = _texture_storage.getOrThrow(texture);
	t->destroy();
	_texture_storage.remove(texture);
}

SokolCompositor::SokolCompositor(int32_t screenWidth, int32_t screenHeight)
	: _screen_width(screenWidth)
	, _screen_height(screenHeight)
	, _pass_action()
	{}

void SokolCompositor::render_begin() {
	assert(_pass_action && "renderer is not initialized");

	sg_begin_default_pass(*_pass_action, (int)_logical_screen_width, (int)_logical_screen_height);
}

void SokolCompositor::render_present() {
	sg_end_pass();
	sg_commit();
}

void SokolCompositor::initialize() {
	sg_setup(sg_desc{});
	assert(sg_isvalid());
	_pass_action = std::make_unique<sg_pass_action>(sg_pass_action{
		.colors = {
			/* [0] =*/ {
				.action=SG_ACTION_DONTCARE,
			}
		}
	});
}

void SokolCompositor::dispose() {
	for(auto& [rid, t]: _texture_storage){
		texture_destroy(rid);
	}

	sg_shutdown();
}

void SokolCompositor::texture_query(Texture texture, int32_t *width, int32_t *height, TextureType *texture_type,
									  BlendMode *blend_mode) {
	auto& t = _texture_storage.getOrThrow(texture);
	if(texture_type != nullptr){
		*texture_type = t->texture_type();
	}

	if(blend_mode != nullptr){
		*blend_mode = t->blend_mode();
	}

	if(width != nullptr){
		*width = t->width();
	}

	if(height != nullptr){
		*height = t->height();
	}
}

void SokolCompositor::texture_set_color(Texture texture, const Color &color){
	auto& t = _texture_storage.getOrThrow(texture);
	t->set_color(color);
}

void SokolCompositor::query_output_size(int32_t *width, int32_t *height) {
	if(width != nullptr){
		*width = _screen_width;
	}

	if(height != nullptr){
		*height = _screen_height;
	}
}

void SokolCompositor::set_physical_screen_size(int32_t width, int32_t height)
{
	_screen_width = width;
	_screen_height = height;
}

void SokolCompositor::set_logical_screen_size(int32_t width, int32_t height) {
	_logical_screen_width = width;
	_logical_screen_height = height;
}

SokolCompositor::~SokolCompositor() = default;

void SokolCompositor::read_pixels(uint8_t *){
	// TODO:
	throw RendererException("SokolCoreRenderer::read_pixels is not implemented");
}
