#ifndef SOKOLCAMERA_H
#define SOKOLCAMERA_H

#include "../AbstractRenderer.h"
#include "../../lib/HandmadeMath.h"

namespace renderer::scene::sokol {
	class SokolCamera
	{
	public:
		SokolCamera(const CameraDescription& camera_description);
		void set_transform(const Transform& transform);
		hmm_mat4 transform_mat(const MapDescription& desc) const;
	private:
		hmm_mat4 _projection;
		hmm_vec3 _position;
		hmm_quaternion _rotation;
	};

}


#endif // SOKOLCAMERA_H
