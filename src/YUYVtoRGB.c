/*****************************************************************************
 * YUYVtoRGB.c: YUYV (packed) to RGB (planar) conversion functions
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

#include "YUYVtoRGB.h"

#include "filter_picture.h"

void YUYVtoRGB(const picture_t* p_pic_in, planar_rgb_image_t* image_out)
{
	for(int y = 0; y < p_pic_in->p[0].i_visible_lines; y++)
	{
		const uint8_t* position = &(p_pic_in->p[0].p_pixels[y * p_pic_in->p[0].i_pitch]);
		const uint8_t* position_end = position + p_pic_in->p[0].i_visible_pitch;

		uint8_t* r_position = &(image_out->r_plane[y * image_out->pitch]);
		uint8_t* g_position = &(image_out->g_plane[y * image_out->pitch]);
		uint8_t* b_position = &(image_out->b_plane[y * image_out->pitch]);

		while (position < position_end)
		{
			int r1,g1,b1, r2,g2,b2;

			uint8_t y1 = *(position);
			uint8_t u = *(++position);
			uint8_t y2 = *(++position);
			uint8_t v = *(++position);

			yuv_to_rgb(&r1, &g1, &b1, y1, u, v);
			yuv_to_rgb(&r2, &g2, &b2, y2, u, v);

			*r_position = r1;
			*(++r_position) = r2;

			*g_position = g1;
			*(++g_position) = g2;

			*b_position = b1;
			*(++b_position) = b2;

			++position;
			++r_position;
			++g_position;
			++b_position;
		}
	}
}

void RGBtoYUYV(const planar_rgb_image_t* image_in, picture_t* p_pic_out)
{
	for(int y = 0; y < p_pic_out->p[0].i_visible_lines; y++)
	{
		uint8_t* position = &(p_pic_out->p[0].p_pixels[y * p_pic_out->p[0].i_pitch]);
		uint8_t* position_end = position + p_pic_out->p[0].i_visible_pitch;

		const uint8_t* r_position = &(image_in->r_plane[y * image_in->pitch]);
		const uint8_t* g_position = &(image_in->g_plane[y * image_in->pitch]);
		const uint8_t* b_position = &(image_in->b_plane[y * image_in->pitch]);

		while (position < position_end)
		{
			uint8_t y1,u1,v1, y2,v2,u2;

			const int r1 = *r_position;
			const int r2 = *(++r_position);

			const int g1 = *g_position;
			const int g2 = *(++g_position);

			const int b1 = *b_position;
			const int b2 = *(++b_position);

			rgb_to_yuv(&y1, &u1, &v1, r1, g1, b1);
			rgb_to_yuv(&y2, &u2, &v2, r2, g2, b2);

			int sum_u = u1 + u2;
			int sum_v = v1 + v2;

			*(position) = y1;
			*(++position) = sum_u / 2;
			*(++position) = y2;
			*(++position) = sum_v / 2;

			++position;
			++r_position;
			++g_position;
			++b_position;
		}
	}
}
