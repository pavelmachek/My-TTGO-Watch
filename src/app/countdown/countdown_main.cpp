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

lv_obj_t *countdown_app_main_start_btn = NULL;
lv_obj_t *countdown_app_main_stop_btn = NULL;
lv_obj_t *countdown_app_main_countdownlabel = NULL;

#define ROLLER_ROW_COUNT 4

static bool clock_format_24 = false;
static lv_obj_t *hour_roller = NULL;
static lv_obj_t *minute_roller = NULL;

lv_task_t * _countdown_app_task;

long countdown_milliseconds = 0;
static time_t prev_time;

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

static void countdown_app_main_update_countdownlabel()
{
    //int hr = (countdown_milliseconds / (1000 * 60 * 60)) % 24;

    // minutes
    int min = (countdown_milliseconds / (1000 * 60)) % 60;

    // seconds
    int sec = (countdown_milliseconds / 1000) % 60;

    // milliseconds
    //int mill = countdown_milliseconds % 1000;

    char msg[10];
    sprintf(msg,"%02d:%02d", min, sec);

    lv_label_set_text(countdown_app_main_countdownlabel, msg);
    lv_obj_align(countdown_app_main_countdownlabel, NULL, LV_ALIGN_CENTER, 0, 0);
    //countdown_app_update_widget_label( msg );
}

void countdown_app_task( lv_task_t * task ) {

    time_t now = time(0);
    double dif_seconds = difftime(now,prev_time);
    countdown_milliseconds -= dif_seconds * 1000;
    prev_time = now;

    countdown_app_main_update_countdownlabel();
}

/* FIXME: see countdown.c 

static void remove_main_tile_widget(){
    countdown_widget = widget_remove( countdown_widget );
}

static void add_main_tile_widget(){

*/


void countdown_add_widget(void) {}
void countdown_remove_widget(void) {}

void countdown_start(void)
{
	// create an task that runs every secound
	prev_time = time(0);

	int hour = lv_roller_get_selected(hour_roller);
	int minute = lv_roller_get_selected(minute_roller);

	prev_time += hour*60 + minute;
	
	_countdown_app_task = lv_task_create( countdown_app_task, 1000, LV_TASK_PRIO_MID, NULL );
	lv_obj_set_hidden(countdown_app_main_start_btn, true);
	lv_obj_set_hidden(countdown_app_main_stop_btn, false);
	countdown_add_widget();
	//countdown_app_hide_app_icon_info( false );
}

void countdown_stop(void)
{
	lv_task_del(_countdown_app_task);
	lv_obj_set_hidden(countdown_app_main_start_btn, false);
	lv_obj_set_hidden(countdown_app_main_stop_btn, true);
	countdown_remove_widget();
	//countdown_app_hide_app_icon_info( true );
}

static void start_countdown_app_main_event_cb( lv_obj_t * obj, lv_event_t event ) {
	switch( event ) {
        case( LV_EVENT_CLICKED ):
		countdown_start();
		break;
	}
}

static void stop_countdown_app_main_event_cb( lv_obj_t * obj, lv_event_t event ) {
	switch( event ) {
        case( LV_EVENT_CLICKED ):       // create an task that runs every secound
		countdown_stop();
		break;
	}
}

void countdown_main_setup( uint32_t tile_num ) {
    lv_obj_t * main_tile = mainbar_get_tile_obj( tile_num );

    lv_obj_t *roller_container = wf_add_container(main_tile, LV_LAYOUT_PRETTY_MID, LV_FIT_PARENT, LV_FIT_TIGHT, false, APP_STYLE );
    lv_obj_set_style_local_pad_left( roller_container, LV_CONT_PART_MAIN , LV_STATE_DEFAULT, 1);
    lv_obj_set_style_local_pad_right( roller_container, LV_CONT_PART_MAIN , LV_STATE_DEFAULT, 1);

    hour_roller = wf_add_roller( roller_container, get_roller_content(24, false, !clock_format_24), LV_ROLLER_MODE_INIFINITE, ROLLER_ROW_COUNT );
    lv_obj_set_width( hour_roller, 90 );
    lv_obj_set_height( hour_roller, 90 );

    wf_add_label(roller_container, ":");

    minute_roller = wf_add_roller( roller_container, get_roller_content(60, true, false), LV_ROLLER_MODE_INIFINITE, ROLLER_ROW_COUNT );
    lv_obj_set_width(minute_roller, 90);
    lv_obj_set_height( minute_roller, 90 );

    countdown_app_main_countdownlabel = lv_label_create( roller_container , NULL);
    lv_label_set_text(countdown_app_main_countdownlabel, "00:00");
    lv_obj_reset_style_list( countdown_app_main_countdownlabel, LV_OBJ_PART_MAIN );
    lv_obj_add_style( countdown_app_main_countdownlabel, LV_OBJ_PART_MAIN, APP_STYLE );
    lv_obj_align(countdown_app_main_countdownlabel, NULL, LV_ALIGN_CENTER, 0, 0);
    

    lv_obj_t *exit_btn = wf_add_exit_button( main_tile );
    lv_obj_align(exit_btn, main_tile, LV_ALIGN_IN_BOTTOM_LEFT, THEME_ICON_PADDING, -THEME_ICON_PADDING );

    countdown_app_main_start_btn = lv_btn_create(main_tile, NULL);  
    lv_obj_set_size(countdown_app_main_start_btn, 50, 50);
    lv_obj_add_style(countdown_app_main_start_btn, LV_IMGBTN_PART_MAIN, APP_STYLE );
    lv_obj_align(countdown_app_main_start_btn, main_tile, LV_ALIGN_IN_BOTTOM_MID, 0, 0 );
    lv_obj_set_event_cb( countdown_app_main_start_btn, start_countdown_app_main_event_cb );

    lv_obj_t *countdown_app_main_start_btn_label = lv_label_create(countdown_app_main_start_btn, NULL);
    lv_label_set_text(countdown_app_main_start_btn_label, LV_SYMBOL_PLAY);

    countdown_app_main_stop_btn = lv_btn_create(main_tile, NULL);  
    lv_obj_set_size(countdown_app_main_stop_btn, 50, 50);
    lv_obj_add_style(countdown_app_main_stop_btn, LV_IMGBTN_PART_MAIN, APP_STYLE );
    lv_obj_align(countdown_app_main_stop_btn, main_tile, LV_ALIGN_IN_BOTTOM_MID, 0, 0 );
    lv_obj_set_event_cb( countdown_app_main_stop_btn, stop_countdown_app_main_event_cb );
    lv_obj_set_hidden(countdown_app_main_stop_btn, true);

    lv_obj_t *countdown_app_main_stop_btn_label = lv_label_create(countdown_app_main_stop_btn, NULL);
    lv_label_set_text(countdown_app_main_stop_btn_label, LV_SYMBOL_STOP);
    
    lv_obj_t *setup_btn = wf_add_setup_button( main_tile, enter_countdown_setup_event_cb );
    lv_obj_align(setup_btn, main_tile, LV_ALIGN_IN_BOTTOM_RIGHT, -THEME_ICON_PADDING, -THEME_ICON_PADDING );
}

void countdown_main_set_data_to_display(rtcctl_alarm_t *countdown_data, bool clock_24){
    lv_roller_set_selected(hour_roller, 5, LV_ANIM_OFF);
    lv_roller_set_selected(minute_roller, 0, LV_ANIM_OFF);
}

rtcctl_alarm_t *countdown_main_get_data_to_store(){
    static rtcctl_alarm_t data = {};
    
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

