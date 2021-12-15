#include <cassert>
#include <iostream>
#include <cstring>

#include "MapUpdater.h"
#include "SokolHeightMap.h"

using namespace renderer::scene::sokol;

MapUpdater::MapUpdater()
	: _requested_region({})
{

}

void MapUpdater::request_region_update(const renderer::Rect &rect)
{
	_requested_region.expand(rect);

	std::cout << "MapUpdater::request_region_update() "
			<< ", rect="<<rect
			<< " => _requested_region="<<_requested_region
			<<std::endl;
}

void MapUpdater::map_update(renderer::scene::sokol::SokolHeightMap& map)
{
	if(_requested_region.is_empty()){
		return;
	}

	const MapDescription& desc = map.map_decscription();

	int32_t map_width = desc.width;
	int32_t map_height = desc.height;
	uint8_t** lineT = desc.lineT;

	// TODO: forcing region to the full level width...
	_requested_region.x = 0;
	_requested_region.width = map_width;


	if(map_width < _requested_region.width + _requested_region.x){
		_requested_region.width = map_width - _requested_region.x;
	}

	if(map_height < _requested_region.height + _requested_region.y){
		_requested_region.height = map_height - _requested_region.y;
	}

	assert(map_width >= _requested_region.width + _requested_region.x);
	assert(map_height >= _requested_region.height + _requested_region.y);

	int32_t width= _requested_region.width;
	int32_t height = _requested_region.height;
	uint8_t* height_map = new uint8_t[width * height];
	uint8_t* meta = new uint8_t[width * height];

	std::memset(height_map, 0, sizeof(uint8_t) * width * height);
	std::memset(meta, 0, sizeof(uint8_t) * width * height);

	int x_start = _requested_region.x;
	int y_start = _requested_region.y;

	for(int iy = 0; iy < height; iy++){
		uint8_t* height_map_r = height_map + iy * width;
		uint8_t* meta_r = meta + iy * width;

		uint8_t* _lineT_r = lineT[iy + y_start];
		if(_lineT_r != nullptr){
			std::memcpy(height_map_r, _lineT_r + x_start, sizeof(uint8_t) * width);
			std::memcpy(meta_r, _lineT_r + x_start + map_width, sizeof(uint8_t) * width);
		}
	}

	map.update_region(_requested_region, height_map, meta);

	delete[] height_map;
	delete[] meta;
	_requested_region = {};
}
