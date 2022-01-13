//
// Created by nikita on 05.12.2021.
//

#include <cstring>
#include <cassert>
#include <string>

#include "../../lib/sokol_gfx.h"
#include "../../lib/HandmadeMath.h"

#include "SokolTexture.h"
#include "../../exception.h"

#define CODE(...) #__VA_ARGS__
using namespace renderer::core;

sg_shader get_shader_rgba();

typedef struct {
	hmm_mat4 transform;
	hmm_vec4 uv_transform;
} vs_params_t;

typedef struct {
	hmm_vec4 color;
} fs_params_t;


struct renderer::core::RenderContext {
	sg_pipeline pip;
	sg_bindings bind;
	sg_buffer vertex_buffer;
	sg_buffer index_buffer;
	sg_shader shader;
	sg_image image;
};


SokolTexture::SokolTexture(TextureType texture_type, BlendMode blend_mode, int32_t width, int32_t height,
						   std::unique_ptr<RenderContext> &&render_context)
	: _texture_type(texture_type)
	, _blend_mode(blend_mode)
	, _width(width)
	, _height(height)
	, _color({255, 255, 255, 255})
	, _render_context(std::move(render_context))
	, _data(nullptr)
{}

SokolTexture::~SokolTexture() {
	delete[] _data;
}

std::unique_ptr<SokolTexture>
SokolTexture::create(TextureType texture_type, BlendMode blend_mode, int32_t width, int32_t height) {
	std::unique_ptr<RenderContext> render_context = std::make_unique<RenderContext>(RenderContext{});

	float vertices[] = {
		-1.0f,  1.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 1.0f, 0.0f,
		 1.0f, -1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 1.0f,
	};
	sg_buffer_desc vbuf_desc = {
			.size = 0,
			.type = SG_BUFFERTYPE_VERTEXBUFFER,
			.data = SG_RANGE(vertices),
	};
	sg_buffer vbuf = sg_make_buffer(&vbuf_desc);
	render_context->bind.vertex_buffers[0] = vbuf;
	render_context->vertex_buffer = vbuf;

	uint16_t indices[] = {
			0, 1, 2,
			0, 2, 3
	};

	sg_buffer_desc ibuf_desc = {
//			.size = sizeof(indices),
			.type = SG_BUFFERTYPE_INDEXBUFFER,
			.data = SG_RANGE(indices),
	};

	sg_buffer ibuf = sg_make_buffer(ibuf_desc);
	render_context->index_buffer = ibuf;
	render_context->bind.index_buffer = ibuf;


	sg_shader shd;

	switch (texture_type) {
		case TextureType::RGBA32:{
			 sg_image image = sg_make_image({
								.width = (int32_t)width,
								.height = (int32_t)height,
								.usage = SG_USAGE_DYNAMIC,
								.pixel_format = SG_PIXELFORMAT_RGBA8,
								.min_filter = SG_FILTER_LINEAR,
								.mag_filter = SG_FILTER_LINEAR,
							});
			shd = get_shader_rgba();
			render_context->bind.fs_images[0] = image;
			render_context->image = image;
		}
		break;
		default:
			throw RendererException(std::string("InvalidTextureType ") + std::to_string((int)texture_type));
	}

	render_context->shader = shd;

	sg_blend_state blend_state;
	switch (blend_mode) {
		case BlendMode::Alpha:
			blend_state = {
					.enabled = true,
					.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
					.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
					.op_rgb = SG_BLENDOP_ADD,
					.src_factor_alpha = SG_BLENDFACTOR_ONE,
					.dst_factor_alpha = SG_BLENDFACTOR_ZERO,
					.op_alpha = SG_BLENDOP_ADD,
			};
			break;
		default:
			blend_state = {};
			break;
	}
	sg_pipeline pip = sg_make_pipeline({
		.shader = shd,
		.layout = {
			.attrs = {
				/*[0] =*/ {.offset = 0, .format = SG_VERTEXFORMAT_FLOAT2},
				/*[1] =*/ {.offset = sizeof(float) * 2, .format = SG_VERTEXFORMAT_FLOAT2},
			}
		},

		.colors = {
			/* [0] =*/ {
				.write_mask = SG_COLORMASK_RGBA,
				.blend = blend_state,
			}
		},
       .index_type = SG_INDEXTYPE_UINT16,
	});
	render_context->pip = pip;

	return std::unique_ptr<SokolTexture>(new SokolTexture(texture_type, BlendMode::None, width, height,
	                                                      std::move(render_context)));
}

void SokolTexture::set_palette(const uint8_t *palette) {
	sg_image_data image_data = {
			.subimage = {
					/*[0] =*/ {
							/* [0] =*/ {
									.ptr = palette,
									.size = 256 * sizeof(uint32_t)
							}
					}}
	};

	sg_update_image(_render_context->bind.fs_images[1], image_data);
}

void SokolTexture::set_data(const uint8_t* data) {
	sg_image_data image_data = {
			.subimage = {
					/* [0] =*/ {
							/* [0] =*/ {
							.ptr = data,
							.size = data_size()
					}
			}}
	};

	sg_update_image(_render_context->bind.fs_images[0], image_data);
}

void SokolTexture::render(int32_t screen_width, int32_t screen_height, const Rect &src_rect, const Rect &dst_rect) {
	sg_apply_pipeline(_render_context->pip);
	sg_apply_bindings(_render_context->bind);
	vs_params_t vs_params;


	if(src_rect.width != 0 && src_rect.height != 0){
		vs_params.uv_transform = {
			.Elements = {
				(float)src_rect.x / (float)_width,
				(float)src_rect.y / (float)_height, 
				(float)(src_rect.x + src_rect.width) / (float)_width, 
				(float)(src_rect.y + src_rect.height) / (float)_height,
			}
		};
	} else {
		vs_params.uv_transform = {.Elements = {0.0f, 0.0f, 1.0f, 1.0f}};
	}

	if(dst_rect.width != 0 && dst_rect.height != 0){
		float image_scale_x = (float) dst_rect.width / (float) screen_width;
		float image_scale_y = (float) dst_rect.height / (float) screen_height;

		float translate_x = -1.0f + image_scale_x + 2.0f * (float)dst_rect.x / (float)screen_width;
		float translate_y = 1.0f - image_scale_y - 2.0f * (float)dst_rect.y / (float)screen_height;

		vs_params.transform = HMM_MultiplyMat4(
				HMM_Translate({{translate_x, translate_y, 0.0f}}),
				HMM_Scale({{image_scale_x, image_scale_y, 1.0f}})
		);
	} else {
		vs_params.transform = HMM_Mat4d(1.0);
	}

	// TODO: set _color type as hmm_vec4
	fs_params_t fs_params {
		.color = {
			.Elements = {
				(float)_color.r / 255.0f, 
				(float)_color.g / 255.0f, 
				(float)_color.b / 255.0f, 
				(float)_color.a / 255.0f
			}
		}
	};
	sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, SG_RANGE(vs_params));
	sg_apply_uniforms(SG_SHADERSTAGE_FS, 0, SG_RANGE(fs_params));
	sg_draw(0, 6, 1);
}

size_t SokolTexture::data_size() const {
	switch (_texture_type) {
		case TextureType::RGBA32:
			return sizeof (uint32_t) * _width * _height;
		default:
			throw RendererException(std::string("InvalidTextureType ") + std::to_string((int)_texture_type));
	}
}

void SokolTexture::destroy() {
	assert(_render_context);

	sg_destroy_image(_render_context->image);
	sg_destroy_buffer(_render_context->vertex_buffer);
	sg_destroy_buffer(_render_context->index_buffer);
	sg_destroy_shader(_render_context->shader);
	sg_destroy_pipeline(_render_context->pip);
}

sg_shader get_shader_rgba() {
	sg_shader_desc shd_desc = {
			.vs = {
					.source = R"(
	#version 300 es
	precision mediump float;
	uniform mat4 transform;
	uniform vec4 uv_transform;
	attribute vec2 position;
	attribute vec2 texcoord0;

	out vec2 uv;

	void main() {
		vec4 pos = vec4(position, 0.5, 1.0);

		gl_Position = transform * pos;
		uv = texcoord0 * uv_transform.zw + uv_transform.xy;
	}
					)",
					.uniform_blocks = {
							/*[0] =*/ {
									.size = sizeof(vs_params_t),
									.uniforms = {
											/*[0] =*/ {
													.name = "transform",
													.type=SG_UNIFORMTYPE_MAT4,
											},
											/*[1] =*/ {
													.name = "uv_transform",
													.type=SG_UNIFORMTYPE_FLOAT4,
											},
									}
							}
					}

			},
			.fs = {
					.source = R"(
	#version 300 es
	precision mediump float;

	uniform vec4 color;
	uniform sampler2D tex;

	in vec2 uv;
	out vec4 FragColor;
	void main() {
		FragColor = texture(tex, uv) * color;
	}
					)",
					.uniform_blocks = {
							/*[0] =*/ {
									.size = sizeof(fs_params_t),
									.uniforms = {
											/*[0] =*/ {
													.name = "color",
													.type=SG_UNIFORMTYPE_FLOAT4,
											},
									}
							}
					},

					.images= {
							/*[0] =*/ {.name = "tex", .image_type = SG_IMAGETYPE_2D,}
					},
			}
	};

	sg_shader shd = sg_make_shader(&shd_desc);
	return shd;
}

//sg_shader SokolTexture::get_shader_palette8bit() {
//	sg_shader_desc shd_desc = {
//			.vs = {
//					.source = "#version 330\n" CODE(
//							uniform mat4 mvp;
//							layout(location=0)  in vec2 position;
//							layout(location=1) in vec2 texcoord0;
//
//							out vec2 uv;
//
//							void main() {
//								vec4 pos = vec4(position, 0.5, 1.0);
//
//								gl_Position = mvp * pos;
//								uv = texcoord0;
//							}
//					),
//					.uniform_blocks = {
//							[0] = {
//									.size = sizeof(params_t),
//									.uniforms = {
//											[0] = {
//													.name = "mvp",
//													.type=SG_UNIFORMTYPE_MAT4
//											}
//									}
//							}
//					}
//
//			},
//			.fs = {
//					.source = CODE(
//							  uniform usampler2D t_Color;
//					          uniform sampler2D palette;
//							  in vec2 uv;
//							  out vec4 frag_color;
//							  void main() {
//							    uint index = texture(t_Color, uv).r;
//							    vec4 palC = texture(palette, vec2(float(index)/256.0,0));
////							    frag_color = vec4(1.0, 0.0, 0.0, 1.0);
//						        frag_color = palC;
//					          })
//					,
//					.images= {
//							[0] = {.name = "t_Color", .image_type = SG_IMAGETYPE_2D},
//							[1] = {.name = "palette", .image_type = SG_IMAGETYPE_2D},
//					}
//			}
//	};
//
//	sg_shader shd = sg_make_shader(&shd_desc);
//	return shd;
//}

int32_t SokolTexture::height() const {
	return _height;
}

int32_t SokolTexture::width() const {
	return _width;
}

TextureType SokolTexture::texture_type() const {
	return _texture_type;
}

BlendMode SokolTexture::blend_mode() const {
	return _blend_mode;
}

void SokolTexture::set_color(const Color &color){
	_color = color;
}
