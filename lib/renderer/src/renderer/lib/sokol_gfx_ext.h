#ifndef RENDERER_LIB_SOKOL_GFX_EXT
#define RENDERER_LIB_SOKOL_GFX_EXT

typedef struct sg_ext_image_update_data {
	int x;
	int y;
	int z;
	int width;
	int height;
	int num_slices;
	sg_image_data data;
} sg_ext_image_update_data;

SOKOL_GFX_API_DECL void sg_ext_update_subimage(sg_image img, const sg_ext_image_update_data *data);

inline void sg_ext_update_subimage(sg_image img, const sg_ext_image_update_data& data){
	return sg_ext_update_subimage(img, &data);
}

#endif // RENDERER_LIB_SOKOL_GFX_EXT
