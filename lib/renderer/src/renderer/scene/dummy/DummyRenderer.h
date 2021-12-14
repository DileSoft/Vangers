//
// Created by nikita on 30.11.2021.
//

#ifndef VANGERS_DUMMYRENDERER_H
#define VANGERS_DUMMYRENDERER_H

#include "../AbstractRenderer.h"

namespace renderer::scene::dummy {
	class DummyRenderer: public AbstractRenderer{
	public:
		HeightMap map_create(const MapDescription& map_description) override;

		void map_destroy(HeightMap map_rid) override;

		void map_update_data(HeightMap map_rid, const Rect& rect, uint8_t* height, uint8_t* meta) override;

		void render(const Rect& viewport, Camera camera) override;

		Camera camera_create(const CameraDescription &camera_description) override;

		void camera_destroy(Camera camera) override;

		void camera_set_transform(Camera camera, const Transform &transform) override;

		void map_query(HeightMap map, int32_t *width, int32_t *height) override;

		void map_update_palette(HeightMap map, uint32_t *palette, int32_t palette_size) override;
	};

}


#endif //VANGERS_DUMMYRENDERER_H
