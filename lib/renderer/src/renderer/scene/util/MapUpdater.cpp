#include <cassert>
#include <iostream>
#include <cstring>

#include "MapUpdater.h"

using namespace renderer::scene::util;

MapUpdater::MapUpdater(HeightMap map_rid, uint8_t **lineT)
	: _map_rid(map_rid)
	, _lineT(lineT)
	, _requested_region({})
{

}

void MapUpdater::request_region_update(const renderer::Rect &rect)
{
	if(_requested_region.width == 0 && _requested_region.height == 0){
		_requested_region = rect;
	} else {
		if(rect.x < _requested_region.x) {
			_requested_region.x = rect.x;
		}

		if(rect.y < _requested_region.y) {
			_requested_region.y = rect.y;
		}

		if(rect.x + rect.width > _requested_region.x + _requested_region.width) {
			int32_t new_width = rect.x + (uint32_t)rect.width - _requested_region.x;
			assert(new_width >= 0);
			_requested_region.width = new_width;
		}

		if(rect.y + rect.height > _requested_region.y + _requested_region.height) {
			int32_t new_height = rect.y + (uint32_t)rect.height - _requested_region.y;
			assert(new_height >= 0);
			_requested_region.height = new_height;
		}
	}

	std::cout << "MapUpdater::request_region_update() "
			<< "rect="<<rect
			<< " => _requested_region="<<_requested_region
			<<std::endl;
}

void MapUpdater::map_update(AbstractRenderer& renderer)
{
	int32_t map_width;
	int32_t map_height;
	renderer.map_query(_map_rid, &map_width, &map_height);

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

		uint8_t* _lineT_r = _lineT[iy + y_start];
		if(_lineT_r != nullptr){
			std::memcpy(height_map_r, _lineT_r + x_start, width * sizeof(uint8_t));
			std::memcpy(meta_r, _lineT_r + x_start + map_width, width * sizeof(uint8_t));
		}
	}

	renderer.map_update_data(_map_rid, _requested_region, height_map, meta);

	delete[] height_map;
	delete[] meta;
	_requested_region = {};
}
