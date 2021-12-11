#ifndef LIB_RENDERER_SRC_RENDERER_LIB_SOKOL_GFX_IMPL
#define LIB_RENDERER_SRC_RENDERER_LIB_SOKOL_GFX_IMPL

#define SOKOL_IMPL
#include "sokol_gfx_settings.h"
#include "sokol_gfx.h"
#include "sokol_gfx_ext.h"

/*== DUMMY BACKEND IMPL ======================================================*/
#if defined(SOKOL_DUMMY_BACKEND)
_SOKOL_PRIVATE void _sg_bqq_update_subimage(_sg_image_t *dst_img, const sg_image_desc *desc, int dst_x, int dst_y, int dst_z)
{
}

/*== GL BACKEND ==============================================================*/
#elif defined(SOKOL_GLCORE33) || defined(SOKOL_GLES2) || defined(SOKOL_GLES3)
_SOKOL_PRIVATE void _gl_update_subimage(_sg_image_t *img, const sg_ext_image_update_data *data)
{
	SOKOL_ASSERT(img && data);
	// TODO:
//	/* only one update per image per frame allowed */
//	if (++img->cmn.active_slot >= img->cmn.num_slots) {
//		img->cmn.active_slot = 0;
//	}
	SOKOL_ASSERT(img->cmn.active_slot < SG_NUM_INFLIGHT_FRAMES);
	SOKOL_ASSERT(0 != img->gl.tex[img->cmn.active_slot]);
	_sg_gl_cache_store_texture_binding(0);
	_sg_gl_cache_bind_texture(0, img->gl.target, img->gl.tex[img->cmn.active_slot]);
	const GLenum gl_img_format = _sg_gl_teximage_format(img->cmn.pixel_format);
	const GLenum gl_img_type = _sg_gl_teximage_type(img->cmn.pixel_format);
	const int num_faces = img->cmn.type == SG_IMAGETYPE_CUBE ? 6 : 1;
	const int num_mips = img->cmn.num_mipmaps;
	for (int face_index = 0; face_index < num_faces; face_index++) {
		for (int mip_index = 0; mip_index < num_mips; mip_index++) {
			GLenum gl_img_target = img->gl.target;
			if (SG_IMAGETYPE_CUBE == img->cmn.type) {
				gl_img_target = _sg_gl_cubeface_target(face_index);
			}
			const GLvoid* data_ptr = data->data.subimage[face_index][mip_index].ptr;
			int mip_width = data->width >> mip_index;
			if (mip_width == 0) {
				mip_width = 1;
			}
			int mip_height = data->height >> mip_index;
			if (mip_height == 0) {
				mip_height = 1;
			}
			if ((SG_IMAGETYPE_2D == img->cmn.type) || (SG_IMAGETYPE_CUBE == img->cmn.type)) {
				glTexSubImage2D(gl_img_target, mip_index,
					data->x, data->y,
					mip_width, mip_height,
					gl_img_format, gl_img_type,
					data_ptr);
			}
			#if !defined(SOKOL_GLES2)
			else if (!_sg.gl.gles2 && ((SG_IMAGETYPE_3D == img->cmn.type) || (SG_IMAGETYPE_ARRAY == img->cmn.type))) {
				int mip_depth = data->num_slices >> mip_index;
				if (mip_depth == 0) {
					mip_depth = 1;
				}
				glTexSubImage3D(gl_img_target, mip_index,
					data->x, data->y, data->z,
					mip_width, mip_height, mip_depth,
					gl_img_format, gl_img_type,
					data_ptr);

			}
			#endif
		}
	}
	_sg_gl_cache_restore_texture_binding(0);
}


/*== D3D11 BACKEND IMPLEMENTATION ============================================*/
#elif defined(SOKOL_D3D11)

#endif

static inline void _sg_ext_update_subimage(_sg_image_t* img, const sg_ext_image_update_data *data) {
	#if defined(_SOKOL_ANY_GL)
	_gl_update_subimage(img, data);
	#elif defined(SOKOL_METAL)
	#error("NOT IMPLEMENTED");
	#elif defined(SOKOL_D3D11)
	#error("NOT IMPLEMENTED");
	#elif defined(SOKOL_WGPU)
	#error("NOT IMPLEMENTED");
	#elif defined(SOKOL_DUMMY_BACKEND)
	#error("NOT IMPLEMENTED");
	#else
	#error("INVALID BACKEND");
	#endif
}

_SOKOL_PRIVATE bool _sg_ext_validate_update_subimage(const _sg_image_t* img, const sg_ext_image_update_data *data) {
	#if !defined(SOKOL_DEBUG)
		_SOKOL_UNUSED(img);
		_SOKOL_UNUSED(data);
		return true;
	#else
		SOKOL_ASSERT(img && data);
		SOKOL_VALIDATE_BEGIN();
		SOKOL_VALIDATE(img->cmn.usage != SG_USAGE_IMMUTABLE, _SG_VALIDATE_UPDIMG_USAGE);
		SOKOL_VALIDATE(img->cmn.upd_frame_index != _sg.frame_index, _SG_VALIDATE_UPDIMG_ONCE);
		_sg_validate_image_data(&data->data,
			img->cmn.pixel_format,
			data->width,
			data->height,
			(img->cmn.type == SG_IMAGETYPE_CUBE) ? 6 : 1,
			img->cmn.num_mipmaps,
			data->num_slices);
		bool has_valid_size = data->x >= 0 &&
							  data->y >= 0 &&
							  data->z >= 0 &&
							  img->cmn.width >= data->width + data->x &&
							  img->cmn.height >= data->height + data->y &&
							  img->cmn.num_slices >= data->num_slices + data->z;
		SOKOL_VALIDATE(has_valid_size, _SG_VALIDATE_IMAGEDATA_DATA_SIZE);
		return SOKOL_VALIDATE_END();
	#endif
}

SOKOL_GFX_API_DECL void sg_ext_update_subimage(sg_image img_id, const sg_ext_image_update_data *data) {
	SOKOL_ASSERT(_sg.valid);
	SOKOL_ASSERT(data);

	_sg_image_t* img = _sg_lookup_image(&_sg.pools, img_id.id);
	if (img && img->slot.state == SG_RESOURCESTATE_VALID) {
		if (_sg_ext_validate_update_subimage(img, data)) {
			SOKOL_ASSERT(img->cmn.upd_frame_index != _sg.frame_index);
			_sg_ext_update_subimage(img, data);
			img->cmn.upd_frame_index = _sg.frame_index;
		}
	}
	_SG_TRACE_ARGS(update_image, img_id, data);
}



#endif /* LIB_RENDERER_SRC_RENDERER_LIB_SOKOL_GFX_IMPL */
