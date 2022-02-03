/****************************************************************************
 *   Copyright  2020  Jakub Vesely
 *   Email: jakub_vesely@seznam.cz
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#pragma once

#include "config/countdown_config.h"

void countdown_setup( void );
uint32_t countdown_get_app_setup_tile_num( void );
uint32_t countdown_get_app_main_tile_num( void );

countdown_properties_t * countdown_get_properties();
int countdown_get_am_pm_hour(int hour24);
char const * countdown_get_am_pm_value(int hour24, bool short_format);
char const * countdown_get_week_day(int index, bool short_format);
char * countdown_get_clock_label(bool show_day);

