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
#include "done_in_progress.h"
#include "config/countdown_config.h"

#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"
#include "gui/sound/piep.h"
#include "gui/widget_factory.h"
#include "gui/widget_styles.h"
#include "hardware/display.h"
#include "hardware/motor.h"
#include "hardware/rtcctl.h"
#include "hardware/sound.h"
#include "hardware/timesync.h"

#define BEEP_TO_VIBE_DELAY 2
#define BEEP_OFTEN_DELAY 5

LV_FONT_DECLARE(Ubuntu_72px);
LV_IMG_DECLARE(cancel_32px);
LV_IMG_DECLARE(countdown_64px);

static const int highlight_time = 1000; //ms
static const int vibe_time = 500; //ms

static lv_obj_t *tile=NULL;
static uint32_t tile_num = 0;
static bool in_progress = false;
static bool highlighted = false;
static lv_obj_t *label = NULL;
static lv_style_t label_style;
static int brightness = 0;
static int vibe_delay_coutdown = 0;
static int beep_often_countown = 0;

bool done_in_progress_style_change_event_cb( EventBits_t event, void *arg );

bool done_in_progress_style_change_event_cb( EventBits_t event, void *arg ) {
    switch( event ) {
        case STYLE_CHANGE:  lv_style_copy( &label_style, APP_ICON_LABEL_STYLE );
                            lv_style_set_text_font( &label_style, LV_STATE_DEFAULT, &Ubuntu_72px);
                            break;
    }
    return( true );
}

static void exit_event_callback( lv_obj_t * obj, lv_event_t event ){
    switch( event ) {
        case( LV_EVENT_CLICKED ):
            in_progress = false;
            break;
    }
}

static bool is_countdown_time(){
    time_t now;
    struct tm time_tm;
    return true;
    time( &now );
    localtime_r( &now, &time_tm );
    return time_tm.tm_hour == rtcctl_get_alarm_data()->hour && time_tm.tm_min == rtcctl_get_alarm_data()->minute;
}

static void countdown_task_function(lv_task_t * task){
    countdown_properties_t * properties = countdown_get_properties();
    if (in_progress && !is_countdown_time()){
        in_progress = false;
    }

    if (!in_progress){ //last turn
        lv_task_del(task);
        highlighted = false; //set default value
    }

    if (properties->beep && in_progress && vibe_delay_coutdown == 0){ //beeping starts after defined number of vibrations
        if (beep_often_countown == 0 || highlighted){ //increase number of beeps after a defined while 
            if (beep_often_countown > 0){
                beep_often_countown--;
            }
            sound_play_progmem_wav(piep_wav, piep_wav_len);
        }
    }
    
    if (highlighted && properties->vibe){
        motor_vibe(vibe_time / 10, true);
        
        if (vibe_delay_coutdown > 0){
            vibe_delay_coutdown--;
        }
    }

    

    if (properties->fade){
        //used brightmess because is smooth for SW dimming would be necessary to use double buffer display
        display_set_brightness(highlighted ? DISPLAY_MAX_BRIGHTNESS : DISPLAY_MIN_BRIGHTNESS);
    }
    //lv_style_set_text_color( &label_style, LV_OBJ_PART_MAIN, highlighted ? LV_COLOR_BLACK : LV_COLOR_WHITE );
    //lv_style_set_text_opa(&label_style, LV_OBJ_PART_MAIN, highlighted ? LV_OPA_100 : LV_OPA_0);

    lv_obj_invalidate(tile);

    if (in_progress){
        highlighted = !highlighted;
        lv_disp_trig_activity( NULL ); //to stay display on
    }
    else{
        display_set_brightness(brightness);
        mainbar_jump_back();
    }
}

void done_in_progress_start_alarm(){
    mainbar_jump_to_tilenumber( tile_num, LV_ANIM_OFF );
    statusbar_hide( true );

    lv_label_set_text(label, countdown_get_clock_label(false));
    lv_obj_align( label, tile, LV_ALIGN_CENTER, 0, 0 );

    highlighted = true;
    in_progress = true;
    vibe_delay_coutdown = countdown_get_properties()->vibe ? BEEP_TO_VIBE_DELAY : 0;
    beep_often_countown = BEEP_OFTEN_DELAY;
    brightness = display_get_brightness();
    lv_task_create( countdown_task_function, highlight_time, LV_TASK_PRIO_MID, NULL );
}

void done_in_progress_finish_alarm(){
    in_progress = false;
}

void done_in_progress_tile_setup( void ) {
    // get an app tile and copy mainstyle
    tile_num = mainbar_add_app_tile( 1, 1, "alarm in progress" );
    tile = mainbar_get_tile_obj( tile_num );

    lv_obj_add_style( tile, LV_OBJ_PART_MAIN, APP_STYLE );
    lv_style_copy( &label_style, APP_ICON_LABEL_STYLE );
    lv_style_set_text_font( &label_style, LV_STATE_DEFAULT, &Ubuntu_72px);

    lv_obj_t * cancel_btm = wf_add_close_button( tile, exit_event_callback, SYSTEM_ICON_STYLE );
    lv_obj_align( cancel_btm, tile, LV_ALIGN_IN_TOP_LEFT, THEME_PADDING, THEME_PADDING );

    label = wf_add_label( tile, "00:00" );
    lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    lv_obj_add_style( label, LV_OBJ_PART_MAIN, &label_style );
    lv_obj_align( label, tile, LV_ALIGN_CENTER, 0, 0 );

    lv_obj_t *countdown_icon = wf_add_image( tile, countdown_64px);
    lv_obj_align( countdown_icon, label, LV_ALIGN_OUT_BOTTOM_MID, 0, THEME_PADDING );

    styles_register_cb( STYLE_CHANGE, done_in_progress_style_change_event_cb, "alarm in progress style change event" );
}
