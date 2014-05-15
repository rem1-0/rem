/*****************************************************************************
 * rgb_image.h: Definition of the RGB image provided to the implementer
 *****************************************************************************
 * Copyright (C) 2014 Jean-Christophe Lechenet and Benoit Gastinne
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#ifndef RGB_IMAGE_H
#define RGB_IMAGE_H

#include <stdlib.h>
#include <stdint.h>

typedef struct
{
    // the three planes that compose the image (padded_lines*padded_pitch bytes
    // for each)
    uint8_t* r_plane;
    uint8_t* g_plane;
    uint8_t* b_plane;

    // number of lines and number of pixels on each line
    uint32_t lines;
    uint32_t pitch;
} planar_rgb_image_t;

static inline planar_rgb_image_t new_planar_rgb_image(uint32_t lines,
                                                      uint32_t pitch)
{
	planar_rgb_image_t image = {
			(uint8_t*) malloc(sizeof(uint8_t)*lines*pitch),
			(uint8_t*) malloc(sizeof(uint8_t)*lines*pitch),
			(uint8_t*) malloc(sizeof(uint8_t)*lines*pitch),
			lines,
			pitch
	};

	return image;
}

static inline void free_planar_rgb_image(planar_rgb_image_t image)
{
	if (image.r_plane) free(image.r_plane);
	if (image.g_plane) free(image.g_plane);
	if (image.b_plane) free(image.b_plane);
}


typedef union
{
	uint32_t packed;
	struct
	{
		uint8_t b;
		uint8_t g;
		uint8_t r;
		uint8_t padding;
	} unpacked;
} rgb32_pixel_t;

typedef struct
{
	rgb32_pixel_t* plane;

    // number of lines and number of pixels on each line
    uint32_t lines;
    uint32_t padded_lines;

    uint32_t pitch;
    uint32_t padded_pitch;
} packed_rgb32_image_t;

static inline packed_rgb32_image_t new_packed_rgb32_rgb_image(uint32_t lines,
                                                              uint32_t pitch)
{
	packed_rgb32_image_t image = {
			(rgb32_pixel_t*) malloc(sizeof(packed_rgb32_image_t)*lines*pitch),
			lines,
			lines,
			pitch,
			pitch
	};

	return image;
}

static inline void free_packed_rgb32_rgb_image(packed_rgb32_image_t image)
{
	if (image.plane) free(image.plane);
}

#endif // RGB_IMAGE_H
