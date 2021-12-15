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

		Camera camera_create(const CameraDescription& desc) override;
		void camera_destroy(Camera camera) override;
		void camera_set_transform(Camera camera, const Transform& transform) override;

		HeightMap map_create(const MapDescription& map_description) override;

		void map_destroy(HeightMap map_rid) override;

		void map_request_update(HeightMap map_rid, const Rect& region) override;

		void render(const Rect& viewport, Camera camera) override;

		void map_update_palette(HeightMap map_rid, uint32_t *palette, int32_t palette_size) override;

		void map_query(HeightMap map_rid, int32_t* width, int32_t* height) override;
	private:
		ResourceStorage<HeightMap, sokol::SokolHeightMap> map_storage;
		ResourceStorage<Camera, sokol::SokolCamera> camera_storage;
	};

}


#endif //VANGERS_SOKOLRENDERER_H
