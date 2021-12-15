//
// Created by nikita on 28.11.2021.
//

#include <cstdint>
#include <cstring>
#include <cassert>
#include "../../lib/sokol_gfx.h"
#include "../../lib/sokol_gfx_ext.h"
#include "../../lib/HandmadeMath.h"

#include "SokolHeightMap.h"
#include "MapUpdater.h"

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
		sg_image offsets_texture;
		sg_pipeline pip;
		sg_bindings bind;
		sg_shader shader;
	};
}

SokolHeightMap::SokolHeightMap(const MapDescription &map_description)
	: map_desc(map_description)
	, map_updater(std::make_unique<MapUpdater>())
	, palette(new uint32_t[256])
	, render_context(create_context(map_description))
{
}

void SokolHeightMap::request_update_region(const renderer::Rect &region)
{
	map_updater->request_region_update(region);
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
				const float c_HorFactor = 0.5f; //H_CORRECTION
				const float c_DiffuseScale = 8.0;
				const float c_ShadowDepthScale = 2.0 / 3.0;

				uniform usampler2D t_Height;
				uniform usampler2D t_Meta;
				uniform sampler2D t_Palette;
				uniform usampler2D t_Offsets;

				uniform vec4 u_MapData;

				in vec2 uv;
				out vec4 frag_color;

				struct TexelInfo {
					uint height;
					uint terrain;
					uint double_level;
					ivec2 world_pos;
				};

				TexelInfo get_texel_info(ivec2 world_pos){
//					world_pos = mod(world_pos, u_MapData.x);
					uint meta = texelFetch(t_Meta, world_pos, 0).x;

					uint double_level = (meta >> uint(6)) & uint(1);
					uint height;

					if(double_level == uint(1)){
						if(world_pos.x % 2 == 0){
							height = texelFetch(t_Height, world_pos + ivec2(1, 0), 0).x;
							meta = texelFetch(t_Meta, world_pos + ivec2(1, 0), 0).x;
						} else {
							height = texelFetch(t_Height, world_pos, 0).x;
						}
					} else {
						height = texelFetch(t_Height, world_pos, 0).x;
					}

					uint terrain = (meta >> uint(3)) & uint(7);
					return TexelInfo(height, terrain, double_level, world_pos);
				}

			vec4 get_color(TexelInfo texel_info){
				float height_normalized = float(texel_info.height) / 255.0f;
				TexelInfo t_left = get_texel_info(texel_info.world_pos + ivec2(-1, 0));
				TexelInfo t_right= get_texel_info(texel_info.world_pos + ivec2(1, 0));

				float height_diff = (float(t_right.height) - float(t_left.height)) / 255.0f;


				vec3 mat = texel_info.terrain == uint(0) ? vec3(5.0, 1.25, 0.5) : vec3(1.0);
				float dx = mat.x * c_DiffuseScale;
				float sd = mat.y * c_ShadowDepthScale;
				float jj = mat.z * height_diff * 256.0;

				float light_clr = (dx * sd - jj) / sqrt((1.0 + sd * sd) * (dx * dx + jj * jj));


				float lit_factor = light_clr - c_HorFactor * (1.0 - height_normalized);
				lit_factor = clamp(lit_factor, 0.0f, 1.0f);


				uvec4 offsets = texelFetch(t_Offsets, ivec2(texel_info.terrain, 0), 0);
				uint offset_begin = offsets.x;
				uint offset_end = offsets.y;
				vec4 color_begin = texelFetch(t_Palette, ivec2(offset_begin, 0), 0);
				vec4 color_end = texelFetch(t_Palette, ivec2(offset_end, 0), 0);

				vec4 color =  mix(color_begin, color_end, lit_factor);
				return color;
			}
				void main(){
					TexelInfo info = get_texel_info(ivec2(mod(uv, 1.0f) * u_MapData.xy));

					frag_color = get_color(info);

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
				/*[3] =*/ {.name = "t_Offsets", .image_type = SG_IMAGETYPE_2D},
			},
		}
	};

	return sg_make_shader(shd_desc);
}

sg_image make_offsets_texture(const renderer::scene::MapDescription& map_desc, uint8_t* buffer){
	for(int i = 0; i < map_desc.material_count; i++){
		buffer[i * 4 + 0] = map_desc.material_begin_offsets[i];
		buffer[i * 4 + 1] = map_desc.material_end_offsets[i];
	}

	return sg_make_image({
		 .width = map_desc.material_count,
		 .height = 1,
		 .usage = SG_USAGE_IMMUTABLE,
		 .pixel_format = SG_PIXELFORMAT_RGBA8UI,
		 .min_filter = SG_FILTER_NEAREST,
		 .mag_filter = SG_FILTER_NEAREST,
		 .data = sg_image_data {
			 .subimage = {
				 /*[0] =*/ {
					 /* [0]=*/ sg_range {
						 .ptr = buffer,
						 .size = sizeof (uint32_t) * map_desc.material_count
					 }
				 }
			 }
		 }
	});
}

sg_image make_height_texture(int32_t width, int32_t height){
	return sg_make_image({
		 .width = width,
		 .height = height,
		 .usage = SG_USAGE_STREAM,
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

std::unique_ptr<RenderContext> SokolHeightMap::create_context(const MapDescription& map_description) {
	std::cout << "HeightMap::create_contex"<<std::endl;
	float vertices[] = {
		-1.0f,  2.0f, 0.0f, 0.0f,
		2.0f,  2.0f, 3.0f, 0.0f,
		2.0f,  -1.0f, 3.0f, 3.0f,
		-1.0f,  -1.0f, 0.0f, 3.0f,
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
	uint8_t* offsets_buffer = new u_int8_t[sizeof(uint32_t) * map_description.material_count];
	sg_image offsets_texture = make_offsets_texture(map_description, offsets_buffer);
	delete[] offsets_buffer;

	render_context->height_texture = height_texture;
	render_context->meta_texture = meta_texture;
	render_context->palette_texture = palette_texture;
	render_context->offsets_texture = offsets_texture;

	render_context->bind.fs_images[0] = height_texture;
	render_context->bind.fs_images[1] = meta_texture;
	render_context->bind.fs_images[2] = palette_texture;
	render_context->bind.fs_images[3] = offsets_texture;

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

void SokolHeightMap::update_region(const Rect& region, uint8_t* region_height_map, uint8_t* region_meta)
{
	std::cout << "HeightMap::update_region()"
				<< " region: " << region
				<< " height_texture: " << render_context->height_texture.id
				<< " meta_texture: " << render_context->meta_texture.id
				<< std::endl;

	int32_t map_width = map_desc.width;
	int32_t map_height = map_desc.height;

	assert(region.x + region.width <= map_width);
	assert(region.y + region.height <= map_height);

	sg_ext_update_subimage(render_context->height_texture, {
	   .x = region.x,
	   .y = region.y,
	   .z = 0,
	   .width = region.width,
	   .height = region.height,
	   .num_slices = 1,
	   .data = {
		   .subimage = {
			   /* [0] =*/ {
					   /* [0] =*/ {
					   .ptr = region_height_map,
					   .size = sizeof(uint8_t) * region.width * region.height
			   }
		   }}
	   }
   });

	sg_ext_update_subimage(render_context->meta_texture, {
		.x = region.x,
		.y = region.y,
		.z = 0,
		.width = region.width,
		.height = region.height,
		.num_slices = 1,
		.data = {
			.subimage = {
				/* [0] =*/ {
						/* [0] =*/ {
						.ptr = region_meta,
						.size = sizeof(uint8_t) * region.width * region.height
				}
			}}
		}
	});


}

void SokolHeightMap::update_palette_texture()
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

void SokolHeightMap::update_palette(uint32_t *palette)
{
	std::memcpy(this->palette, palette, sizeof(uint32_t) * 256);
}

const renderer::scene::MapDescription &SokolHeightMap::map_decscription() const
{
	return this->map_desc;
}

void SokolHeightMap::destroy()
{
	std::cout << "HeightMap::destroy" << std::endl;
	assert(render_context);

	sg_destroy_image(render_context->height_texture);
	sg_destroy_image(render_context->meta_texture);
	sg_destroy_image(render_context->palette_texture);
	sg_destroy_image(render_context->offsets_texture);
	sg_destroy_buffer(render_context->vertex_buffer);
	sg_destroy_buffer(render_context->index_buffer);
	sg_destroy_shader(render_context->shader);
	sg_destroy_pipeline(render_context->pip);
}


void SokolHeightMap::render(const Rect& viewport, const hmm_mat4& camera_transform) {
	map_updater->map_update(*this);
	update_palette_texture();

	int32_t width = map_desc.width;
	int32_t height = map_desc.height;

	vs_params_t vs_params = {
		.transform = camera_transform,
	};

	fs_params_t fs_params = {
		.world_info = HMM_Vec4(width, height, 0.0f, 0.0f),
	};

	sg_begin_default_pass(&render_context->pass_action, viewport.width, viewport.height);
	sg_apply_pipeline(render_context->pip);
	sg_apply_bindings(render_context->bind);
	sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, SG_RANGE(vs_params));
	sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, SG_RANGE(fs_params));

	sg_draw(0, 6, 1);
	sg_end_pass();
	sg_commit();
}

SokolHeightMap::~SokolHeightMap() = default;
