#ifndef MAPUPDATER_H
#define MAPUPDATER_H

#include "../../common.h"
#include "../AbstractRenderer.h"

namespace renderer::scene::sokol {

	// PIMPL
	class SokolHeightMap;

	class MapUpdater
	{
	public:
		MapUpdater();
		void request_region_update(const Rect& rect);
		void map_update(renderer::scene::sokol::SokolHeightMap& map);
	private:
		Rect _requested_region;
	};

}

#endif // MAPUPDATER_H
