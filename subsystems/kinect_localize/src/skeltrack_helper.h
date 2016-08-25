// Helper functions for Skeltrack taken from the provided test-kinect.c.

#ifndef SKELTRACK_HELPER_H_INCLUDED
#define SKELTRACK_HELPER_H_INCLUDED

#include <glib-object.h>

static const uint16_t THRESHOLD_BEGIN = 500;
/* Adjust this value to increase of decrease
   the threshold */
static const uint16_t THRESHOLD_END = 2000;

typedef struct {
	uint16_t * reduced_buffer;
	uint16_t width;
	uint16_t height;
	uint16_t reduced_width;
	uint16_t reduced_height;
} BufferInfo;

static BufferInfo * process_buffer (
	uint16_t * buffer,
    uint16_t width,
    uint16_t height,
  	uint16_t dimension_factor,
    uint16_t threshold_begin,
    uint16_t threshold_end,
    bool camera_upside_down
){
 	g_return_val_if_fail (buffer != NULL, NULL);
  	uint16_t reduced_width = (width - width % dimension_factor) / dimension_factor;
 	uint16_t reduced_height = (height - height % dimension_factor) / dimension_factor;

 	uint16_t * reduced_buffer = new uint16_t [reduced_width * reduced_height];

  	for (int i = 0; i < reduced_width; ++i) {
	  	for (int j = 0; j < reduced_height; ++j) {
	  		int index;
	  		if(camera_upside_down)
	  			index = (reduced_height-1-j) * width * dimension_factor - i * dimension_factor;
	  		else
		    	index = j * width * dimension_factor + i * dimension_factor;
		    uint16_t value = buffer[index];

		    if (value < threshold_begin || value > threshold_end) {
		        reduced_buffer[j * reduced_width + i] = 0;
		        continue;
		    }
		      	reduced_buffer[j * reduced_width + i] = value;
		}
    }
    BufferInfo *buffer_info = new BufferInfo;
	buffer_info->reduced_buffer = reduced_buffer;
	buffer_info->reduced_width = reduced_width;
	buffer_info->reduced_height = reduced_height;
	buffer_info->width = width;
	buffer_info->height = height;

 	return buffer_info;
}
#endif // SKELTRACK_HELPER_H_INCLUDED