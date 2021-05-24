/****************************************************************************
 *   Copyright  2021  Dirk Brosswick
 *   Email: dirk.brosswick@googlemail.com
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
#include "config.h"
#include <TTGO.h>

#include "alarm_app.h"

#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"
#include "gui/app.h"
#include "gui/widget.h"

#include "hardware/rtcctl.h"

uint32_t alarm_app_main_tile_num;
uint32_t aparm_app_setup_tile_num;

rtcctl_alarm_t *rtcctl_alarm = NULL;
// app icon
icon_t *alarm_app = NULL;
icon_t *alarm_widget = NULL;

// declare you images or fonts you need
LV_IMG_DECLARE(alarm_clock_64px);

// declare callback functions
static void enter_alarm_app_event_cb( lv_obj_t * obj, lv_event_t event );

// setup routine for example app
void alarm_app_setup( void ) {
    alarm_app_main_tile_num = mainbar_add_app_tile( 1, 1, "alarm main" );
    aparm_app_setup_tile_num = mainbar_add_app_tile( 1, 1, "alarm setup" );

    alarm_app = app_register( "alarm", &alarm_clock_64px, enter_alarm_app_event_cb );

    rtcctl_alarm = rtcctl_add_alarm_entry();
    rtcctl_alarm = rtcctl_add_alarm_entry();
    rtcctl_alarm = rtcctl_add_alarm_entry();
    rtcctl_alarm = rtcctl_add_alarm_entry();
}

uint32_t alarm_app_get_app_main_tile_num( void ) {
    return( alarm_app_main_tile_num );
}

static void enter_alarm_app_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_tilenumber( alarm_app_main_tile_num, LV_ANIM_OFF );
                                        statusbar_hide( true );
                                        break;
    }    
}
