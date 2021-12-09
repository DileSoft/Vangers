#ifndef MAPUPDATER_H
#define MAPUPDATER_H

#include "../../common.h"
#include "../AbstractRenderer.h"

namespace renderer::scene::util {
	class MapUpdater
	{
	public:
		MapUpdater(HeightMap map, uint8_t** _lineT);
		void request_region_update(const Rect& rect);
		void map_update(AbstractRenderer& renderer);
	private:
		HeightMap _map_rid;
		uint8_t** _lineT;
		Rect _requested_region;
	};

}

#endif // MAPUPDATER_H
