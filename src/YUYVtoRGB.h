/*****************************************************************************
 * YUYVtoRGB.h: YUYV (packed) to RGB (planar) conversion functions
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

#ifndef YUYVTORGB_H
#define YUYVTORGB_H

#include <vlc_picture.h>

#include "rgb_image.h"

void YUYVtoRGB(const picture_t* p_pic_in, planar_rgb_image_t* image_out);
void RGBtoYUYV(const planar_rgb_image_t* image_in, picture_t* p_pic_out);

#endif // YUYVTORGB_H
