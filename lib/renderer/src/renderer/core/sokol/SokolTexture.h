//
// Created by nikita on 05.12.2021.
//

#ifndef VANGERS_SOKOLTEXTURE_H
#define VANGERS_SOKOLTEXTURE_H

#include <memory>

#include "../AbstractCoreRenderer.h"

namespace renderer::core {
	struct RenderContext;

	class SokolTexture {
	public:
		static std::unique_ptr<SokolTexture>
		create(TextureType texture_type, BlendMode blend_mode, int32_t width, int32_t height);
		void set_data(const uint8_t* data);
		void set_palette(const uint8_t* palette);
		void set_color(const Color& color);
		void render(int32_t screen_width, int32_t screen_height, const Rect &src_rect, const Rect &dst_rect);
		void destroy();
		int32_t width() const;
		int32_t height() const;
		TextureType texture_type() const;
		BlendMode blend_mode() const;
		~SokolTexture();
	private:
		SokolTexture(TextureType texture_type, BlendMode blend_mode, int32_t width, int32_t height,
		             std::unique_ptr<RenderContext> &&render_context);
		size_t data_size() const;
		TextureType _texture_type;
		BlendMode _blend_mode;
		int32_t _width;
		int32_t _height;
		Color _color;
		std::unique_ptr<RenderContext> _render_context;
		uint8_t* _data;
	};
}


#endif //VANGERS_SOKOLTEXTURE_H
