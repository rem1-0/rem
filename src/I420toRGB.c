/*****************************************************************************
 * I420toRGB.c: YUV I420 (planar) to RGB (planar) conversion functions
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

#include "I420toRGB.h"

#include "filter_picture.h"

void I420toRGB(const picture_t* p_pic_in, planar_rgb_image_t* image_out)
{
	for( int y = 0; y < p_pic_in->p[Y_PLANE].i_visible_lines; y++ )
	{
		int y_u = y/2;

		uint8_t *p_src_y = &p_pic_in->p[Y_PLANE].p_pixels[y * p_pic_in->p[Y_PLANE].i_pitch];
		uint8_t *p_src_u = &p_pic_in->p[U_PLANE].p_pixels[y_u * p_pic_in->p[U_PLANE].i_pitch];
		uint8_t *p_src_v = &p_pic_in->p[V_PLANE].p_pixels[y_u * p_pic_in->p[V_PLANE].i_pitch];

		for( int x = 0; x < p_pic_in->p[Y_PLANE].i_visible_pitch; x++ )
		{
			int r, g, b;
			yuv_to_rgb( &r, &g, &b, *p_src_y, *p_src_u, *p_src_v );

			image_out->r_plane[x + y*image_out->pitch] = r;
			image_out->g_plane[x + y*image_out->pitch] = g;
			image_out->b_plane[x + y*image_out->pitch] = b;

			if (x%2 == 1)
			{
				p_src_u++;
				p_src_v++;
			}
			p_src_y++;
		}
	}
}

void RGBtoI420(const planar_rgb_image_t* image_in, picture_t* p_pic_out)
{
	for( int y = 0; y < p_pic_out->p[Y_PLANE].i_visible_lines; y++ )
	{
		int y_u = y/2;

		uint8_t *p_dst_y = &p_pic_out->p[Y_PLANE].p_pixels[y * p_pic_out->p[Y_PLANE].i_pitch];
		uint8_t *p_dst_u = &p_pic_out->p[U_PLANE].p_pixels[y_u * p_pic_out->p[U_PLANE].i_pitch];
		uint8_t *p_dst_v = &p_pic_out->p[V_PLANE].p_pixels[y_u * p_pic_out->p[V_PLANE].i_pitch];

		unsigned int moy_u, moy_v;

		for( int x = 0; x < p_pic_out->p[Y_PLANE].i_visible_pitch; x++ )
		{
			int r,g,b;
			r = image_in->r_plane[x + y*image_in->pitch];
			g = image_in->g_plane[x + y*image_in->pitch];
			b = image_in->b_plane[x + y*image_in->pitch];

			uint8_t tmp_u, tmp_v;

			rgb_to_yuv( p_dst_y, &tmp_u, &tmp_v, r, g, b);

			moy_u += tmp_u;
			moy_v += tmp_v;

			if (x%2 == 1 )
			{
				if (y%2 == 0)
				{
					*p_dst_u = moy_u/4;
					*p_dst_v = moy_v/4;
					moy_u = 0;
					moy_v = 0;
				}
				else
				{
					*p_dst_u = *p_dst_u + moy_u/4;
					*p_dst_v = *p_dst_v + moy_v/4;
					moy_u = 0;
					moy_v = 0;
				}
				p_dst_u++;
				p_dst_v++;
			}
			p_dst_y++;
		}
	}
}
