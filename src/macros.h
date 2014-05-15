/*****************************************************************************
 * macros.h: Macros to use to define properties
 * you must have one and only one configuration block in the source files of
 * your library
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

#ifndef MACROS_H
#define MACROS_H

#include "rgb_image.h"

typedef struct
{
	void (*transform_planar) (const planar_rgb_image_t* in_image,
							  planar_rgb_image_t* out_image,
							  void* void_pointer);
	void (*transform_packed32) (const packed_rgb32_image_t* in_image,
								packed_rgb32_image_t* out_image,
								void* void_pointer);

	void (*init_planar) (void** p_void_pointer);
	void (*init_packed32) (void** p_void_pointer);

	void (*finalize_planar) (void** p_void_pointer);
	void (*finalize_packed32) (void** p_void_pointer);

	void (*register_stat_time) (const char* name, void* private_pointer);
	void (*begin_stat_time) (const char* name, void* private_pointer);
	void (*end_stat_time) (const char* name, void* private_pointer);

	void (*register_stat_int) (const char* name, void* private_pointer);
	void (*set_stat_int) (const char* name, int value, void* private_pointer);

	void (*draw_line) (int x1, int y1,
					   int x2, int y2,
					   void* private_pointer);

	void* private_pointer;
} com_medium_t_MACRO;

extern com_medium_t_MACRO com_medium_MACRO;


static inline void BEGIN_TIME_STAT(const char* stat)
{
	com_medium_MACRO.begin_stat_time(stat, com_medium_MACRO.private_pointer);
}

static inline void END_TIME_STAT(const char* stat)
{
	com_medium_MACRO.end_stat_time(stat, com_medium_MACRO.private_pointer);
}


static inline void SET_INT_STAT(const char* stat, int value)
{
	com_medium_MACRO.set_stat_int(stat, value,
                                  com_medium_MACRO.private_pointer);
}


static inline void DRAW_LINE(int x1, int y1,
							 int x2, int y2)
{
	com_medium_MACRO.draw_line(x1, y1, x2, y2,
                               com_medium_MACRO.private_pointer);
}

static inline void DRAW_RECTANGLE(int x1, int y1,
							      int x2, int y2)
{
	DRAW_LINE(x1, y1, x1, y2);
	DRAW_LINE(x2, y1, x2, y2);
	DRAW_LINE(x1, y1, x2, y1);
	DRAW_LINE(x1, y2, x2, y2);
}

#ifdef __cplusplus
#define REM_EXTERN_MACRO extern "C"
#else
#define REM_EXTERN_MACRO
#endif

#define CONFIG_START \
	com_medium_t_MACRO com_medium_MACRO = {NULL,NULL, NULL,NULL, NULL,NULL,\
										   NULL,NULL,NULL, NULL,NULL, NULL,\
                                           NULL};\
	\
	void TIME_STAT(const char* stat)\
	{com_medium_MACRO.register_stat_time(stat,\
                                         com_medium_MACRO.private_pointer);}\
	void INT_STAT(const char* stat)\
	{com_medium_MACRO.register_stat_int(stat,\
                                        com_medium_MACRO.private_pointer);}\
	\
	void PLANAR_RGB_HANDLE(void (*func) (const planar_rgb_image_t*,\
										 planar_rgb_image_t*,\
										 void* void_pointer))\
	{com_medium_MACRO.transform_planar = func;}\
	void PACKED_RGB32_HANDLE(void (*func) (const packed_rgb32_image_t*,\
										   packed_rgb32_image_t*,\
										   void* void_pointer))\
	{com_medium_MACRO.transform_packed32 = func;}\
	\
	void PLANAR_RGB_INIT(void (*func) (void** p_void_pointer))\
	{com_medium_MACRO.init_planar = func;}\
	void PACKED_RGB32_INIT(void (*func) (void** p_void_pointer))\
	{com_medium_MACRO.init_packed32 = func;}\
	\
	void PLANAR_RGB_FINALIZE(void (*func) (void** p_void_pointer))\
	{com_medium_MACRO.finalize_planar = func;}\
	void PACKED_RGB32_FINALIZE(void (*func) (void** p_void_pointer))\
	{com_medium_MACRO.finalize_packed32 = func;}\
	\
	REM_EXTERN_MACRO void init_MACRO() {

#define CONFIG_END }



#endif // MACROS_H
