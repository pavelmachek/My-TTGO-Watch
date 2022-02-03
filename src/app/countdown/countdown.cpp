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
#include "config.h"

#include "countdown.h"
#include "countdown_main.h"
#include "countdown_setup.h"
#include "done_in_progress.h"
#include "config/countdown_config.h"

#include "gui/mainbar/app_tile/app_tile.h"
#include "gui/mainbar/main_tile/main_tile.h"
#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"
#include "gui/widget.h"
#include "gui/widget_factory.h"
#include "utils/json_psram_allocator.h"
#include "hardware/powermgm.h"
#include "hardware/rtcctl.h"
#include "hardware/timesync.h"

#ifdef NATIVE_64BIT
    #include "utils/logging.h"
#else
    #ifdef M5PAPER
    #elif defined( LILYGO_WATCH_2020_V1 ) || defined( LILYGO_WATCH_2020_V2 ) || defined( LILYGO_WATCH_2020_V3 )
    #else // NEW_HARDWARE_TAG
    #endif
#endif

#define LABEL_MAX_SIZE 11

static const char countdown_week_day_2[7][3] = {"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa"};
static const char countdown_week_day_3[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

// declare you images or fonts you need
LV_IMG_DECLARE(countdown_64px);
LV_IMG_DECLARE(countdown_48px);

static uint32_t main_tile_num;
static uint32_t setup_tile_num;
static countdown_properties_t properties;

static icon_t *countdown_widget = NULL;

// declare callback functions
static void enter_countdown_event_cb( lv_obj_t * obj, lv_event_t event );


static void create_countdown_app_icon(){
    // create an app icon, label it and get the lv_obj_t icon container
    lv_obj_t * countdown_icon_cont = app_tile_register_app( "alarm");
    // set your own icon and register her callback to activate by an click
    // remember, an app icon must have an size of 64x64 pixel with an alpha channel
    // use https://lvgl.io/tools/imageconverter to convert your images and set "true color with alpha" to get fancy images
    // the resulting c-file can put in /app/examples/images/
    lv_obj_t * countdown_icon = wf_add_image_button( countdown_icon_cont, countdown_64px, enter_countdown_event_cb );
    lv_obj_reset_style_list( countdown_icon, LV_OBJ_PART_MAIN );
    lv_obj_align( countdown_icon , countdown_icon_cont, LV_ALIGN_CENTER, 0, 0 );

    // make app icon drag scroll the mainbar
    mainbar_add_slide_element(countdown_icon);
}

static void main_tile_activate_callback (){
    countdown_main_set_data_to_display(rtcctl_get_alarm_data(), timesync_get_24hr());
}

static void main_tile_hibernate_callback (){
    rtcctl_set_alarm(countdown_main_get_data_to_store());
}

static void setup_tile_activate_callback (){
    countdown_setup_set_data_to_display(&properties);
}

static void update_main_tile_widget_label(){
    if (countdown_widget != NULL){
        widget_set_label(countdown_widget, countdown_get_clock_label(true));
    }
}

static bool countdown_term_changed_cb(EventBits_t event, void *arg ){
    switch ( event ){
        case ( RTCCTL_ALARM_ENABLED):
        case ( RTCCTL_ALARM_DISABLED):
        case ( RTCCTL_ALARM_TERM_SET ):
            update_main_tile_widget_label();
            break;
    }
    return true;
}

static void remove_main_tile_widget(){
    countdown_widget = widget_remove( countdown_widget );
}

static void add_main_tile_widget(){
    countdown_widget = widget_register( countdown_get_clock_label(true), &countdown_48px, enter_countdown_event_cb );
    widget_hide_indicator(countdown_widget);
    update_main_tile_widget_label();
}

static void setup_tile_hibernate_callback (){
    log_d("countdown_setup_is_main_tile_switch_on(): %d, properties.show_on_main_tile: %d", countdown_setup_is_main_tile_switch_on(), properties.show_on_main_tile);
    if (countdown_setup_is_main_tile_switch_on() != properties.show_on_main_tile) {
        if (countdown_setup_is_main_tile_switch_on()){
            add_main_tile_widget();
        }
        else{
            remove_main_tile_widget();
        }
    }
    properties = *countdown_setup_get_data_to_store();
    properties.save();
}

static void create_countdown_main_tile(uint32_t tile_num ){
    countdown_main_setup( main_tile_num );
    mainbar_add_tile_activate_cb(tile_num, main_tile_activate_callback);
    mainbar_add_tile_hibernate_cb(tile_num, main_tile_hibernate_callback);
}

static void create_countdown_setup_tile(uint32_t tile_num){
    countdown_setup_setup( tile_num );
    mainbar_add_tile_activate_cb(tile_num, setup_tile_activate_callback);
    mainbar_add_tile_hibernate_cb(tile_num, setup_tile_hibernate_callback);
}

static void create_done_in_progress_tile(){
    done_in_progress_tile_setup();
}

bool countdown_occurred_event_event_callback ( EventBits_t event, void *arg  ){
    switch ( event ){
        case ( RTCCTL_ALARM_OCCURRED ):
            powermgm_set_event( POWERMGM_WAKEUP_REQUEST );
            done_in_progress_start_alarm();
            rtcctl_set_next_alarm();
            break;
    }
    return( true );
}

static bool powermgmt_callback( EventBits_t event, void *arg  ){
    switch( event ) {
        case( POWERMGM_STANDBY ):
            done_in_progress_finish_alarm();
            break;
    }
    return( true );
}

// setup routine for example app
void countdown_setup( void ) {
    #if defined( ONLY_ESSENTIAL )
        return;
    #endif

    properties.load();

    create_countdown_app_icon();
    if (properties.show_on_main_tile){
        add_main_tile_widget();
    }
    // register 2 vertical tiles and get the first tile number and save it for later use
    main_tile_num = mainbar_add_app_tile( 1, 1, "countdown");
    setup_tile_num = mainbar_add_setup_tile( 1, 1, "countdown setup");

    create_countdown_main_tile(main_tile_num);
    create_countdown_setup_tile(setup_tile_num);
    create_done_in_progress_tile();

    rtcctl_register_cb( RTCCTL_ALARM_OCCURRED , countdown_occurred_event_event_callback, "countdown");
    rtcctl_register_cb( RTCCTL_ALARM_ENABLED | RTCCTL_ALARM_DISABLED| RTCCTL_ALARM_TERM_SET , countdown_term_changed_cb, "countdown");
    powermgm_register_cb( POWERMGM_STANDBY, powermgmt_callback, "countdown");
}

uint32_t countdown_get_app_main_tile_num( void ) {
    return( main_tile_num );
}

uint32_t countdown_get_app_setup_tile_num( void ) {
    return( setup_tile_num );
}

static void enter_countdown_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):
            mainbar_jump_to_tilenumber( main_tile_num, LV_ANIM_OFF );
            statusbar_hide( true );
            break;
    }
}

countdown_properties_t * countdown_get_properties(){
    return &properties;
}

int countdown_get_am_pm_hour(int hour24){
    //FIXME: taken from main_tile.cpp. It would be good to place common function somewhere and use it on both places
    if (hour24 == 0){
        return 12;
    }
    if (hour24 > 12){
        return hour24 - 12;
    }
    return hour24;
}

char const* countdown_get_am_pm_value(int hour24, bool short_format){
    if (hour24 < 12){
        return short_format ? AM_ONE : AM;
    }

    return short_format ? PM_ONE : PM;
}

char const * countdown_get_week_day(int index, bool short_format){
    return short_format ? countdown_week_day_2[index] : countdown_week_day_3[index];
}

char * countdown_get_clock_label(bool show_day)
{
    static char text[LABEL_MAX_SIZE]; //DoW + '\n' + HH:MMA  + '\0'

    snprintf(text, LABEL_MAX_SIZE, "---\n--:--");
    return text;
}
