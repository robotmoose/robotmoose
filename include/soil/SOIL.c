/*
HACKED version by Orion Lawlor (removed DXT and DDS loading code)

Originally by:
	Jonathan Dummer
	2007-07-26-10.36

	Simple OpenGL Image Library

	Public Domain
	using Sean Barret's stb_image as a base

	Thanks to:
	* Sean Barret - for the awesome stb_image
	* Dan Venkitachalam - for finding some non-compliant DDS files, and patching some explicit casts
	* everybody at gamedev.net
*/

#define SOIL_CHECK_FOR_GL_ERRORS 0

#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <wingdi.h>
	#include <GL/gl.h>
#elif defined(__APPLE__) || defined(__APPLE_CC__)
	/*	I can't test this Apple stuff!	*/
	#include <OpenGL/gl.h>
	#include <Carbon/Carbon.h>
	#define APIENTRY
#else
	#include <GL/gl.h>
	/* #include <GL/glx.h>   not needed */
#endif

#include "SOIL.h"
#include "stb_image_aug.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>



/**
	This function upscales an image.
	Not to be used to create MIPmaps,
	but to make it square,
	or to make it a power-of-two sized.
**/
int
	up_scale_image
	(
		const unsigned char* const orig,
		int width, int height, int channels,
		unsigned char* resampled,
		int resampled_width, int resampled_height
	);

/**
	This function downscales an image.
	Used for creating MIPmaps,
	the incoming image should be a
	power-of-two sized.
**/
int
	mipmap_image
	(
		const unsigned char* const orig,
		int width, int height, int channels,
		unsigned char* resampled,
		int block_size_x, int block_size_y
	);

/**
	This function takes the RGB components of the image
	and scales each channel from [0,255] to [16,235].
	This makes the colors "Safe" for display on NTSC
	displays.  Note that this is _NOT_ a good idea for
	loading images like normal- or height-maps!
**/
int
	scale_image_RGB_to_NTSC_safe
	(
		unsigned char* orig,
		int width, int height, int channels
	);

/**
	This function takes the RGB components of the image
	and converts them into YCoCg.  3 components will be
	re-ordered to CoYCg (for optimum DXT1 compression),
	while 4 components will be ordered CoCgAY (for DXT5
	compression).
**/
int
	convert_RGB_to_YCoCg
	(
		unsigned char* orig,
		int width, int height, int channels
	);

/**
	This function takes the YCoCg components of the image
	and converts them into RGB.  See above.
**/
int
	convert_YCoCg_to_RGB
	(
		unsigned char* orig,
		int width, int height, int channels
	);

/**
	Converts an HDR image from an array
	of unsigned chars (RGBE) to RGBdivA
	\return 0 if failed, otherwise returns 1
**/
int
	RGBE_to_RGBdivA
	(
		unsigned char *image,
		int width, int height,
		int rescale_to_max
	);

/**
	Converts an HDR image from an array
	of unsigned chars (RGBE) to RGBdivA2
	\return 0 if failed, otherwise returns 1
**/
int
	RGBE_to_RGBdivA2
	(
		unsigned char *image,
		int width, int height,
		int rescale_to_max
	);




/*	error reporting	*/
const char *result_string_pointer = "SOIL initialized";

/*	for loading cube maps	*/
enum{
	SOIL_CAPABILITY_UNKNOWN = -1,
	SOIL_CAPABILITY_NONE = 0,
	SOIL_CAPABILITY_PRESENT = 1
};
static int has_cubemap_capability = SOIL_CAPABILITY_UNKNOWN;
int query_cubemap_capability( void );
#define SOIL_TEXTURE_WRAP_R					0x8072
#define SOIL_CLAMP_TO_EDGE					0x812F
#define SOIL_NORMAL_MAP						0x8511
#define SOIL_REFLECTION_MAP					0x8512
#define SOIL_TEXTURE_CUBE_MAP				0x8513
#define SOIL_TEXTURE_BINDING_CUBE_MAP		0x8514
#define SOIL_TEXTURE_CUBE_MAP_POSITIVE_X	0x8515
#define SOIL_TEXTURE_CUBE_MAP_NEGATIVE_X	0x8516
#define SOIL_TEXTURE_CUBE_MAP_POSITIVE_Y	0x8517
#define SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Y	0x8518
#define SOIL_TEXTURE_CUBE_MAP_POSITIVE_Z	0x8519
#define SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Z	0x851A
#define SOIL_PROXY_TEXTURE_CUBE_MAP			0x851B
#define SOIL_MAX_CUBE_MAP_TEXTURE_SIZE		0x851C
/*	for non-power-of-two texture	*/
static int has_NPOT_capability = SOIL_CAPABILITY_UNKNOWN;
int query_NPOT_capability( void );
/*	for texture rectangles	*/
static int has_tex_rectangle_capability = SOIL_CAPABILITY_UNKNOWN;
int query_tex_rectangle_capability( void );
#define SOIL_TEXTURE_RECTANGLE_ARB				0x84F5
#define SOIL_MAX_RECTANGLE_TEXTURE_SIZE_ARB		0x84F8
/*	other functions	*/
unsigned int
	SOIL_internal_create_OGL_texture
	(
		const unsigned char *const data,
		int width, int height, int channels,
		unsigned int reuse_texture_ID,
		unsigned int flags,
		unsigned int opengl_texture_type,
		unsigned int opengl_texture_target,
		unsigned int texture_check_size_enum,
		unsigned int internal_format=0
	);

/*	and the code magic begins here [8^)	*/
unsigned int
	SOIL_load_OGL_texture
	(
		const char *filename,
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags,
		unsigned int internal_format
	)
{
	/*	variables	*/
	unsigned char* img;
	int width, height, channels;
	unsigned int tex_id;
	
	/*	try to load the image	*/
	img = SOIL_load_image( filename, &width, &height, &channels, force_channels );
	/*	channels holds the original number of channels, which may have been forced	*/
	if( (force_channels >= 1) && (force_channels <= 4) )
	{
		channels = force_channels;
	}
	if( NULL == img )
	{
		/*	image loading failed	*/
		result_string_pointer = stbi_failure_reason();
		return 0;
	}
	/*	OK, make it a texture!	*/
	tex_id = SOIL_internal_create_OGL_texture(
			img, width, height, channels,
			reuse_texture_ID, flags,
			GL_TEXTURE_2D, GL_TEXTURE_2D,
			GL_MAX_TEXTURE_SIZE,
			internal_format );
	/*	and nuke the image data	*/
	SOIL_free_image_data( img );
	/*	and return the handle, such as it is	*/
	return tex_id;
}

unsigned int
	SOIL_load_OGL_HDR_texture
	(
		const char *filename,
		int fake_HDR_format,
		int rescale_to_max,
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	variables	*/
	unsigned char* img;
	int width, height, channels;
	unsigned int tex_id;
	/*	no direct uploading of the image as a DDS file	*/
	/* error check */
	if( (fake_HDR_format != SOIL_HDR_RGBE) &&
		(fake_HDR_format != SOIL_HDR_RGBdivA) &&
		(fake_HDR_format != SOIL_HDR_RGBdivA2) )
	{
		result_string_pointer = "Invalid fake HDR format specified";
		return 0;
	}
	/*	try to load the image (only the HDR type) */
	img = stbi_hdr_load_rgbe( filename, &width, &height, &channels, 4 );
	/*	channels holds the original number of channels, which may have been forced	*/
	if( NULL == img )
	{
		/*	image loading failed	*/
		result_string_pointer = stbi_failure_reason();
		return 0;
	}
	/* the load worked, do I need to convert it? */
	if( fake_HDR_format == SOIL_HDR_RGBdivA )
	{
		RGBE_to_RGBdivA( img, width, height, rescale_to_max );
	} else if( fake_HDR_format == SOIL_HDR_RGBdivA2 )
	{
		RGBE_to_RGBdivA2( img, width, height, rescale_to_max );
	}
	/*	OK, make it a texture!	*/
	tex_id = SOIL_internal_create_OGL_texture(
			img, width, height, channels,
			reuse_texture_ID, flags,
			GL_TEXTURE_2D, GL_TEXTURE_2D,
			GL_MAX_TEXTURE_SIZE );
	/*	and nuke the image data	*/
	SOIL_free_image_data( img );
	/*	and return the handle, such as it is	*/
	return tex_id;
}

unsigned int
	SOIL_load_OGL_texture_from_memory
	(
		const unsigned char *const buffer,
		int buffer_length,
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	variables	*/
	unsigned char* img;
	int width, height, channels;
	unsigned int tex_id;
	/*	try to load the image	*/
	img = SOIL_load_image_from_memory(
					buffer, buffer_length,
					&width, &height, &channels,
					force_channels );
	/*	channels holds the original number of channels, which may have been forced	*/
	if( (force_channels >= 1) && (force_channels <= 4) )
	{
		channels = force_channels;
	}
	if( NULL == img )
	{
		/*	image loading failed	*/
		result_string_pointer = stbi_failure_reason();
		return 0;
	}
	/*	OK, make it a texture!	*/
	tex_id = SOIL_internal_create_OGL_texture(
			img, width, height, channels,
			reuse_texture_ID, flags,
			GL_TEXTURE_2D, GL_TEXTURE_2D,
			GL_MAX_TEXTURE_SIZE );
	/*	and nuke the image data	*/
	SOIL_free_image_data( img );
	/*	and return the handle, such as it is	*/
	return tex_id;
}

unsigned int
	SOIL_load_OGL_cubemap
	(
		const char *x_pos_file,
		const char *x_neg_file,
		const char *y_pos_file,
		const char *y_neg_file,
		const char *z_pos_file,
		const char *z_neg_file,
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	variables	*/
	unsigned char* img;
	int width, height, channels;
	unsigned int tex_id;
	/*	error checking	*/
	if( (x_pos_file == NULL) ||
		(x_neg_file == NULL) ||
		(y_pos_file == NULL) ||
		(y_neg_file == NULL) ||
		(z_pos_file == NULL) ||
		(z_neg_file == NULL) )
	{
		result_string_pointer = "Invalid cube map files list";
		return 0;
	}
	/*	capability checking	*/
	if( query_cubemap_capability() != SOIL_CAPABILITY_PRESENT )
	{
		result_string_pointer = "No cube map capability present";
		return 0;
	}
	/*	1st face: try to load the image	*/
	img = SOIL_load_image( x_pos_file, &width, &height, &channels, force_channels );
	/*	channels holds the original number of channels, which may have been forced	*/
	if( (force_channels >= 1) && (force_channels <= 4) )
	{
		channels = force_channels;
	}
	if( NULL == img )
	{
		/*	image loading failed	*/
		result_string_pointer = stbi_failure_reason();
		return 0;
	}
	/*	upload the texture, and create a texture ID if necessary	*/
	tex_id = SOIL_internal_create_OGL_texture(
			img, width, height, channels,
			reuse_texture_ID, flags,
			SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_POSITIVE_X,
			SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
	/*	and nuke the image data	*/
	SOIL_free_image_data( img );
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image( x_neg_file, &width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, width, height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_NEGATIVE_X,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image( y_pos_file, &width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, width, height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_POSITIVE_Y,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image( y_neg_file, &width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, width, height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image( z_pos_file, &width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, width, height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_POSITIVE_Z,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image( z_neg_file, &width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, width, height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	and return the handle, such as it is	*/
	return tex_id;
}

unsigned int
	SOIL_load_OGL_cubemap_from_memory
	(
		const unsigned char *const x_pos_buffer,
		int x_pos_buffer_length,
		const unsigned char *const x_neg_buffer,
		int x_neg_buffer_length,
		const unsigned char *const y_pos_buffer,
		int y_pos_buffer_length,
		const unsigned char *const y_neg_buffer,
		int y_neg_buffer_length,
		const unsigned char *const z_pos_buffer,
		int z_pos_buffer_length,
		const unsigned char *const z_neg_buffer,
		int z_neg_buffer_length,
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	variables	*/
	unsigned char* img;
	int width, height, channels;
	unsigned int tex_id;
	/*	error checking	*/
	if( (x_pos_buffer == NULL) ||
		(x_neg_buffer == NULL) ||
		(y_pos_buffer == NULL) ||
		(y_neg_buffer == NULL) ||
		(z_pos_buffer == NULL) ||
		(z_neg_buffer == NULL) )
	{
		result_string_pointer = "Invalid cube map buffers list";
		return 0;
	}
	/*	capability checking	*/
	if( query_cubemap_capability() != SOIL_CAPABILITY_PRESENT )
	{
		result_string_pointer = "No cube map capability present";
		return 0;
	}
	/*	1st face: try to load the image	*/
	img = SOIL_load_image_from_memory(
			x_pos_buffer, x_pos_buffer_length,
			&width, &height, &channels, force_channels );
	/*	channels holds the original number of channels, which may have been forced	*/
	if( (force_channels >= 1) && (force_channels <= 4) )
	{
		channels = force_channels;
	}
	if( NULL == img )
	{
		/*	image loading failed	*/
		result_string_pointer = stbi_failure_reason();
		return 0;
	}
	/*	upload the texture, and create a texture ID if necessary	*/
	tex_id = SOIL_internal_create_OGL_texture(
			img, width, height, channels,
			reuse_texture_ID, flags,
			SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_POSITIVE_X,
			SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
	/*	and nuke the image data	*/
	SOIL_free_image_data( img );
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image_from_memory(
				x_neg_buffer, x_neg_buffer_length,
				&width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, width, height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_NEGATIVE_X,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image_from_memory(
				y_pos_buffer, y_pos_buffer_length,
				&width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, width, height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_POSITIVE_Y,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image_from_memory(
				y_neg_buffer, y_neg_buffer_length,
				&width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, width, height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image_from_memory(
				z_pos_buffer, z_pos_buffer_length,
				&width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, width, height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_POSITIVE_Z,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	continue?	*/
	if( tex_id != 0 )
	{
		/*	1st face: try to load the image	*/
		img = SOIL_load_image_from_memory(
				z_neg_buffer, z_neg_buffer_length,
				&width, &height, &channels, force_channels );
		/*	channels holds the original number of channels, which may have been forced	*/
		if( (force_channels >= 1) && (force_channels <= 4) )
		{
			channels = force_channels;
		}
		if( NULL == img )
		{
			/*	image loading failed	*/
			result_string_pointer = stbi_failure_reason();
			return 0;
		}
		/*	upload the texture, but reuse the assigned texture ID	*/
		tex_id = SOIL_internal_create_OGL_texture(
				img, width, height, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP, SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
		/*	and nuke the image data	*/
		SOIL_free_image_data( img );
	}
	/*	and return the handle, such as it is	*/
	return tex_id;
}

unsigned int
	SOIL_load_OGL_single_cubemap
	(
		const char *filename,
		const char face_order[6],
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	variables	*/
	unsigned char* img;
	int width, height, channels, i;
	unsigned int tex_id = 0;
	/*	error checking	*/
	if( filename == NULL )
	{
		result_string_pointer = "Invalid single cube map file name";
		return 0;
	}
	/*	face order checking	*/
	for( i = 0; i < 6; ++i )
	{
		if( (face_order[i] != 'N') &&
			(face_order[i] != 'S') &&
			(face_order[i] != 'W') &&
			(face_order[i] != 'E') &&
			(face_order[i] != 'U') &&
			(face_order[i] != 'D') )
		{
			result_string_pointer = "Invalid single cube map face order";
			return 0;
		};
	}
	/*	capability checking	*/
	if( query_cubemap_capability() != SOIL_CAPABILITY_PRESENT )
	{
		result_string_pointer = "No cube map capability present";
		return 0;
	}
	/*	1st off, try to load the full image	*/
	img = SOIL_load_image( filename, &width, &height, &channels, force_channels );
	/*	channels holds the original number of channels, which may have been forced	*/
	if( (force_channels >= 1) && (force_channels <= 4) )
	{
		channels = force_channels;
	}
	if( NULL == img )
	{
		/*	image loading failed	*/
		result_string_pointer = stbi_failure_reason();
		return 0;
	}
	/*	now, does this image have the right dimensions?	*/
	if( (width != 6*height) &&
		(6*width != height) )
	{
		SOIL_free_image_data( img );
		result_string_pointer = "Single cubemap image must have a 6:1 ratio";
		return 0;
	}
	/*	try the image split and create	*/
	tex_id = SOIL_create_OGL_single_cubemap(
			img, width, height, channels,
			face_order, reuse_texture_ID, flags
			);
	/*	nuke the temporary image data and return the texture handle	*/
	SOIL_free_image_data( img );
	return tex_id;
}

unsigned int
	SOIL_load_OGL_single_cubemap_from_memory
	(
		const unsigned char *const buffer,
		int buffer_length,
		const char face_order[6],
		int force_channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	variables	*/
	unsigned char* img;
	int width, height, channels, i;
	unsigned int tex_id = 0;
	/*	error checking	*/
	if( buffer == NULL )
	{
		result_string_pointer = "Invalid single cube map buffer";
		return 0;
	}
	/*	face order checking	*/
	for( i = 0; i < 6; ++i )
	{
		if( (face_order[i] != 'N') &&
			(face_order[i] != 'S') &&
			(face_order[i] != 'W') &&
			(face_order[i] != 'E') &&
			(face_order[i] != 'U') &&
			(face_order[i] != 'D') )
		{
			result_string_pointer = "Invalid single cube map face order";
			return 0;
		};
	}
	/*	capability checking	*/
	if( query_cubemap_capability() != SOIL_CAPABILITY_PRESENT )
	{
		result_string_pointer = "No cube map capability present";
		return 0;
	}
	/*	1st off, try to load the full image	*/
	img = SOIL_load_image_from_memory(
			buffer, buffer_length,
			&width, &height, &channels,
			force_channels );
	/*	channels holds the original number of channels, which may have been forced	*/
	if( (force_channels >= 1) && (force_channels <= 4) )
	{
		channels = force_channels;
	}
	if( NULL == img )
	{
		/*	image loading failed	*/
		result_string_pointer = stbi_failure_reason();
		return 0;
	}
	/*	now, does this image have the right dimensions?	*/
	if( (width != 6*height) &&
		(6*width != height) )
	{
		SOIL_free_image_data( img );
		result_string_pointer = "Single cubemap image must have a 6:1 ratio";
		return 0;
	}
	/*	try the image split and create	*/
	tex_id = SOIL_create_OGL_single_cubemap(
			img, width, height, channels,
			face_order, reuse_texture_ID, flags
			);
	/*	nuke the temporary image data and return the texture handle	*/
	SOIL_free_image_data( img );
	return tex_id;
}

unsigned int
	SOIL_create_OGL_single_cubemap
	(
		const unsigned char *const data,
		int width, int height, int channels,
		const char face_order[6],
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	variables	*/
	unsigned char* sub_img;
	int dw, dh, sz, i;
	unsigned int tex_id;
	/*	error checking	*/
	if( data == NULL )
	{
		result_string_pointer = "Invalid single cube map image data";
		return 0;
	}
	/*	face order checking	*/
	for( i = 0; i < 6; ++i )
	{
		if( (face_order[i] != 'N') &&
			(face_order[i] != 'S') &&
			(face_order[i] != 'W') &&
			(face_order[i] != 'E') &&
			(face_order[i] != 'U') &&
			(face_order[i] != 'D') )
		{
			result_string_pointer = "Invalid single cube map face order";
			return 0;
		};
	}
	/*	capability checking	*/
	if( query_cubemap_capability() != SOIL_CAPABILITY_PRESENT )
	{
		result_string_pointer = "No cube map capability present";
		return 0;
	}
	/*	now, does this image have the right dimensions?	*/
	if( (width != 6*height) &&
		(6*width != height) )
	{
		result_string_pointer = "Single cubemap image must have a 6:1 ratio";
		return 0;
	}
	/*	which way am I stepping?	*/
	if( width > height )
	{
		dw = height;
		dh = 0;
	} else
	{
		dw = 0;
		dh = width;
	}
	sz = dw+dh;
	sub_img = (unsigned char *)malloc( sz*sz*channels );
	/*	do the splitting and uploading	*/
	tex_id = reuse_texture_ID;
	for( i = 0; i < 6; ++i )
	{
		int x, y, idx = 0;
		unsigned int cubemap_target = 0;
		/*	copy in the sub-image	*/
		for( y = i*dh; y < i*dh+sz; ++y )
		{
			for( x = i*dw*channels; x < (i*dw+sz)*channels; ++x )
			{
				sub_img[idx++] = data[y*width*channels+x];
			}
		}
		/*	what is my texture target?
			remember, this coordinate system is
			LHS if viewed from inside the cube!	*/
		switch( face_order[i] )
		{
		case 'N':
			cubemap_target = SOIL_TEXTURE_CUBE_MAP_POSITIVE_Z;
			break;
		case 'S':
			cubemap_target = SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
			break;
		case 'W':
			cubemap_target = SOIL_TEXTURE_CUBE_MAP_NEGATIVE_X;
			break;
		case 'E':
			cubemap_target = SOIL_TEXTURE_CUBE_MAP_POSITIVE_X;
			break;
		case 'U':
			cubemap_target = SOIL_TEXTURE_CUBE_MAP_POSITIVE_Y;
			break;
		case 'D':
			cubemap_target = SOIL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
			break;
		}
		/*	upload it as a texture	*/
		tex_id = SOIL_internal_create_OGL_texture(
				sub_img, sz, sz, channels,
				tex_id, flags,
				SOIL_TEXTURE_CUBE_MAP,
				cubemap_target,
				SOIL_MAX_CUBE_MAP_TEXTURE_SIZE );
	}
	/*	and nuke the image and sub-image data	*/
	SOIL_free_image_data( sub_img );
	/*	and return the handle, such as it is	*/
	return tex_id;
}

unsigned int
	SOIL_create_OGL_texture
	(
		const unsigned char *const data,
		int width, int height, int channels,
		unsigned int reuse_texture_ID,
		unsigned int flags
	)
{
	/*	wrapper function for 2D textures	*/
	return SOIL_internal_create_OGL_texture(
				data, width, height, channels,
				reuse_texture_ID, flags,
				GL_TEXTURE_2D, GL_TEXTURE_2D,
				GL_MAX_TEXTURE_SIZE );
}

#if SOIL_CHECK_FOR_GL_ERRORS
void check_for_GL_errors( const char *calling_location )
{
	/*	check for errors	*/
	GLenum err_code = glGetError();
	while( GL_NO_ERROR != err_code )
	{
		printf( "OpenGL Error @ %s: %i", calling_location, err_code );
		err_code = glGetError();
	}
}
#else
void check_for_GL_errors( const char * /*calling_location*/ )
{
	/*	no check for errors	*/
}
#endif

unsigned int
	SOIL_internal_create_OGL_texture
	(
		const unsigned char *const data,
		int width, int height, int channels,
		unsigned int reuse_texture_ID,
		unsigned int flags,
		unsigned int opengl_texture_type,
		unsigned int opengl_texture_target,
		unsigned int texture_check_size_enum,
		unsigned int internal_format
	)
{
	/*	variables	*/
	unsigned char* img;
	unsigned int tex_id;
	unsigned int internal_texture_format = internal_format, original_texture_format = 0;
	int max_supported_size;
	/*	If the user wants to use the texture rectangle I kill a few flags	*/
	if( flags & SOIL_FLAG_TEXTURE_RECTANGLE )
	{
		/*	well, the user asked for it, can we do that?	*/
		if( query_tex_rectangle_capability() == SOIL_CAPABILITY_PRESENT )
		{
			/*	only allow this if the user in _NOT_ trying to do a cubemap!	*/
			if( opengl_texture_type == GL_TEXTURE_2D )
			{
				/*	clean out the flags that cannot be used with texture rectangles	*/
				flags &= ~(
						SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS |
						SOIL_FLAG_TEXTURE_REPEATS
					);
				/*	and change my target	*/
				opengl_texture_target = SOIL_TEXTURE_RECTANGLE_ARB;
				opengl_texture_type = SOIL_TEXTURE_RECTANGLE_ARB;
			} else
			{
				/*	not allowed for any other uses (yes, I'm looking at you, cubemaps!)	*/
				flags &= ~SOIL_FLAG_TEXTURE_RECTANGLE;
			}

		} else
		{
			/*	can't do it, and that is a breakable offense (uv coords use pixels instead of [0,1]!)	*/
			result_string_pointer = "Texture Rectangle extension unsupported";
			return 0;
		}
	}
	/*	create a copy the image data	*/
	img = (unsigned char*)malloc( width*height*channels );
	memcpy( img, data, width*height*channels );
	/*	does the user want me to invert the image?	*/
	if( flags & SOIL_FLAG_INVERT_Y )
	{
		int i, j;
		for( j = 0; j*2 < height; ++j )
		{
			int index1 = j * width * channels;
			int index2 = (height - 1 - j) * width * channels;
			for( i = width * channels; i > 0; --i )
			{
				unsigned char temp = img[index1];
				img[index1] = img[index2];
				img[index2] = temp;
				++index1;
				++index2;
			}
		}
	}
	/*	does the user want me to scale the colors into the NTSC safe RGB range?	*/
	if( flags & SOIL_FLAG_NTSC_SAFE_RGB )
	{
		scale_image_RGB_to_NTSC_safe( img, width, height, channels );
	}
	/*	does the user want me to convert from straight to pre-multiplied alpha?
		(and do we even _have_ alpha?)	*/
	if( flags & SOIL_FLAG_MULTIPLY_ALPHA )
	{
		int i;
		switch( channels )
		{
		case 2:
			for( i = 0; i < 2*width*height; i += 2 )
			{
				img[i] = (img[i] * img[i+1] + 128) >> 8;
			}
			break;
		case 4:
			for( i = 0; i < 4*width*height; i += 4 )
			{
				img[i+0] = (img[i+0] * img[i+3] + 128) >> 8;
				img[i+1] = (img[i+1] * img[i+3] + 128) >> 8;
				img[i+2] = (img[i+2] * img[i+3] + 128) >> 8;
			}
			break;
		default:
			/*	no other number of channels contains alpha data	*/
			break;
		}
	}
	/*	if the user can't support NPOT textures, make sure we force the POT option	*/
	if( (query_NPOT_capability() == SOIL_CAPABILITY_NONE) &&
		!(flags & SOIL_FLAG_TEXTURE_RECTANGLE) )
	{
		/*	add in the POT flag */
		flags |= SOIL_FLAG_POWER_OF_TWO;
	}
	/*	how large of a texture can this OpenGL implementation handle?	*/
	/*	texture_check_size_enum will be GL_MAX_TEXTURE_SIZE or SOIL_MAX_CUBE_MAP_TEXTURE_SIZE	*/
	glGetIntegerv( texture_check_size_enum, &max_supported_size );
	/*	do I need to make it a power of 2?	*/
	if(
		(flags & SOIL_FLAG_POWER_OF_TWO) ||	/*	user asked for it	*/
		(flags & SOIL_FLAG_MIPMAPS) ||		/*	need it for the MIP-maps	*/
		(width > max_supported_size) ||		/*	it's too big, (make sure it's	*/
		(height > max_supported_size) )		/*	2^n for later down-sampling)	*/
	{
		int new_width = 1;
		int new_height = 1;
		while( new_width < width )
		{
			new_width *= 2;
		}
		while( new_height < height )
		{
			new_height *= 2;
		}
		/*	still?	*/
		if( (new_width != width) || (new_height != height) )
		{
			/*	yep, resize	*/
			unsigned char *resampled = (unsigned char*)malloc( channels*new_width*new_height );
			up_scale_image(
					img, width, height, channels,
					resampled, new_width, new_height );
			/*	OJO	this is for debug only!	*/
			/*
			SOIL_save_image( "\\showme.bmp", SOIL_SAVE_TYPE_BMP,
							new_width, new_height, channels,
							resampled );
			*/
			/*	nuke the old guy, then point it at the new guy	*/
			SOIL_free_image_data( img );
			img = resampled;
			width = new_width;
			height = new_height;
		}
	}
	/*	now, if it is too large...	*/
	if( (width > max_supported_size) || (height > max_supported_size) )
	{
		/*	I've already made it a power of two, so simply use the MIPmapping
			code to reduce its size to the allowable maximum.	*/
		unsigned char *resampled;
		int reduce_block_x = 1, reduce_block_y = 1;
		int new_width, new_height;
		if( width > max_supported_size )
		{
			reduce_block_x = width / max_supported_size;
		}
		if( height > max_supported_size )
		{
			reduce_block_y = height / max_supported_size;
		}
		new_width = width / reduce_block_x;
		new_height = height / reduce_block_y;
		resampled = (unsigned char*)malloc( channels*new_width*new_height );
		/*	perform the actual reduction	*/
		mipmap_image(	img, width, height, channels,
						resampled, reduce_block_x, reduce_block_y );
		/*	nuke the old guy, then point it at the new guy	*/
		SOIL_free_image_data( img );
		img = resampled;
		width = new_width;
		height = new_height;
	}
	/*	does the user want us to use YCoCg color space?	*/
	if( flags & SOIL_FLAG_CoCg_Y )
	{
		/*	this will only work with RGB and RGBA images */
		convert_RGB_to_YCoCg( img, width, height, channels );
	}
	/*	create the OpenGL texture ID handle
    	(note: allowing a forced texture ID lets me reload a texture)	*/
    tex_id = reuse_texture_ID;
    if( tex_id == 0 )
    {
		glGenTextures( 1, &tex_id );
    }
	check_for_GL_errors( "glGenTextures" );
	/* Note: sometimes glGenTextures fails (usually no OpenGL context)	*/
	if( tex_id )
	{
		/*	and what type am I using as the internal texture format?	*/
		switch( channels )
		{
		case 1:
			original_texture_format = GL_LUMINANCE;
			break;
		case 2:
			original_texture_format = GL_LUMINANCE_ALPHA;
			break;
		case 3:
			original_texture_format = GL_RGB;
			break;
		case 4:
			original_texture_format = GL_RGBA;
			break;
		}
		if (internal_texture_format ==0) 
			internal_texture_format = original_texture_format;
		/*  bind an OpenGL texture ID	*/
		glBindTexture( opengl_texture_type, tex_id );
		check_for_GL_errors( "glBindTexture" );
		/*	user want OpenGL to do all the work!	*/
		glTexImage2D(
			opengl_texture_target, 0,
			internal_texture_format, width, height, 0,
			original_texture_format, GL_UNSIGNED_BYTE, img );
		check_for_GL_errors( "glTexImage2D" );
			
		/*	are any MIPmaps desired?	*/
		if( flags & SOIL_FLAG_MIPMAPS )
		{
			int MIPlevel = 1;
			int MIPwidth = (width+1) / 2;
			int MIPheight = (height+1) / 2;
			unsigned char *resampled = (unsigned char*)malloc( channels*MIPwidth*MIPheight );
			while( ((1<<MIPlevel) <= width) || ((1<<MIPlevel) <= height) )
			{
				/*	do this MIPmap level	*/
				mipmap_image(
						img, width, height, channels,
						resampled,
						(1 << MIPlevel), (1 << MIPlevel) );
				/*	user want OpenGL to do all the work!	*/
				glTexImage2D(
					opengl_texture_target, MIPlevel,
					internal_texture_format, MIPwidth, MIPheight, 0,
					original_texture_format, GL_UNSIGNED_BYTE, resampled );
				check_for_GL_errors( "glTexImage2D" );
					
				/*	prep for the next level	*/
				++MIPlevel;
				MIPwidth = (MIPwidth + 1) / 2;
				MIPheight = (MIPheight + 1) / 2;
			}
			SOIL_free_image_data( resampled );
			/*	instruct OpenGL to use the MIPmaps	*/
			glTexParameteri( opengl_texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( opengl_texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
			check_for_GL_errors( "GL_TEXTURE_MIN/MAG_FILTER" );
		} else
		{
			/*	instruct OpenGL _NOT_ to use the MIPmaps	*/
			glTexParameteri( opengl_texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			glTexParameteri( opengl_texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			check_for_GL_errors( "GL_TEXTURE_MIN/MAG_FILTER" );
		}
		/*	does the user want clamping, or wrapping?	*/
		if( flags & SOIL_FLAG_TEXTURE_REPEATS )
		{
			glTexParameteri( opengl_texture_type, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameteri( opengl_texture_type, GL_TEXTURE_WRAP_T, GL_REPEAT );
			if( opengl_texture_type == SOIL_TEXTURE_CUBE_MAP )
			{
				/*	SOIL_TEXTURE_WRAP_R is invalid if cubemaps aren't supported	*/
				glTexParameteri( opengl_texture_type, SOIL_TEXTURE_WRAP_R, GL_REPEAT );
			}
			check_for_GL_errors( "GL_TEXTURE_WRAP_*" );
		} else
		{
			/*	unsigned int clamp_mode = SOIL_CLAMP_TO_EDGE;	*/
			unsigned int clamp_mode = GL_CLAMP;
			glTexParameteri( opengl_texture_type, GL_TEXTURE_WRAP_S, clamp_mode );
			glTexParameteri( opengl_texture_type, GL_TEXTURE_WRAP_T, clamp_mode );
			if( opengl_texture_type == SOIL_TEXTURE_CUBE_MAP )
			{
				/*	SOIL_TEXTURE_WRAP_R is invalid if cubemaps aren't supported	*/
				glTexParameteri( opengl_texture_type, SOIL_TEXTURE_WRAP_R, clamp_mode );
			}
			check_for_GL_errors( "GL_TEXTURE_WRAP_*" );
		}
		/*	done	*/
		result_string_pointer = "Image loaded as an OpenGL texture";
	} else
	{
		/*	failed	*/
		result_string_pointer = "Failed to generate an OpenGL texture name; missing OpenGL context?";
	}
	SOIL_free_image_data( img );
	return tex_id;
}

int
	SOIL_save_screenshot
	(
		const char *filename,
		int image_type,
		int x, int y,
		int width, int height
	)
{
	unsigned char *pixel_data;
	int i, j;
	int save_result;

	/*	error checks	*/
	if( (width < 1) || (height < 1) )
	{
		result_string_pointer = "Invalid screenshot dimensions";
		return 0;
	}
	if( (x < 0) || (y < 0) )
	{
		result_string_pointer = "Invalid screenshot location";
		return 0;
	}
	if( filename == NULL )
	{
		result_string_pointer = "Invalid screenshot filename";
		return 0;
	}

    /*  Get the data from OpenGL	*/
    pixel_data = (unsigned char*)malloc( 3*width*height );
    glReadPixels (x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixel_data);

    /*	invert the image	*/
    for( j = 0; j*2 < height; ++j )
	{
		int index1 = j * width * 3;
		int index2 = (height - 1 - j) * width * 3;
		for( i = width * 3; i > 0; --i )
		{
			unsigned char temp = pixel_data[index1];
			pixel_data[index1] = pixel_data[index2];
			pixel_data[index2] = temp;
			++index1;
			++index2;
		}
	}

    /*	save the image	*/
    save_result = SOIL_save_image( filename, image_type, width, height, 3, pixel_data);

    /*  And free the memory	*/
    SOIL_free_image_data( pixel_data );
	return save_result;
}

unsigned char*
	SOIL_load_image
	(
		const char *filename,
		int *width, int *height, int *channels,
		int force_channels
	)
{
	unsigned char *result = stbi_load( filename,
			width, height, channels, force_channels );
	if( result == NULL )
	{
		result_string_pointer = stbi_failure_reason();
	} else
	{
		result_string_pointer = "Image loaded";
	}
	return result;
}

unsigned char*
	SOIL_load_image_from_memory
	(
		const unsigned char *const buffer,
		int buffer_length,
		int *width, int *height, int *channels,
		int force_channels
	)
{
	unsigned char *result = stbi_load_from_memory(
				buffer, buffer_length,
				width, height, channels,
				force_channels );
	if( result == NULL )
	{
		result_string_pointer = stbi_failure_reason();
	} else
	{
		result_string_pointer = "Image loaded from memory";
	}
	return result;
}

int
	SOIL_save_image
	(
		const char *filename,
		int image_type,
		int width, int height, int channels,
		const unsigned char *const data
	)
{
	int save_result;

	/*	error check	*/
	if( (width < 1) || (height < 1) ||
		(channels < 1) || (channels > 4) ||
		(data == NULL) ||
		(filename == NULL) )
	{
		return 0;
	}
	if( image_type == SOIL_SAVE_TYPE_BMP )
	{
		save_result = stbi_write_bmp( filename,
				width, height, channels, (void*)data );
	} else
	if( image_type == SOIL_SAVE_TYPE_TGA )
	{
		save_result = stbi_write_tga( filename,
				width, height, channels, (void*)data );
	} else
	{
		save_result = 0;
	}
	if( save_result == 0 )
	{
		result_string_pointer = "Saving the image failed";
	} else
	{
		result_string_pointer = "Image saved";
	}
	return save_result;
}

void
	SOIL_free_image_data
	(
		unsigned char *img_data
	)
{
	free( (void*)img_data );
}

const char*
	SOIL_last_result
	(
		void
	)
{
	return result_string_pointer;
}


int query_NPOT_capability( void )
{
	/*	check for the capability	*/
	if( has_NPOT_capability == SOIL_CAPABILITY_UNKNOWN )
	{
		/*	we haven't yet checked for the capability, do so	*/
		if(
			(NULL == strstr( (char const*)glGetString( GL_EXTENSIONS ),
				"GL_ARB_texture_non_power_of_two" ) )
			)
		{
			/*	not there, flag the failure	*/
			has_NPOT_capability = SOIL_CAPABILITY_NONE;
		} else
		{
			/*	it's there!	*/
			has_NPOT_capability = SOIL_CAPABILITY_PRESENT;
		}
	}
	/*	let the user know if we can do non-power-of-two textures or not	*/
	return has_NPOT_capability;
}

int query_tex_rectangle_capability( void )
{
	/*	check for the capability	*/
	if( has_tex_rectangle_capability == SOIL_CAPABILITY_UNKNOWN )
	{
		/*	we haven't yet checked for the capability, do so	*/
		if(
			(NULL == strstr( (char const*)glGetString( GL_EXTENSIONS ),
				"GL_ARB_texture_rectangle" ) )
		&&
			(NULL == strstr( (char const*)glGetString( GL_EXTENSIONS ),
				"GL_EXT_texture_rectangle" ) )
		&&
			(NULL == strstr( (char const*)glGetString( GL_EXTENSIONS ),
				"GL_NV_texture_rectangle" ) )
			)
		{
			/*	not there, flag the failure	*/
			has_tex_rectangle_capability = SOIL_CAPABILITY_NONE;
		} else
		{
			/*	it's there!	*/
			has_tex_rectangle_capability = SOIL_CAPABILITY_PRESENT;
		}
	}
	/*	let the user know if we can do texture rectangles or not	*/
	return has_tex_rectangle_capability;
}

int query_cubemap_capability( void )
{
	/*	check for the capability	*/
	if( has_cubemap_capability == SOIL_CAPABILITY_UNKNOWN )
	{
		/*	we haven't yet checked for the capability, do so	*/
		if(
			(NULL == strstr( (char const*)glGetString( GL_EXTENSIONS ),
				"GL_ARB_texture_cube_map" ) )
		&&
			(NULL == strstr( (char const*)glGetString( GL_EXTENSIONS ),
				"GL_EXT_texture_cube_map" ) )
			)
		{
			/*	not there, flag the failure	*/
			has_cubemap_capability = SOIL_CAPABILITY_NONE;
		} else
		{
			/*	it's there!	*/
			has_cubemap_capability = SOIL_CAPABILITY_PRESENT;
		}
	}
	/*	let the user know if we can do cubemaps or not	*/
	return has_cubemap_capability;
}
/*
    Jonathan Dummer

    image helper functions

    MIT license
*/

/*	Upscaling the image uses simple bilinear interpolation	*/
int
	up_scale_image
	(
		const unsigned char* const orig,
		int width, int height, int channels,
		unsigned char* resampled,
		int resampled_width, int resampled_height
	)
{
	float dx, dy;
	int x, y, c;

    /* error(s) check	*/
    if ( 	(width < 1) || (height < 1) ||
            (resampled_width < 2) || (resampled_height < 2) ||
            (channels < 1) ||
            (NULL == orig) || (NULL == resampled) )
    {
        /*	signify badness	*/
        return 0;
    }
    /*
		for each given pixel in the new map, find the exact location
		from the original map which would contribute to this guy
	*/
    dx = (width - 1.0f) / (resampled_width - 1.0f);
    dy = (height - 1.0f) / (resampled_height - 1.0f);
    for ( y = 0; y < resampled_height; ++y )
    {
    	/* find the base y index and fractional offset from that	*/
    	float sampley = y * dy;
    	int inty = (int)sampley;
    	/*	if( inty < 0 ) { inty = 0; } else	*/
		if( inty > height - 2 ) { inty = height - 2; }
		sampley -= inty;
        for ( x = 0; x < resampled_width; ++x )
        {
			float samplex = x * dx;
			int intx = (int)samplex;
			int base_index;
			/* find the base x index and fractional offset from that	*/
			/*	if( intx < 0 ) { intx = 0; } else	*/
			if( intx > width - 2 ) { intx = width - 2; }
			samplex -= intx;
			/*	base index into the original image	*/
			base_index = (inty * width + intx) * channels;
            for ( c = 0; c < channels; ++c )
            {
            	/*	do the sampling	*/
				float value = 0.5f;
				value += orig[base_index]
							*(1.0f-samplex)*(1.0f-sampley);
				value += orig[base_index+channels]
							*(samplex)*(1.0f-sampley);
				value += orig[base_index+width*channels]
							*(1.0f-samplex)*(sampley);
				value += orig[base_index+width*channels+channels]
							*(samplex)*(sampley);
				/*	move to the next channel	*/
				++base_index;
            	/*	save the new value	*/
            	resampled[y*resampled_width*channels+x*channels+c] =
						(unsigned char)(value);
            }
        }
    }
    /*	done	*/
    return 1;
}

int
	mipmap_image
	(
		const unsigned char* const orig,
		int width, int height, int channels,
		unsigned char* resampled,
		int block_size_x, int block_size_y
	)
{
	int mip_width, mip_height;
	int i, j, c;

	/*	error check	*/
	if( (width < 1) || (height < 1) ||
		(channels < 1) || (orig == NULL) ||
		(resampled == NULL) ||
		(block_size_x < 1) || (block_size_y < 1) )
	{
		/*	nothing to do	*/
		return 0;
	}
	mip_width = width / block_size_x;
	mip_height = height / block_size_y;
	if( mip_width < 1 )
	{
		mip_width = 1;
	}
	if( mip_height < 1 )
	{
		mip_height = 1;
	}
	for( j = 0; j < mip_height; ++j )
	{
		for( i = 0; i < mip_width; ++i )
		{
			for( c = 0; c < channels; ++c )
			{
				const int index = (j*block_size_y)*width*channels + (i*block_size_x)*channels + c;
				int sum_value;
				int u,v;
				int u_block = block_size_x;
				int v_block = block_size_y;
				int block_area;
				/*	do a bit of checking so we don't over-run the boundaries
					(necessary for non-square textures!)	*/
				if( block_size_x * (i+1) > width )
				{
					u_block = width - i*block_size_y;
				}
				if( block_size_y * (j+1) > height )
				{
					v_block = height - j*block_size_y;
				}
				block_area = u_block*v_block;
				/*	for this pixel, see what the average
					of all the values in the block are.
					note: start the sum at the rounding value, not at 0	*/
				sum_value = block_area >> 1;
				for( v = 0; v < v_block; ++v )
				for( u = 0; u < u_block; ++u )
				{
					sum_value += orig[index + v*width*channels + u*channels];
				}
				resampled[j*mip_width*channels + i*channels + c] = (unsigned char)(sum_value / block_area);
			}
		}
	}
	return 1;
}

int
	scale_image_RGB_to_NTSC_safe
	(
		unsigned char* orig,
		int width, int height, int channels
	)
{
	const float scale_lo = 16.0f - 0.499f;
	const float scale_hi = 235.0f + 0.499f;
	int i, j;
	int nc = channels;
	unsigned char scale_LUT[256];
	/*	error check	*/
	if( (width < 1) || (height < 1) ||
		(channels < 1) || (orig == NULL) )
	{
		/*	nothing to do	*/
		return 0;
	}
	/*	set up the scaling Look Up Table	*/
	for( i = 0; i < 256; ++i )
	{
		scale_LUT[i] = (unsigned char)((scale_hi - scale_lo) * i / 255.0f + scale_lo);
	}
	/*	for channels = 2 or 4, ignore the alpha component	*/
	nc -= 1 - (channels & 1);
	/*	OK, go through the image and scale any non-alpha components	*/
	for( i = 0; i < width*height*channels; i += channels )
	{
		for( j = 0; j < nc; ++j )
		{
			orig[i+j] = scale_LUT[orig[i+j]];
		}
	}
	return 1;
}

unsigned char clamp_byte( int x ) { 
	return (unsigned char)( (x) < 0 ? (0) : ( (x) > 255 ? 255 : (x) ) ); 
}

/*
	This function takes the RGB components of the image
	and converts them into YCoCg.  3 components will be
	re-ordered to CoYCg (for optimum DXT1 compression),
	while 4 components will be ordered CoCgAY (for DXT5
	compression).
*/
int
	convert_RGB_to_YCoCg
	(
		unsigned char* orig,
		int width, int height, int channels
	)
{
	int i;
	/*	error check	*/
	if( (width < 1) || (height < 1) ||
		(channels < 3) || (channels > 4) ||
		(orig == NULL) )
	{
		/*	nothing to do	*/
		return -1;
	}
	/*	do the conversion	*/
	if( channels == 3 )
	{
		for( i = 0; i < width*height*3; i += 3 )
		{
			int r = orig[i+0];
			int g = (orig[i+1] + 1) >> 1;
			int b = orig[i+2];
			int tmp = (2 + r + b) >> 2;
			/*	Co	*/
			orig[i+0] = clamp_byte( 128 + ((r - b + 1) >> 1) );
			/*	Y	*/
			orig[i+1] = clamp_byte( g + tmp );
			/*	Cg	*/
			orig[i+2] = clamp_byte( 128 + g - tmp );
		}
	} else
	{
		for( i = 0; i < width*height*4; i += 4 )
		{
			int r = orig[i+0];
			int g = (orig[i+1] + 1) >> 1;
			int b = orig[i+2];
			unsigned char a = orig[i+3];
			int tmp = (2 + r + b) >> 2;
			/*	Co	*/
			orig[i+0] = clamp_byte( 128 + ((r - b + 1) >> 1) );
			/*	Cg	*/
			orig[i+1] = clamp_byte( 128 + g - tmp );
			/*	Alpha	*/
			orig[i+2] = a;
			/*	Y	*/
			orig[i+3] = clamp_byte( g + tmp );
		}
	}
	/*	done	*/
	return 0;
}

/*
	This function takes the YCoCg components of the image
	and converts them into RGB.  See above.
*/
int
	convert_YCoCg_to_RGB
	(
		unsigned char* orig,
		int width, int height, int channels
	)
{
	int i;
	/*	error check	*/
	if( (width < 1) || (height < 1) ||
		(channels < 3) || (channels > 4) ||
		(orig == NULL) )
	{
		/*	nothing to do	*/
		return -1;
	}
	/*	do the conversion	*/
	if( channels == 3 )
	{
		for( i = 0; i < width*height*3; i += 3 )
		{
			int co = orig[i+0] - 128;
			int y  = orig[i+1];
			int cg = orig[i+2] - 128;
			/*	R	*/
			orig[i+0] = clamp_byte( y + co - cg );
			/*	G	*/
			orig[i+1] = clamp_byte( y + cg );
			/*	B	*/
			orig[i+2] = clamp_byte( y - co - cg );
		}
	} else
	{
		for( i = 0; i < width*height*4; i += 4 )
		{
			int co = orig[i+0] - 128;
			int cg = orig[i+1] - 128;
			unsigned char a  = orig[i+2];
			int y  = orig[i+3];
			/*	R	*/
			orig[i+0] = clamp_byte( y + co - cg );
			/*	G	*/
			orig[i+1] = clamp_byte( y + cg );
			/*	B	*/
			orig[i+2] = clamp_byte( y - co - cg );
			/*	A	*/
			orig[i+3] = a;
		}
	}
	/*	done	*/
	return 0;
}

float
find_max_RGBE
(
	unsigned char *image,
    int width, int height
)
{
	float max_val = 0.0f;
	unsigned char *img = image;
	int i, j;
	for( i = width * height; i > 0; --i )
	{
		/* float scale = powf( 2.0f, img[3] - 128.0f ) / 255.0f; */
		float scale = ldexp( 1.0f / 255.0f, (int)(img[3]) - 128 );
		for( j = 0; j < 3; ++j )
		{
			if( img[j] * scale > max_val )
			{
				max_val = img[j] * scale;
			}
		}
		/* next pixel */
		img += 4;
	}
	return max_val;
}

int
RGBE_to_RGBdivA
(
    unsigned char *image,
    int width, int height,
    int rescale_to_max
)
{
	/* local variables */
	int i, iv;
	unsigned char *img = image;
	float scale = 1.0f;
	/* error check */
	if( (!image) || (width < 1) || (height < 1) )
	{
		return 0;
	}
	/* convert (note: no negative numbers, but 0.0 is possible) */
	if( rescale_to_max )
	{
		scale = 255.0f / find_max_RGBE( image, width, height );
	}
	for( i = width * height; i > 0; --i )
	{
		/* decode this pixel, and find the max */
		float r,g,b,e, m;
		/* e = scale * powf( 2.0f, img[3] - 128.0f ) / 255.0f; */
		e = scale * ldexp( 1.0f / 255.0f, (int)(img[3]) - 128 );
		r = e * img[0];
		g = e * img[1];
		b = e * img[2];
		m = (r > g) ? r : g;
		m = (b > m) ? b : m;
		/* and encode it into RGBdivA */
		iv = (int)((m != 0.0f) ? (int)(255.0f / m) : 1.0f);
		iv = (iv < 1) ? 1 : iv;
		img[3] = (unsigned char)((iv > 255) ? 255 : iv);
		iv = (int)(img[3] * r + 0.5f);
		img[0] = (unsigned char)((iv > 255) ? 255 : iv);
		iv = (int)(img[3] * g + 0.5f);
		img[1] = (unsigned char)((iv > 255) ? 255 : iv);
		iv = (int)(img[3] * b + 0.5f);
		img[2] = (unsigned char)((iv > 255) ? 255 : iv);
		/* and on to the next pixel */
		img += 4;
	}
	return 1;
}

int
RGBE_to_RGBdivA2
(
    unsigned char *image,
    int width, int height,
    int rescale_to_max
)
{
	/* local variables */
	int i, iv;
	unsigned char *img = image;
	float scale = 1.0f;
	/* error check */
	if( (!image) || (width < 1) || (height < 1) )
	{
		return 0;
	}
	/* convert (note: no negative numbers, but 0.0 is possible) */
	if( rescale_to_max )
	{
		scale = 255.0f * 255.0f / find_max_RGBE( image, width, height );
	}
	for( i = width * height; i > 0; --i )
	{
		/* decode this pixel, and find the max */
		float r,g,b,e, m;
		/* e = scale * powf( 2.0f, img[3] - 128.0f ) / 255.0f; */
		e = scale * ldexp( 1.0f / 255.0f, (int)(img[3]) - 128 );
		r = e * img[0];
		g = e * img[1];
		b = e * img[2];
		m = (r > g) ? r : g;
		m = (b > m) ? b : m;
		/* and encode it into RGBdivA */
		iv = (int)((m != 0.0f) ? (int)sqrtf( 255.0f * 255.0f / m ) : 1.0f);
		iv = (iv < 1) ? 1 : iv;
		img[3] = (unsigned char)((iv > 255) ? 255 : iv);
		iv = (int)(img[3] * img[3] * r / 255.0f + 0.5f);
		img[0] = (unsigned char)((iv > 255) ? 255 : iv);
		iv = (int)(img[3] * img[3] * g / 255.0f + 0.5f);
		img[1] = (unsigned char)((iv > 255) ? 255 : iv);
		iv = (int)(img[3] * img[3] * b / 255.0f + 0.5f);
		img[2] = (unsigned char)((iv > 255) ? 255 : iv);
		/* and on to the next pixel */
		img += 4;
	}
	return 1;
}
