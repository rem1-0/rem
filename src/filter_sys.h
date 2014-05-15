/*****************************************************************************
 * filter_sys.h: definition of the struct that holds data needed by the module
 *****************************************************************************
 * Copyright (C) 2014 Jean-Christophe Lechenet and Benoit Gastinne
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *****************************************************************************/

#ifndef FILTER_SYS_H
#define FILTER_SYS_H

#include <stdbool.h>

#include <vlc_picture.h>
#include <vlc_image.h>

#include "rgb_image.h"
#include "macros.h"


enum treatment_format_t
{
	none,
	planar_rgb,
	packed_rgb32
};


struct filter_sys_t
{
	image_handler_t* p_handler;

	enum treatment_format_t treatment_format;
	bool b_use_vlc_conversion;

	planar_rgb_image_t planar_rgb_image_in;
	planar_rgb_image_t planar_rgb_image_out;

	packed_rgb32_image_t packed_rgb32_image_in;
	packed_rgb32_image_t packed_rgb32_image_out;

	void (*convert_in) (const picture_t* p_pic_in, planar_rgb_image_t* image_out);
	void (*convert_out) (const planar_rgb_image_t* image_out, picture_t* p_pic_in);

	com_medium_t_MACRO* p_com_medium;
	void* user_private_pointer;

	unsigned int stats_interval;

	unsigned int frame_counter;
	unsigned int last_stats_frame;

	unsigned int treatment_times_sum;
	unsigned int total_times_sum;

	const char* stats_message;
	bool b_stats_to_print;

	void* stats_engine_private_pointer;
	void* drawing_private_pointer;
};

#endif // FILTER_SYS_H
