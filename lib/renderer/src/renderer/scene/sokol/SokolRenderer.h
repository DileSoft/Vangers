//
// Created by nikita on 28.11.2021.
//

#ifndef VANGERS_SOKOLRENDERER_H
#define VANGERS_SOKOLRENDERER_H

#include <memory>

#include "../AbstractRenderer.h"
#include "../../ResourceStorage.h"
#include "HeightMap.h"

namespace renderer::scene {
	class SokolRenderer: public AbstractRenderer{
	public:
		SokolRenderer();
		~SokolRenderer() override;

		HeightMap map_create(const MapDescription& map_description) override;

		void map_destroy(HeightMap map_rid) override;

		void map_update_data(HeightMap map_rid, const Rect& rect, uint8_t* height, uint8_t* meta) override;

		void render(int32_t viewport_width, int32_t viewport_height, int32_t camera_pos_x, int32_t camera_pos_y, int32_t camera_pos_z) override;

		void map_update_palette(HeightMap map_rid, uint32_t *palette, int32_t palette_size) override;

		void map_query(HeightMap map_rid, int32_t* width, int32_t* height) override;
	private:
		ResourceStorage<HeightMap, sokol::HeightMap> map_storage;
	};

}


#endif //VANGERS_SOKOLRENDERER_H
