/*****************************************************************************
 * module.c: the main file of the rem project that provides the functions and
 * configuration needed by VLC.
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <ltdl.h>
#include <vlc_common.h>
#include <vlc_plugin.h>

#include <vlc_filter.h>

#include "filter_picture.h"

#include "macros.h"
#include "filter_sys.h"
#include "stats_engine.h"
#include "drawing.h"

#include "rgb_image.h"
#include "I420toRGB.h"
#include "YUYVtoRGB.h"


/*****************************************************************************
 * Local prototypes
 *****************************************************************************/
static int Open(vlc_object_t*);                     // also called Create
static void Close(vlc_object_t*);                   // also called Distroy

static picture_t* Filter(filter_t*, picture_t*);    // function that applies
                                                    // the filter


/*****************************************************************************
 * Module descriptor
 *****************************************************************************/
vlc_module_begin()
    set_shortname("Rem")
    set_description("Rem generic video filter")
    set_category(CAT_VIDEO)                 // location of the preferences
                                            // in the UI
    set_subcategory(SUBCAT_VIDEO_VFILTER)   // idem
    set_capability("video filter2", 0)      // type of module and its score
                                            // score 0 means that the module
                                            // is not called unless explicitly
    add_shortcut("rem")
    set_callbacks(Open, Close)              // the names must be the same
                                            // as defined above
    add_loadfile("treatment", NULL, "Treatment to be used",
                 "Library to be loaded and used fot the treatment",
                 false)
    add_bool("accept_packed32", 1, "Packed RGB 32bits",
             "Accept the usage of packed RGB 32 bits for the treatment",
             false)
    add_bool("accept_planar", 1, "Planar RGB",
             "Accept the usage of planar RGB for the treatment",
             false)
    add_integer_with_range("stats_interval", 25, 1, UINT32_MAX,
                           "Interval",
                           "Number of frames over which the statistics"
                               "are averaged",
                           false)
vlc_module_end()

/*****************************************************************************
 * Module callbacks
 *****************************************************************************/
static int Open( vlc_object_t *p_this )
{
    filter_t* p_filter = (filter_t*)p_this;

    vlc_fourcc_t fourcc = p_filter->fmt_in.video.i_chroma;

    const vlc_chroma_description_t *p_chroma =
        vlc_fourcc_GetChromaDescription(fourcc);

    if (p_chroma == NULL || p_chroma->plane_count == 0)
    {
        return VLC_EGENERIC;
    }

    p_filter->p_sys = malloc(sizeof(filter_sys_t));
    if (! p_filter->p_sys)
	{
    	msg_Err(p_filter, "Problem initializing filter");
        return VLC_EGENERIC;
	}

    p_filter->p_sys->treatment_format = none;

	p_filter->p_sys->stats_interval = var_InheritInteger(p_filter,
                                                         "stats_interval");

    p_filter->p_sys->frame_counter = 0;
    p_filter->p_sys->last_stats_frame = 0;

    p_filter->p_sys->treatment_times_sum = 0;
    p_filter->p_sys->total_times_sum = 0;

    lt_dlinit();

	char* module_path = var_InheritString(p_filter, "treatment");
	if (! module_path)
	{
		msg_Info(p_filter, "No treatment to load");
		lt_dlexit();
		free(p_filter->p_sys);
		return VLC_EGENERIC;
	}

	lt_dlhandle module = lt_dlopenext(module_path);
	if (! module)
	{
		msg_Err(p_filter, "Unable to open treatment library at %s",
                module_path);
		lt_dlexit();
		free(p_filter->p_sys);
		return VLC_EGENERIC;
	}

	p_filter->p_sys->p_com_medium = lt_dlsym(module, "com_medium_MACRO");
	if (! p_filter->p_sys->p_com_medium)
	{
		msg_Err(p_filter, "Invalid treatment library: communication medium"
                          " not found (was macros.h included ?)");
		lt_dlexit();
		free(p_filter->p_sys);
		return VLC_EGENERIC;
	}

	void (*init) (void);
	init = lt_dlsym(module, "init_MACRO");
	if (! init)
	{
		msg_Err(p_filter, "Invalid treatment library: init function not found"
                          " (was there a configuration ?)");
	    lt_dlexit();
		free(p_filter->p_sys);
		return VLC_EGENERIC;
	}

	p_filter->p_sys->p_com_medium->private_pointer = p_filter->p_sys;

	init_stats(p_filter->p_sys);
	init_drawing(p_filter->p_sys);

	(*init)();

	bool b_treatment_found = false;

	if (var_InheritBool(p_filter, "accept_packed32") &&
		p_filter->p_sys->p_com_medium->transform_packed32)
	{
		if (p_filter->p_sys->p_com_medium->init_packed32)
			p_filter->p_sys->p_com_medium->init_packed32(
                &p_filter->p_sys->user_private_pointer);

		b_treatment_found = true;
		p_filter->p_sys->treatment_format = packed_rgb32;

		p_filter->p_sys->packed_rgb32_image_out.padded_lines =
            p_filter->fmt_out.video.i_height;
		p_filter->p_sys->packed_rgb32_image_out.padded_pitch =
            p_filter->fmt_out.video.i_width;
		p_filter->p_sys->packed_rgb32_image_out.lines =
            p_filter->fmt_out.video.i_visible_height;
		p_filter->p_sys->packed_rgb32_image_out.pitch =
            p_filter->fmt_out.video.i_visible_width;

		p_filter->p_sys->packed_rgb32_image_in.padded_lines =
            p_filter->fmt_out.video.i_height;
		p_filter->p_sys->packed_rgb32_image_in.padded_pitch =
            p_filter->fmt_out.video.i_width;
		p_filter->p_sys->packed_rgb32_image_in.lines =
            p_filter->fmt_out.video.i_visible_height;
		p_filter->p_sys->packed_rgb32_image_in.pitch =
            p_filter->fmt_out.video.i_visible_width;

		if (fourcc == VLC_CODEC_RGB32)
		{
			p_filter->p_sys->b_use_vlc_conversion = false;
		}
		else
		{
			p_filter->p_sys->b_use_vlc_conversion = true;
			p_filter->fmt_out.video.i_chroma = VLC_CODEC_RGB32;
		}
	}
	if (!b_treatment_found &&
		var_InheritBool(p_filter, "accept_planar") &&
		p_filter->p_sys->p_com_medium->transform_planar)
	{
		if (p_filter->p_sys->p_com_medium->init_planar)
			p_filter->p_sys->p_com_medium->init_planar(
                &p_filter->p_sys->user_private_pointer);

		b_treatment_found = true;
		p_filter->p_sys->treatment_format = planar_rgb;

		p_filter->p_sys->planar_rgb_image_in =
            new_planar_rgb_image(p_filter->fmt_in.video.i_visible_height,
                                 p_filter->fmt_in.video.i_visible_width);
		p_filter->p_sys->planar_rgb_image_out =
            new_planar_rgb_image(p_filter->fmt_out.video.i_visible_height,
                                 p_filter->fmt_out.video.i_visible_width);

		if (fourcc == VLC_CODEC_I420)
		{
			p_filter->p_sys->b_use_vlc_conversion = false;
			p_filter->p_sys->convert_in = I420toRGB;
			p_filter->p_sys->convert_out = RGBtoI420;
		}
		else if (fourcc == VLC_CODEC_YUYV)
		{
			p_filter->p_sys->b_use_vlc_conversion = false;
			p_filter->p_sys->convert_in = YUYVtoRGB;
			p_filter->p_sys->convert_out = RGBtoYUYV;
		}
		else
		{
			p_filter->p_sys->b_use_vlc_conversion = true;
			p_filter->fmt_out.video.i_chroma = VLC_CODEC_I420;
			p_filter->p_sys->convert_in = I420toRGB;
			p_filter->p_sys->convert_out = RGBtoI420;
		}
	}
	if (!b_treatment_found)
	{
		msg_Err(p_filter,
                "Invalid treatment library: doesn't handle any active format");
	    finalize_stats(p_filter->p_sys);
	    finalize_drawing(p_filter->p_sys);
	    lt_dlexit();
		free(p_filter->p_sys);
		return VLC_EGENERIC;
	}

	p_filter->p_sys->p_handler = image_HandlerCreate(p_filter);
    p_filter->pf_video_filter = Filter;
    return VLC_SUCCESS;
}

static void Close(vlc_object_t *p_this)
{
    filter_t* p_filter = (filter_t*)p_this;

    image_HandlerDelete(p_filter->p_sys->p_handler);

    switch (p_filter->p_sys->treatment_format)
    {
    case planar_rgb:
    	free_planar_rgb_image(p_filter->p_sys->planar_rgb_image_in);
    	free_planar_rgb_image(p_filter->p_sys->planar_rgb_image_out);
    	if (p_filter->p_sys->p_com_medium->finalize_planar)
    		p_filter->p_sys->p_com_medium->finalize_planar(
                &p_filter->p_sys->user_private_pointer);
    	break;
    case packed_rgb32:
    	if (p_filter->p_sys->p_com_medium->finalize_packed32)
    	    p_filter->p_sys->p_com_medium->finalize_packed32(
                &p_filter->p_sys->user_private_pointer);
    	break;
    }

    lt_dlexit();

    free(p_filter->p_sys);

    //J'ai lu ça dans yuvp.c
    VLC_UNUSED(p_this);
}


/*****************************************************************************
 * Main Callback
 *****************************************************************************/
static picture_t* Filter(filter_t* p_filter, picture_t* p_pic)
{
	mtime_t time_begin = mdate();

	if(!p_pic)			//traitement d'une erreur possible
		return NULL;

	if (! p_filter->p_sys->p_com_medium)
	{
		return p_pic;
	}

	picture_t* p_inpic;
	picture_t* p_outpic = filter_NewPicture(p_filter);

	if(!p_outpic)
	{
		msg_Err(p_filter, "Problem initializing output picture");
		picture_Release(p_pic);
		return NULL;
	}

	if (p_filter->p_sys->b_use_vlc_conversion)
	{
		p_inpic = image_Convert(p_filter->p_sys->p_handler, p_pic,
                                &(p_filter->fmt_in.video),
                                &(p_filter->fmt_out.video));

		if(!p_inpic)
		{
			msg_Err(p_filter, "Problem initializing converted input picture");
			picture_Release(p_pic);
			return NULL;
		}
	}
	else
	{
		p_inpic = p_pic;
	}

	mtime_t time_begin_treatment;
	mtime_t time_end_treatment;

	if (p_filter->p_sys->treatment_format == planar_rgb)
	{
		(*p_filter->p_sys->convert_in)(p_inpic,
                                       &(p_filter->p_sys->planar_rgb_image_in));
		time_begin_treatment = mdate();
		(*p_filter->p_sys->p_com_medium->transform_planar)(
            &(p_filter->p_sys->planar_rgb_image_in),
            &(p_filter->p_sys->planar_rgb_image_out),
            p_filter->p_sys->user_private_pointer);
		time_end_treatment = mdate();

		draw_planar(&(p_filter->p_sys->planar_rgb_image_out), p_filter->p_sys);

		(*p_filter->p_sys->convert_out)(
            &(p_filter->p_sys->planar_rgb_image_out),
            p_outpic);
	}
	else if (p_filter->p_sys->treatment_format == packed_rgb32)
	{
		p_filter->p_sys->packed_rgb32_image_in.plane =
            (rgb32_pixel_t*)p_inpic->p[0].p_pixels;
		p_filter->p_sys->packed_rgb32_image_out.plane =
            (rgb32_pixel_t*)p_outpic->p[0].p_pixels;

		time_begin_treatment = mdate();
		(*p_filter->p_sys->p_com_medium->transform_packed32)(
            &(p_filter->p_sys->packed_rgb32_image_in),
            &(p_filter->p_sys->packed_rgb32_image_out),
            p_filter->p_sys->user_private_pointer);
		time_end_treatment = mdate();

		draw_packed(&(p_filter->p_sys->packed_rgb32_image_out),
                    p_filter->p_sys);
	}

	if (p_filter->p_sys->b_use_vlc_conversion)
	{
		picture_Release(p_inpic);
	}

	mtime_t time_end = mdate();

	p_filter->p_sys->treatment_times_sum += (time_end_treatment -
                                            time_begin_treatment);
	p_filter->p_sys->total_times_sum += (time_end - time_begin);

	treat_stats(p_filter->p_sys);

	if (p_filter->p_sys->b_stats_to_print)
	{
		msg_Info(p_filter, p_filter->p_sys->stats_message);
		p_filter->p_sys->b_stats_to_print = 0;
	}

	return CopyInfoAndRelease( p_outpic, p_pic );
}
