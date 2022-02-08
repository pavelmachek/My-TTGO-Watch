/****************************************************************************
 *   Copyright  2020  Jakub Vesely
 *   Copyright  2022  Pavel Machek
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

#include "config.h"
#include "countdown.h"
#include "countdown_main.h"
#include "config/countdown_config.h"

#include "gui/mainbar/app_tile/app_tile.h"
#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"
#include "gui/widget_factory.h"
#include "gui/widget_styles.h"
#include "hardware/rtcctl.h"

lv_obj_t *countdown_enabled_switch = NULL;

#define ROLLER_ROW_COUNT 4

static bool clock_format_24 = false;
static lv_obj_t *hour_roller = NULL;
static lv_obj_t *minute_roller = NULL;

static void enter_countdown_setup_event_cb( lv_obj_t * obj, lv_event_t event );

static char* get_roller_content(int count, bool zeros, bool am_pm_roller){
    static char content[60 * 3]; //max(60 sec * 2 digits, 24 * (2 + 1 + AM||PM) ) + \n on each line (on last line is \0 instead)
    int pos = 0;
    for (int index = 0; index < count; ++index){
        int number = index;

        if (number < 10){
            if (zeros){
                content[pos++] = '0';
            }
        }
        else{
            content[pos++] = '0' + number / 10;
        }

        content[pos++] = '0' + number % 10;
        content[pos++] = (index == count - 1 ? '\0' : '\n');
    }
    return content;
}

void countdown_main_setup( uint32_t tile_num ) {
    lv_obj_t * main_tile = mainbar_get_tile_obj( tile_num );
    
    lv_obj_t * countdown_onoff_cont = wf_add_labeled_switch( main_tile, "Activated", &countdown_enabled_switch, true, NULL, APP_STYLE );
    lv_obj_align( countdown_onoff_cont, main_tile, LV_ALIGN_IN_TOP_MID, THEME_ICON_PADDING, THEME_ICON_PADDING );

    lv_obj_t *roller_container = wf_add_container(main_tile, LV_LAYOUT_PRETTY_MID, LV_FIT_PARENT, LV_FIT_TIGHT, false, APP_STYLE );
    lv_obj_set_style_local_pad_left( roller_container, LV_CONT_PART_MAIN , LV_STATE_DEFAULT, 1);
    lv_obj_set_style_local_pad_right( roller_container, LV_CONT_PART_MAIN , LV_STATE_DEFAULT, 1);
    lv_obj_align( roller_container, countdown_onoff_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, THEME_PADDING );

    hour_roller = wf_add_roller( roller_container, get_roller_content(24, false, !clock_format_24), LV_ROLLER_MODE_INIFINITE, ROLLER_ROW_COUNT );
    lv_obj_set_width( hour_roller, 90 );
    lv_obj_set_height( hour_roller, 90 );

    wf_add_label(roller_container, ":");

    minute_roller = wf_add_roller( roller_container, get_roller_content(60, true, false), LV_ROLLER_MODE_INIFINITE, ROLLER_ROW_COUNT );
    lv_obj_set_width(minute_roller, 90);
    lv_obj_set_height( minute_roller, 90 );

    lv_obj_t *exit_btn = wf_add_exit_button( main_tile );
    lv_obj_align(exit_btn, main_tile, LV_ALIGN_IN_BOTTOM_LEFT, THEME_ICON_PADDING, -THEME_ICON_PADDING );

    lv_obj_t *setup_btn = wf_add_setup_button( main_tile, enter_countdown_setup_event_cb );
    lv_obj_align(setup_btn, main_tile, LV_ALIGN_IN_BOTTOM_RIGHT, -THEME_ICON_PADDING, -THEME_ICON_PADDING );
}

void countdown_main_set_data_to_display(rtcctl_alarm_t *countdown_data, bool clock_24){
    if (clock_format_24 != clock_24){
        clock_format_24 = clock_24;
        lv_roller_set_options(hour_roller, get_roller_content(24, false, !clock_24) , LV_ROLLER_MODE_INIFINITE);
    }
    lv_roller_set_selected(hour_roller, countdown_data->hour, LV_ANIM_OFF);
    lv_roller_set_selected(minute_roller, countdown_data->minute, LV_ANIM_OFF);

    if (countdown_data->enabled){
        lv_switch_on(countdown_enabled_switch, LV_ANIM_OFF);
    }
    else{
        lv_switch_off(countdown_enabled_switch, LV_ANIM_OFF);
    }
}

rtcctl_alarm_t *countdown_main_get_data_to_store(){
    static rtcctl_alarm_t data = {};
    data.enabled = lv_switch_get_state(countdown_enabled_switch);
    data.hour = lv_roller_get_selected(hour_roller);
    data.minute = lv_roller_get_selected(minute_roller);
    
    return &data;
}

static void enter_countdown_setup_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):
            statusbar_hide( true );
            mainbar_jump_to_tilenumber( countdown_get_app_setup_tile_num(), LV_ANIM_ON );
            break;
    }
}
