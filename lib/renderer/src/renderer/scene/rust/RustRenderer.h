#ifndef RUSTRENDERER_H
#define RUSTRENDERER_H

#include "../AbstractRenderer.h"

typedef void* rv_context;

namespace renderer::scene::rust {
	class RustRenderer: public AbstractRenderer
	{
	public:
		RustRenderer(int32_t width, int32_t height);
		~RustRenderer();
		void camera_create(const CameraDescription& camera_description) override;
		void camera_destroy() override;
		void camera_set_transform(const Transform& transform) override;
		void map_create(const MapDescription& map_description) override;
		void map_destroy() override;
		void map_request_update(const Rect& region) override;
		void map_update_palette(uint32_t* palette, int32_t palette_size) override;
		void render(const Rect& viewport) override;
		void destroy() override;
	private:
		rv_context _context;
		bool _map_created;
		bool _camera_created;


	};

}

#endif // RUSTRENDERER_H
