#ifndef VANGERS_ABSTRACTRENDERER_H
#define VANGERS_ABSTRACTRENDERER_H

#include "../common.h"
#include "../ResourceId.h"

namespace renderer::scene {
	struct MapDescription {
		int32_t width; // H_SIZE
		int32_t height; // V_SIZE

		// data layout: first `width` bytes are height, the rest `width` bytes are meta data
		uint8_t** lineT;

		// Material offsets in the palette
		uint8_t* material_begin_offsets;
		uint8_t* material_end_offsets;

		// 8 for world, 16 for escave
		int32_t material_count;
	};

	struct CameraDescription {
		float fov;
		float aspect;
        float near_plane;
        float far_plane;
	};

	struct Vector3 {
		float x;
		float y;
		float z;
	};

	struct Quaternion {
		float x;
		float y;
		float z;
		float w;
	};

	struct Transform {
		Vector3 position;
		Quaternion rotation;
	};

	// TODO: Rename it to the AbstractSceneRenderer?
	class AbstractRenderer {
	public:
		virtual void camera_create(const CameraDescription& camera_description) = 0;
		virtual void camera_destroy() = 0;
		virtual void camera_set_transform(const Transform& transform) = 0;

		// Creates HeightMap from description
		virtual void map_create(const MapDescription& map_description) = 0;
		
		// Destroys the HeightMap
		virtual void map_destroy() = 0;
				
		// Updates a rectangular area the selected HeightMap with height data from *height* and meta data from *meta*
		// The renderer must not keep the height and meta pointers since they will be deleted after the call
		// TODO: the *height* parameter name is confusing
		// TODO: 
		virtual void map_request_update(const Rect& region) = 0;
		
		// Updates a 256-color palette for the HeightMap. 
		// The renderer must not keep the palette pointer since if will be deleted after the call
		virtual void map_update_palette(uint32_t* palette, int32_t palette_size) = 0;
		
		// Renders the scene into the viewport with size viewport_width*viewport_height and with camera position *camera_pos_XXX*
		// TODO: need to discuss and refactor this function signature
		virtual void render(const Rect& viewport) = 0;
		virtual void destroy() = 0;
		virtual ~AbstractRenderer() = default;
	};
}



#endif //VANGERS_ABSTRACTRENDERER_H
