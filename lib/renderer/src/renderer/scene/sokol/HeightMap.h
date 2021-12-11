//
// Created by nikita on 28.11.2021.
//

#ifndef VANGERS_HEIGHTMAP_H
#define VANGERS_HEIGHTMAP_H

#include <cstdint>
#include <unordered_set>
#include <memory>

#include "../../common.h"
#include "../AbstractRenderer.h"

namespace renderer::scene::sokol {
	struct RenderContext;


	class HeightMap {
	public:
		HeightMap(const MapDescription& map_description);
		~HeightMap();

		void update_region(const Rect& region, uint8_t* region_height_map, uint8_t* region_meta);
		void render(int32_t viewport_width, int32_t viewport_height, int32_t camera_pos_x, int32_t camera_pos_y, int32_t camera_pos_z);
		void update_palette(uint32_t* palette);
		const MapDescription& map_decscription() const;
		void destroy();
	private:
		static std::unique_ptr<RenderContext> create_context(const MapDescription &map_description);
		void update_palette_texture();
		MapDescription map_desc;
		uint32_t* palette;
		std::unique_ptr<RenderContext> render_context;
	};
}




#endif //VANGERS_HEIGHTMAP_H
