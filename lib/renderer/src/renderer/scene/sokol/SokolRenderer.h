//
// Created by nikita on 28.11.2021.
//

#ifndef VANGERS_SOKOLRENDERER_H
#define VANGERS_SOKOLRENDERER_H

#include <memory>

#include "../AbstractRenderer.h"
#include "../../ResourceStorage.h"
#include "SokolHeightMap.h"
#include "SokolCamera.h"

namespace renderer::scene {
	class SokolRenderer: public AbstractRenderer{
	public:
		SokolRenderer();
		~SokolRenderer() override;

		void camera_create(const CameraDescription& desc) override;
		void camera_destroy() override;
		void camera_set_transform(const Transform& transform) override;

		void map_create(const MapDescription& map_description) override;

		void map_destroy() override;

		void map_request_update(const Rect& region) override;

		void render(const Rect& viewport) override;

		void map_update_palette(uint32_t *palette, int32_t palette_size) override;

		void map_query(int32_t* width, int32_t* height) override;
	private:
		std::unique_ptr<sokol::SokolHeightMap> map;
		std::unique_ptr<sokol::SokolCamera> camera;
	};

}


#endif //VANGERS_SOKOLRENDERER_H
