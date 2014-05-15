/*****************************************************************************
 * stats_engine.cpp: functions used by the statistics system
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

#include "cpp_win32_ugly_fixup.h"
extern "C"
{
	#include "stats_engine.h"
}

#include <map>
#include <sstream>

#include "vlc_common.h"
#include "filter_sys.h"


enum stat_type
{
	time_type,
	integer_type
};

struct stat_t
{
	stat_type type;
	int64_t value;
	int64_t value_sum;

	bool test;
	bool is_valid;
};

typedef std::map<std::string, stat_t> stats_map_t;

struct stats_data_t
{
	stats_map_t stats_map;

	std::stringstream stats_stream;
	std::string stats_message;
};


extern "C" void register_stat_time(const char* name, void* private_pointer)
{
	filter_sys_t* p_sys = reinterpret_cast<filter_sys_t*>(private_pointer);
	stats_data_t& stats_data = *(reinterpret_cast<stats_data_t*>(p_sys->stats_engine_private_pointer));

	std::string name_str(name);

	stat_t& entry = stats_data.stats_map[name_str];

	entry.type = time_type;
	entry.value = 0;
	entry.value_sum = 0;
	entry.test = false;
	entry.is_valid = true;
}

extern "C" void begin_stat_time(const char* name, void* private_pointer)
{
	filter_sys_t* p_sys = reinterpret_cast<filter_sys_t*>(private_pointer);
	stats_data_t& stats_data = *(reinterpret_cast<stats_data_t*>(p_sys->stats_engine_private_pointer));

	std::string name_str(name);
	stat_t& entry = stats_data.stats_map[name_str];

	entry.value = -mdate();
	entry.test = true;
}

extern "C" void end_stat_time(const char* name, void* private_pointer)
{
	filter_sys_t* p_sys = reinterpret_cast<filter_sys_t*>(private_pointer);
	stats_data_t& stats_data = *(reinterpret_cast<stats_data_t*>(p_sys->stats_engine_private_pointer));

	std::string name_str(name);
	stat_t& entry = stats_data.stats_map[name_str];

	entry.value += mdate();

	if (! (entry.test && entry.is_valid))
	{
		entry.is_valid = false;
	}

	stats_data.stats_map[name].test = false;
}


extern "C" void register_stat_int(const char* name, void* private_pointer)
{
	filter_sys_t* p_sys = reinterpret_cast<filter_sys_t*>(private_pointer);
	stats_data_t& stats_data = *(reinterpret_cast<stats_data_t*>(p_sys->stats_engine_private_pointer));

	std::string name_str(name);
	stat_t& entry = stats_data.stats_map[name_str];

	entry.type = integer_type;
	entry.value = 0;
	entry.value_sum = 0;
	entry.test = false;
	entry.is_valid = true;
}

extern "C" void set_stat_int(const char* name, int value, void* private_pointer)
{
	filter_sys_t* p_sys = reinterpret_cast<filter_sys_t*>(private_pointer);
	stats_data_t& stats_data = *(reinterpret_cast<stats_data_t*>(p_sys->stats_engine_private_pointer));

	std::string name_str(name);
	stat_t& entry = stats_data.stats_map[name_str];

	entry.value = value;
	entry.test = true;

}


extern "C" void init_stats(void* private_pointer)
{
	filter_sys_t* p_sys = reinterpret_cast<filter_sys_t*>(private_pointer);

	p_sys->p_com_medium->register_stat_time = register_stat_time;
	p_sys->p_com_medium->begin_stat_time = begin_stat_time;
	p_sys->p_com_medium->end_stat_time = end_stat_time;

	p_sys->p_com_medium->register_stat_int = register_stat_int;
	p_sys->p_com_medium->set_stat_int = set_stat_int;

	p_sys->b_stats_to_print = false;
	p_sys->stats_engine_private_pointer = reinterpret_cast<void*>(new stats_data_t());
}

extern "C" void treat_stats(void* private_pointer)
{
	filter_sys_t* p_sys = reinterpret_cast<filter_sys_t*>(private_pointer);
	stats_data_t& stats_data = *(reinterpret_cast<stats_data_t*>(p_sys->stats_engine_private_pointer));

	for (stats_map_t::iterator it = stats_data.stats_map.begin(); it != stats_data.stats_map.end(); it++)
	{
		if (it->second.is_valid)
		{
			if (it->second.type == time_type && it->second.test)
			{
				it->second.is_valid = false;
			}
			else if (it->second.type == integer_type && ! it->second.test)
			{
				it->second.is_valid = false;
			}
			else
			{
				it->second.value_sum += it->second.value;
			}
		}

		it->second.value = 0;
	}

	p_sys->frame_counter++;

	unsigned int frame_diff = p_sys->frame_counter - p_sys->last_stats_frame;

	if (frame_diff < p_sys->stats_interval)
		return;

	p_sys->last_stats_frame++;

	unsigned int mean_treatment = p_sys->treatment_times_sum / frame_diff;
	unsigned int mean_total = p_sys->total_times_sum / frame_diff;

	stats_data.stats_stream.str("");

	stats_data.stats_stream << "Frames " << p_sys->last_stats_frame
						   << " to " << p_sys->frame_counter << ":" << std::endl
						   << " average treatment time: " << mean_treatment << " microseconds" << std::endl
						   << " average total filtering time: " << mean_total << " microseconds" << std::endl;

	for (stats_map_t::iterator it = stats_data.stats_map.begin(); it != stats_data.stats_map.end(); it++)
	{
		stats_data.stats_stream << " average " << it->first << ": ";

		if (! it->second.is_valid)
		{
			stats_data.stats_stream << "invalid value or acquisition";
		}
		else if (it->second.type == time_type)
		{
			stats_data.stats_stream << it->second.value_sum/frame_diff << " microseconds";
		}
		else if (it->second.type == integer_type)
		{
			stats_data.stats_stream << it->second.value_sum/frame_diff;
		}

		stats_data.stats_stream << std::endl;

		it->second.value_sum = 0;
	}

	p_sys->treatment_times_sum = 0;
	p_sys->total_times_sum = 0;
	p_sys->last_stats_frame = p_sys->frame_counter;

	stats_data.stats_message = stats_data.stats_stream.str();
	p_sys->stats_message = stats_data.stats_message.c_str();

	p_sys->b_stats_to_print = true;
}

extern "C" void finalize_stats(void* private_pointer)
{
	filter_sys_t* p_sys = reinterpret_cast<filter_sys_t*>(private_pointer);
	stats_data_t* stats_data = reinterpret_cast<stats_data_t*>(p_sys->stats_engine_private_pointer);

	if (stats_data) delete stats_data;
}
