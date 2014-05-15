/*****************************************************************************
 * drawing.h: the functions used by rem to allow on-screen drawing
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

#ifndef DRAWING_H_
#define DRAWING_H_

#include "rgb_image.h"

void register_line(int x1, int y1, int x2, int y2, void* vp_filter_sys);

void draw_planar(planar_rgb_image_t* image, void* vp_filter_sys);
void draw_packed(packed_rgb32_image_t* image, void* vp_filter_sys);

void init_drawing(void* vp_filter_sys);
void finalize_drawing(void* vp_filter_sys);

#endif // DRAWING_H_
