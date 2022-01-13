//
// Created by nikita on 05.12.2021.
//

#ifndef VANGERS_SOKOLCORERENDERER_H
#define VANGERS_SOKOLCORERENDERER_H

#include <memory>

#include "../AbstractCompositor.h"
#include "../../ResourceStorage.h"

struct sg_pass_action;

namespace renderer::compositor::sokol {

	class SokolTexture;


	class SokolCompositor: public AbstractCompositor{
	public:
		SokolCompositor(int32_t screenWidth, int32_t screenHeight);

		Texture texture_create(int32_t width, int32_t height, TextureType texture_type, BlendMode blend_mode) override;

		void texture_set_data(Texture texture, uint8_t *data) override;

		void texture_render(Texture texture, const Rect &src_rect, const Rect &dst_rect) override;

		~SokolCompositor() override;

		void render_present() override;

		void texture_destroy(Texture texture) override;

		void texture_set_color(Texture texture, const Color& color) override;
		void render_begin() override;

		void initialize() override;

		void dispose() override;

		void texture_query(Texture texture, int32_t *width, int32_t *height, TextureType *texture_type,
		                   BlendMode *blend_mode) override;

		void query_output_size(int32_t *width, int32_t *height) override;

		void set_physical_screen_size(int32_t width, int32_t height) override;

		void set_logical_screen_size(int32_t width, int32_t height) override;

		void read_pixels(uint8_t *output) override;
	private:
		int32_t _logical_screen_width;
		int32_t _logical_screen_height;
		int32_t _screen_width;
		int32_t _screen_height;
		ResourceStorage<Texture, SokolTexture> _texture_storage;
		std::unique_ptr<sg_pass_action> _pass_action;
	};

}


#endif //VANGERS_SOKOLCORERENDERER_H
