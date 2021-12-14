#include "SokolCamera.h"


renderer::scene::sokol::SokolCamera::SokolCamera(const renderer::scene::CameraDescription &camera_description)
	: _projection(HMM_Perspective(
					  camera_description.fov,
					  camera_description.aspect,
					  camera_description.near,
					  camera_description.far
					  ))
	, _position(HMM_Vec3(0, 0, 0))
	, _rotation(HMM_Quaternion(0, 0, 0, 0))
{

}

void renderer::scene::sokol::SokolCamera::set_transform(const renderer::scene::Transform &transform)
{
	_position = HMM_Vec3(transform.position.x,
						 transform.position.y,
						 transform.position.z
						);
	_rotation = HMM_Quaternion(
					transform.rotation.x,
					transform.rotation.y,
					transform.rotation.z,
					transform.rotation.w
				);
}

hmm_mat4 renderer::scene::sokol::SokolCamera::transform_mat(const MapDescription& desc) const
{
	int32_t width = desc.width;
	int32_t height = desc.height;
	hmm_vec3 camera = _position;

	camera.Y = height - camera.Y;
	camera.Z = camera.Z;
	hmm_mat4 model = HMM_Scale(HMM_Vec3(width, height, 1.0f));

	hmm_mat4 view = HMM_MultiplyMat4(
				HMM_QuaternionToMat4(_rotation),
				HMM_Translate(HMM_MultiplyVec3f(camera, -1))
				);

	hmm_mat4 view_model = HMM_MultiplyMat4(
							view,
							model
						  );

	return HMM_MultiplyMat4(
				_projection,
				view_model
		   );
}
