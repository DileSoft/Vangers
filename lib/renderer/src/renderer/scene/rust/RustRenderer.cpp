#include <iostream>

#include <SDL2/SDL.h>

#include "RustRenderer.h"
#include "vange_rs.h"

using namespace renderer;
using namespace renderer::scene;
using namespace renderer::scene::rust;

RustRenderer::RustRenderer(int32_t width, int32_t height)
	: _map_created(false)
{
	std::cout << "RustRenderer::RustRenderer" << std::endl;

	rv_init_descriptor desc {
		.width = (uint32_t) width,
		.height = (uint32_t) height,
		.gl_functor = SDL_GL_GetProcAddress,
	};

	_context = rv_init(desc);
}

RustRenderer::~RustRenderer()
{
	rv_exit(_context);
}

void RustRenderer::camera_create(const CameraDescription& camera_description)
{
	std::cout << "RustRenderer::camera_create" << std::endl;
	rv_camera_description v_desc {
		.fov = camera_description.fov,
		.aspect = camera_description.aspect,
		.near= camera_description.near,
		.far = camera_description.far,
	};

	rv_camera_init(_context, v_desc);
}

void RustRenderer::camera_destroy()
{
	std::cout << "RustRenderer::camera_destroy" << std::endl;
	// TODO:
//	vange_rs_camera_destroy();
}

void RustRenderer::camera_set_transform(const Transform& transform)
{
	std::cout << "RustRenderer::camera_set_transform" << std::endl;
	rv_transform v_transform {
		.position = vange_rs_vector3 {
			.x = transform.position.x,
			.y = transform.position.y,
			.z = transform.position.z,
		},
		.rotation = vange_rs_quaternion {
			.x = transform.rotation.x,
			.y = transform.rotation.y,
			.z = transform.rotation.z,
			.w = transform.rotation.w,
		}
	};
	rv_camera_set_transform(_context, v_transform);
}

void RustRenderer::map_create(const MapDescription& map_description)
{
	std::cout << "RustRenderer::map_create" << std::endl;
	rv_map_description v_desc {
		.width = map_description.width,
		.height = map_description.height,
		.lineT = map_description.lineT,
		.material_begin_offsets = map_description.material_begin_offsets,
		.material_end_offsets = map_description.material_end_offsets,
		.material_count = map_description.material_count,
	};
	rv_map_init(_context, v_desc);
	_map_created = true;
}

void RustRenderer::map_destroy()
{
	std::cout << "RustRenderer::map_destroy" << std::endl;
	if(_map_created) {
		rv_map_exit(_context);
	} else {
		std::cerr << "map is already destroyed" <<std::endl;
	}
	_map_created = false;
}

void RustRenderer::map_request_update(const Rect& region)
{
	std::cout << "RustRenderer::map_request_update. region=" << region << std::endl;
	rv_rect v_rect {
		.x = region.x,
		.y = region.y,
		.width = region.width,
		.height = region.height,
	};

	rv_map_request_update(_context, v_rect);
}

void RustRenderer::map_update_palette(uint32_t* palette, int32_t palette_size)
{
	std::cout << "RustRenderer::map_update_palette" << std::endl;
	// TODO:
//	vange_rs_map_update_palette(palette, palette_size);
}

void RustRenderer::render(const Rect& viewport)
{
	std::cout << "RustRenderer::render" << std::endl;
	rv_rect v_rect {
		.x = viewport.x,
		.y = viewport.y,
		.width = viewport.width,
		.height = viewport.height,
	};
	rv_render(_context, v_rect);
}

