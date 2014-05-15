/*****************************************************************************
 * stats_engine.h: functions used by the statistics system
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

#ifndef STATS_ENGINE_H
#define STATS_ENGINE_H

void register_stat_time(const char* name, void* private_pointer);
void begin_stat_time(const char* name, void* private_pointer);
void end_stat_time(const char* name, void* private_pointer);

void register_stat_int(const char* name, void* private_pointer);
void set_stat_int(const char* name, int value, void* private_pointer);


void init_stats(void* private_pointer);
void treat_stats(void* private_pointer);
void finalize_stats(void* private_pointer);

#endif // STATS_ENGINE_H
