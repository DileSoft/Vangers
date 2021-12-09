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

		void render(int32_t viewport_width, int32_t viewport_height, int32_t camera_pos_x, int32_t camera_pos_y, int32_t camera_pos_z) override;

	};

}


#endif //VANGERS_DUMMYRENDERER_H
