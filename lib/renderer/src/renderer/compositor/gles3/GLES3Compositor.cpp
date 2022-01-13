#include "GLES3Compositor.h"
#include "GLES3Texture.h"
#include "Shader.h"
#include "QuadVertexArray.h"
#include "../CompositorException.h"
#include "../../lib/HandmadeMath.h"

#include <glad/glad.h>
#include <cstring>

using namespace renderer::compositor;
using namespace renderer::compositor::gles3;

const char* vs_code = R"(
	#version 300 es
	precision mediump float;

	uniform mat4 transform;
	uniform vec4 uv_transform;
	in vec2 position;
	in vec2 texcoord0;

	out vec2 uv;

	void main() {
		vec4 pos = vec4(position, 0.5, 1.0);
		gl_Position = transform * pos;
		uv = texcoord0 * uv_transform.zw + uv_transform.xy;
	}
)";

const char* fs_code = R"(
	#version 300 es
	precision mediump float;

	uniform vec4 color;
	uniform sampler2D tex;

	in vec2 uv;
	out vec4 FragColor;
	void main() {
		FragColor = texture(tex, uv) * color;
//		FragColor = vec4(uv.x, uv.y, 1.0, 1.0f);
	}
)";




GLES3Compositor::GLES3Compositor(int32_t screen_width, int32_t screen_height, GLADloadproc loadproc)
	: _screen_width(screen_width)
    , _screen_height(screen_height)
	, _logical_screen_width(screen_width)
	, _logical_screen_height(screen_height)
	, _texture_storage()
	, _texture_shader(std::make_unique<Shader>())
	, _vertex_array(std::make_unique<QuadVertexArray>())
	, _previous_vao(0)
{
	gladLoadGLES2Loader(loadproc);

	_texture_shader->initialize(vs_code, fs_code);
	_texture_shader->use();
	_texture_shader->set_uniform("tex", 0);
	_texture_shader->unuse();
	_vertex_array->initialize();
}

GLES3Compositor::~GLES3Compositor()
{

}

Texture GLES3Compositor::texture_create(int32_t width, int32_t height, TextureType texture_type, BlendMode blend_mode)
{
	return _texture_storage.create(std::make_unique<GLES3Texture>(width, height, texture_type, blend_mode, nullptr));
}

void GLES3Compositor::texture_set_data(Texture texture, uint8_t* data)
{
	auto& t = _texture_storage.getOrThrow(texture);

	glBindTexture(GL_TEXTURE_2D, t->name());
	// reset unpack options
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
	glTexSubImage2D(GL_TEXTURE_2D,
					0, 0, 0, t->width(), t->height(),
					// TODO:
					GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void GLES3Compositor::texture_render(Texture texture, const renderer::Rect& src_rect, const renderer::Rect& dst_rect)
{
	auto& t = _texture_storage.getOrThrow(texture);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, t->name());

	GLuint gl_error = glGetError();
	if(gl_error != 0){
		std::cout << "glBindTexture error: " << gl_error << std::endl;
	}

	_texture_shader->use();
	_vertex_array->bind();

	hmm_vec4 uv_transform;
	hmm_mat4 transform;

	if(src_rect.width != 0 && src_rect.height != 0){
		uv_transform = {.Elements = {
			(float)src_rect.x / (float)t->width(),
			(float)src_rect.y / (float)t->height(),
			(float)(src_rect.x + src_rect.width) / (float)t->width(),
			(float)(src_rect.y + src_rect.height) / (float)t->height(),
		}
					   };

	} else {
		uv_transform = {.Elements = {0.0f, 0.0f, 1.0f, 1.0f}};
	}

	if(dst_rect.width != 0 && dst_rect.height != 0){
		float image_scale_x = (float) dst_rect.width / (float) _screen_width;
		float image_scale_y = (float) dst_rect.height / (float) _screen_height;

		float translate_x = -1.0f + image_scale_x + 2.0f * (float)dst_rect.x / (float)_screen_width;
		float translate_y = 1.0f - image_scale_y - 2.0f * (float)dst_rect.y / (float)_screen_height;

		transform = HMM_MultiplyMat4(
				HMM_Translate({{translate_x, translate_y, 0.0f}}),
				HMM_Scale({{image_scale_x, image_scale_y, 1.0f}})
		);
	} else {
		transform = HMM_Mat4d(1.0);
	}

	Color tcolor = t->color();
	hmm_vec4 color = {
			.Elements = {
				(float)tcolor.r / 255.0f,
				(float)tcolor.g / 255.0f,
				(float)tcolor.b / 255.0f,
				(float)tcolor.a / 255.0f
			}
	};

	_texture_shader->set_uniform_mat("transform", (float*)transform.Elements);
	_texture_shader->set_uniform("uv_transform", uv_transform.Elements);
	_texture_shader->set_uniform("color", color.Elements);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	_texture_shader->unuse();
	_vertex_array->unbind();
}

void GLES3Compositor::texture_destroy(Texture texture)
{
	auto& t = _texture_storage.getOrThrow(texture);
	t->destroy();
	_texture_storage.remove(texture);
}

void GLES3Compositor::texture_query(Texture texture, int32_t* width, int32_t* height, TextureType* texture_type, BlendMode* blend_mode)
{
	auto& t = _texture_storage.getOrThrow(texture);

	if(width != nullptr){
		*width = t->width();
	}

	if(height != nullptr){
		*height = t->height();
	}

	if(texture_type != nullptr){
		*texture_type = t->texture_type();
	}

	if(blend_mode != nullptr){
		*blend_mode = t->blend_mode();
	}
}

void GLES3Compositor::texture_set_color(Texture texture, const renderer::Color& color)
{
	auto& t = _texture_storage.getOrThrow(texture);
	t->set_color(color);
}

const char* guiDebugGroup = "GLES3Compositor render";

void GLES3Compositor::render_begin()
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, std::strlen(guiDebugGroup), guiDebugGroup);
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &_previous_vao);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, _logical_screen_width, _logical_screen_height);
}

void GLES3Compositor::render_present()
{
	glBindVertexArray(_previous_vao);
	glPopDebugGroup();
}

void GLES3Compositor::initialize()
{

}

void GLES3Compositor::dispose()
{
	for(auto& [rid, t]: _texture_storage){
		texture_destroy(rid);
	}

	// TODO: shader
	// TODO: VAO
}

void GLES3Compositor::query_output_size(int32_t* width, int32_t* height)
{
	if(width != nullptr){
		*width = _screen_width;
	}

	if(height != nullptr){
		*height = _screen_height;
	}
}

void GLES3Compositor::set_physical_screen_size(int32_t width, int32_t height)
{
	_screen_width = width;
	_screen_height = height;
}

void GLES3Compositor::set_logical_screen_size(int32_t width, int32_t height)
{
	_logical_screen_width = width;
	_logical_screen_height = height;
}

void GLES3Compositor::read_pixels(uint8_t*)
{
	// TODO:
	throw CompositorException("GLES3Compositor::read_pixels is not implemented");
}
