//
// Created by nikita on 28.11.2021.
//

#include <cstdint>
#include <cstring>
#include <cassert>
#include "../../lib/sokol_gfx.h"
#include "../../lib/HandmadeMath.h"

#include "HeightMap.h"


#define CODE(...) #__VA_ARGS__

using namespace renderer::scene::sokol;

namespace renderer::scene::sokol {
	typedef struct {
		hmm_mat4 transform;
	} vs_params_t;

	typedef struct {
		hmm_vec4 world_info;
	} fs_params_t;

	struct RenderContext {
		sg_pass_action pass_action;
		sg_buffer vertex_buffer;
		sg_buffer index_buffer;
		sg_image height_texture;
		sg_image meta_texture;
		sg_image palette_texture;
		sg_pipeline pip;
		sg_bindings bind;
		sg_shader shader;
	};
}

HeightMap::HeightMap(const MapDescription &map_description)
	: map_desc(map_description)
	, height_map(new uint8_t[map_desc.width * map_desc.height])
	, meta(new uint8_t[map_desc.width * map_desc.height])
	, palette(new uint32_t[256])
	, last_update(0)
	, render_context(create_context(map_description))
{
	int width = map_desc.width;
	int height = map_desc.height;
	std::memset(height_map, 0, width * height);
	std::memset(meta, 0, width * height);
	for(int iy = 0; iy < height; iy++){
		uint8_t* line = height_map + iy * width;
		for(int ix = 0; ix < width; ix++){
			uint8_t filled = ((ix / 64) % 2) != ((iy / 64) % 2) ? 255 : 0;
			filled *= (float)iy/(float)height;
			*line = filled;
			line ++;
		}
	}
}

sg_shader make_shader(){
	sg_shader_desc shd_desc {
		.vs = {
			.source = "#version 330\n" CODE(
				uniform mat4 transform;

				layout(location=0) in vec2 position;
				layout(location=1) in vec2 texcoord;
				out vec2 uv;

				void main(){
					gl_Position = transform * vec4(position, 0.0, 1.0);
					uv = texcoord;
				}
			),
			.uniform_blocks = {
				/*[0] =*/ {
					.size = sizeof(vs_params_t),
					.uniforms = {
						/*[0] =*/ {
								.name = "transform",
								.type=SG_UNIFORMTYPE_MAT4,
						},
					}
				}
			}
		},
		.fs = {
			.source = "#version 330\n" CODE(
				uniform usampler2D t_Height;
				uniform usampler2D t_Meta;
				uniform sampler2D t_Palette;

				uniform vec4 u_MapData;

				in vec2 uv;
				out vec4 frag_color;


				void main(){
					uint height = texture(t_Height, uv).x;
					uint meta = texture(t_Meta, uv).x;

					ivec2 world_pos = ivec2(uv * u_MapData.xy);

//					uint delta = meta & 3;
//					uint objShadowFlag = (meta >> 2) & 1;
					uint terrain = (meta >> uint(3)) & uint(7);
					uint doubleLevel = (meta >> uint(6)) & uint(1);
					if(doubleLevel == uint(1)){
						if(world_pos.x % 2 == 0){
							height = textureOffset(t_Height, uv, ivec2(1, 0)).x;
						}

					}

					frag_color = vec4(float(height) / 255.0);
				}
			),
			.uniform_blocks = {
				/*[0] =*/ {
					.size = sizeof(fs_params_t),
					.uniforms = {
						/*[0] =*/ {
								.name = "u_MapData",
								.type=SG_UNIFORMTYPE_FLOAT4,
						},
					}
				}
			},
			.images= {
				/*[0] =*/ {.name = "t_Height", .image_type = SG_IMAGETYPE_2D},
				/*[1] =*/ {.name = "t_Meta", .image_type = SG_IMAGETYPE_2D},
				/*[2] =*/ {.name = "t_Palette", .image_type = SG_IMAGETYPE_2D},
			},
		}
	};

	return sg_make_shader(shd_desc);
}

sg_image make_height_texture(int32_t width, int32_t height){
	return sg_make_image({
		 .width = width,
		 .height = height,
		 .usage = SG_USAGE_DYNAMIC,
		 .pixel_format = SG_PIXELFORMAT_R8UI,
		 .min_filter = SG_FILTER_NEAREST,
		 .mag_filter = SG_FILTER_NEAREST,
	});
}

sg_image make_palette_texture(int32_t num_colors){
	return sg_make_image({
		 .width = num_colors,
		 .height = 1,
		 .usage = SG_USAGE_DYNAMIC,
		 .pixel_format = SG_PIXELFORMAT_RGBA8,
		 .min_filter = SG_FILTER_NEAREST,
		 .mag_filter = SG_FILTER_NEAREST,
	});
}

std::unique_ptr<RenderContext> HeightMap::create_context(const MapDescription& map_description) {
	float vertices[] = {
		0.0f,  1.0f, 0.0f, 0.0f,
		1.0f,  1.0f, 1.0f, 0.0f,
		1.0f,  0.0f, 1.0f, 1.0f,
		0.0f,  0.0f, 0.0f, 1.0f,
	};

	std::unique_ptr<RenderContext> render_context = std::make_unique<RenderContext>();
	render_context->bind = {};

	sg_buffer vbuf = sg_make_buffer({
						.size = 0,
						.type = SG_BUFFERTYPE_VERTEXBUFFER,
						.data = SG_RANGE(vertices),
					 });
	render_context->bind.vertex_buffers[0] = vbuf;
	render_context->vertex_buffer = vbuf;

	uint16_t indices[] = {
			0, 1, 2,
			0, 2, 3
	};

	sg_buffer ibuf = sg_make_buffer({
						.type = SG_BUFFERTYPE_INDEXBUFFER,
						.data = SG_RANGE(indices),
					});
	render_context->bind.index_buffer = ibuf;
	render_context->index_buffer = ibuf;

	sg_shader shd = make_shader();

	render_context->shader = shd;

	sg_pipeline_desc pip_desc {
			.shader = shd,
			.layout = {
				.attrs = {
					/*[0] =*/ {.format = SG_VERTEXFORMAT_FLOAT2},
					/*[1] =*/ {.format = SG_VERTEXFORMAT_FLOAT2}
				}
			},
			.index_type = SG_INDEXTYPE_UINT16,
	};

	render_context->pip = sg_make_pipeline(pip_desc);

	int32_t width = map_description.width;
	int32_t height = map_description.height;

	sg_image height_texture = make_height_texture(width, height);
	sg_image meta_texture = make_height_texture(width, height);
	sg_image palette_texture = make_palette_texture(256);

	render_context->height_texture = height_texture;
	render_context->meta_texture = meta_texture;
	render_context->palette_texture = palette_texture;

	render_context->bind.fs_images[0] = height_texture;
	render_context->bind.fs_images[1] = meta_texture;
	render_context->bind.fs_images[2] = palette_texture;

	render_context->pass_action = {
		.colors = {
			/*[0] =*/ {
				.action=SG_ACTION_CLEAR,
				.value={1.0f, 1.0f, 1.0f, 1.0f}
			}
		}
	};
	return render_context;
}

void HeightMap::update_height_meta_textures()
{
	int32_t map_width = map_desc.width;
	int32_t map_height = map_desc.height;

	// TODO: highly uneffecient code!
	// sokol_gfx doesn't have procedures for modifying onle part of the texture
	sg_update_image(render_context->height_texture, {
		.subimage = {
			/* [0] =*/ {
					/* [0] =*/ {
					.ptr = height_map,
					.size = sizeof(uint8_t) * map_width * map_height
			}
		}}
	});

	sg_update_image(render_context->meta_texture, {
		.subimage = {
			/* [0] =*/ {
					/* [0] =*/ {
					.ptr = meta,
					.size = sizeof(uint8_t) * map_width * map_height
			}
		}}
	});
}

void HeightMap::update_region(const Rect& region, uint8_t* region_height_map, uint8_t* region_meta)
{
	std::cout << "HeightMap::update_height_and_meta_textures()" << std::endl;

	int32_t map_width = map_desc.width;
	int32_t map_height = map_desc.height;

	assert(region.x + region.width <= map_width);
	assert(region.y + region.height <= map_height);


	for(int iy = 0; iy < region.height; iy++){
		std::memcpy(
			height_map + (iy + region.y) * map_width + region.x,
			region_height_map + iy * region.width,
			region.width * sizeof(uint8_t)
		);

		std::memcpy(
			meta + (iy + region.y) * map_width + region.x,
			region_meta + iy * region.width,
			region.width * sizeof(uint8_t)
		);
	}




}

void HeightMap::update_palette_texture()
{
	sg_image_data image_data = {
			.subimage = {
					/* [0] =*/ {
							/* [0] =*/ {
							.ptr = palette,
							.size = sizeof(uint32_t) * 256
					}
			}}
	};

	sg_update_image(render_context->palette_texture, image_data);
}

void HeightMap::update_palette(uint32_t *palette)
{
	std::memcpy(this->palette, palette, sizeof(uint32_t) * 256);
}

const renderer::scene::MapDescription &HeightMap::map_decscription() const
{
	return this->map_desc;
}


void HeightMap::render(int32_t viewport_width, int32_t viewport_height, int32_t camera_pos_x, int32_t camera_pos_y, int32_t camera_pos_z) {
	if(last_update++ % update_frequency == 0){
		update_height_meta_textures();
	}
	update_palette_texture();

//	hmm_mat4 view = HMM_Translate({-(float)camera_pos_x, -(float)camera_pos_y, -(float)camera_pos_z});

	int32_t width = map_desc.width;
	int32_t height = map_desc.height;

	camera_pos_y = height - camera_pos_y;
	hmm_vec3 camera = HMM_Vec3(camera_pos_x, camera_pos_y, camera_pos_z * 2);
	hmm_vec3 look_at_center = HMM_Vec3(camera_pos_x, camera_pos_y, 0.0f);
	hmm_vec3 look_at_up = HMM_Vec3(0.0f, 1.0f, 0.0f);

	hmm_mat4 view = HMM_LookAt(camera, look_at_center, look_at_up);
	hmm_mat4 model = HMM_MultiplyMat4(
				HMM_Translate(HMM_Vec3(0, 0, 0.0f)),
				HMM_Scale(HMM_Vec3(width, height, 1.0f))
				);

	hmm_mat4 projection = HMM_Perspective(60, (float)viewport_width/(float)viewport_height, 0.01f, 100000.0f);
	hmm_mat4 transform = HMM_MultiplyMat4(
				HMM_MultiplyMat4(projection, view),
				model
				);
//	transform = HMM_Mat4d(1.0f);
//	transform = model;
//	transform = HMM_MultiplyMat4(view, model);

	vs_params_t vs_params = {
		.transform = transform,
	};

	fs_params_t fs_params = {
		.world_info = HMM_Vec4(width, height, 0.0f, 0.0f),
	};

	sg_begin_default_pass(&render_context->pass_action, viewport_width, viewport_height);
	sg_apply_pipeline(render_context->pip);
	sg_apply_bindings(render_context->bind);
	sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, SG_RANGE(vs_params));
	sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, SG_RANGE(fs_params));

	sg_draw(0, 6, 1);
	sg_end_pass();
	sg_commit();
}

HeightMap::~HeightMap() {
	delete[] height_map;
};
