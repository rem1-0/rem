/*****************************************************************************
 * drawing.c: the functions used by rem to allow on-screen drawing
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

#include "drawing.h"
#include "filter_sys.h"

typedef struct line_chain
{
	int x1;
	int y1;
	int x2;
	int y2;

	struct line_chain* tail;
} line_chain;

void init_drawing(void* vp_filter_sys)
{
	((filter_sys_t*) vp_filter_sys)->drawing_private_pointer = NULL;
	((filter_sys_t*) vp_filter_sys)->p_com_medium->draw_line = register_line;
}

void finalize_drawing(void* vp_filter_sys)
{
	line_chain* p_lines = ((filter_sys_t*) vp_filter_sys)->drawing_private_pointer;

	while (p_lines)
	{
		line_chain* p_lines_old = p_lines;
		p_lines = p_lines->tail;
		free(p_lines_old);
	}
}

static void draw_line_planar(planar_rgb_image_t* image, int x1, int y1, int x2, int y2)
{
	int x_min = (x1 < x2) ? x1 : x2;
	int x_max = (x1 < x2) ? x2 : x1;
	int y_min = (y1 < y2) ? y1 : y2;
	int y_max = (y1 < y2) ? y2 : y1;

	int x_dist = x_max - x_min;
	int y_dist = y_max - y_min;

	if (x_dist < y_dist)
	{
		for (int y_current = y_min; y_current <= y_max; ++y_current)
		{
			if (y_current >= (int) image->lines) return;

			if (y_current < 0)
			{
				y_current = -1;
				continue;
			}

			int x_current = x_min + ((2*x_dist*(y_current-y_min))/y_dist + 1)/2;

			if (x_current < 0) continue;

			image->r_plane[x_current + y_current*image->pitch] = 255;
			image->g_plane[x_current + y_current*image->pitch] = 255;
			image->b_plane[x_current + y_current*image->pitch] = 255;
		}
	}
	else
	{
		for (int x_current = x_min; x_current <= x_max; ++x_current)
		{
			if (x_current >= (int) image->pitch) return;

			if (x_current < 0)
			{
				x_current = -1;
				continue;
			}

			int y_current;
			if (x_dist)
			{
				y_current = y_min + ((2*y_dist*(x_current-x_min))/x_dist + 1)/2;
			}
			else
			{
				y_current = y_min;
			}

			if (y_current < 0) continue;

			image->r_plane[x_current + y_current*image->pitch] = 255;
			image->g_plane[x_current + y_current*image->pitch] = 255;
			image->b_plane[x_current + y_current*image->pitch] = 255;
		}
	}
}

static void draw_line_packed(packed_rgb32_image_t* image, int x1, int y1, int x2, int y2)
{
	int x_min = (x1 < x2) ? x1 : x2;
	int x_max = (x1 < x2) ? x2 : x1;
	int y_min = (y1 < y2) ? y1 : y2;
	int y_max = (y1 < y2) ? y2 : y1;

	int x_dist = x_max - x_min;
	int y_dist = y_max - y_min;

	if (x_dist < y_dist)
	{
		for (int y_current = y_min; y_current <= y_max; ++y_current)
		{
			if (y_current >= (int) image->lines)
				return;

			if (y_current < 0)
			{
				y_current = -1;
				continue;
			}

			int x_current = x_min + ((2*x_dist*(y_current-y_min))/y_dist + 1)/2;

			if (x_current < 0) continue;

			image->plane[x_current + y_current*image->padded_pitch].packed = 0xFFFFFF;
		}
	}
	else
	{
		for (int x_current = x_min; x_current <= x_max; ++x_current)
		{
			if (x_current >= (int) image->pitch) return;

			if (x_current < 0)
			{
				x_current = -1;
				continue;
			}

			int y_current;
			if (x_dist)
			{
				y_current = y_min + ((2*y_dist*(x_current-x_min))/x_dist + 1)/2;
			}
			else
			{
				y_current = y_min;
			}

			if (y_current < 0) continue;

			image->plane[x_current + y_current*image->padded_pitch].packed = 0xFFFFFF;
		}
	}
}

void register_line(int x1, int y1, int x2, int y2, void* vp_filter_sys)
{
	line_chain* p_lines = ((filter_sys_t*) vp_filter_sys)->drawing_private_pointer;
	line_chain* p_new_lines = malloc(sizeof(line_chain));

	p_new_lines->tail = p_lines;
	p_new_lines->x1 = x1;
	p_new_lines->y1 = y1;
	p_new_lines->x2 = x2;
	p_new_lines->y2 = y2;

	((filter_sys_t*) vp_filter_sys)->drawing_private_pointer = p_new_lines;
}

void draw_planar(planar_rgb_image_t* image, void* vp_filter_sys)
{
	line_chain* p_lines = ((filter_sys_t*) vp_filter_sys)->drawing_private_pointer;

	while (p_lines)
	{
		draw_line_planar(image, p_lines->x1, p_lines->y1, p_lines->x2, p_lines->y2);

		line_chain* p_lines_old = p_lines;
		p_lines = p_lines->tail;
		free(p_lines_old);
	}

	((filter_sys_t*) vp_filter_sys)->drawing_private_pointer = NULL;
}

void draw_packed(packed_rgb32_image_t* image, void* vp_filter_sys)
{
	line_chain* p_lines = ((filter_sys_t*) vp_filter_sys)->drawing_private_pointer;

	while (p_lines)
	{
		draw_line_packed(image, p_lines->x1, p_lines->y1, p_lines->x2, p_lines->y2);

		line_chain* p_lines_old = p_lines;
		p_lines = p_lines->tail;
		free(p_lines_old);
	}

	((filter_sys_t*) vp_filter_sys)->drawing_private_pointer = NULL;
}
