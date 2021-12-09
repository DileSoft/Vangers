#ifndef VANGERS_ABSTRACTRENDERER_H
#define VANGERS_ABSTRACTRENDERER_H

#include "../common.h"
#include "../ResourceId.h"

namespace renderer::scene {
	struct MapDescription {
		int32_t width;
		int32_t height;
		uint8_t* material_begin_offsets;
		uint8_t* material_end_offsets;
		int32_t material_count;
	};

	enum class _HeightMapT;
	typedef ResourceId<_HeightMapT> HeightMap;
	static_assert (sizeof (HeightMap) == sizeof (int32_t), "invalid HeightMap RID size");


	class AbstractRenderer {
	public:
		virtual HeightMap map_create(const MapDescription& map_description) = 0;
		virtual void map_destroy(HeightMap map) = 0;
		virtual void map_query(HeightMap map, int32_t* width, int32_t* height) = 0;
		virtual void map_update_data(HeightMap map, const Rect& rect, uint8_t* height, uint8_t* meta) = 0;
		virtual void map_update_palette(HeightMap map, uint32_t* palette, int32_t palette_size) = 0;
		virtual void render(int32_t viewport_width, int32_t viewport_height, int32_t camera_pos_x, int32_t camera_pos_y, int32_t camera_pos_z) = 0;
		virtual ~AbstractRenderer() = default;
	};
}



#endif //VANGERS_ABSTRACTRENDERER_H
