/****************************************************************************
 *   Aug 3 12:17:11 2020
 *   Copyright  2020  Dirk Brosswick
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

#include "web.h"
#include "web_main.h"
#include "web_setup.h"

#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"
#include "gui/app.h"
#include "gui/widget.h"

#ifdef NATIVE_64BIT
    #include "utils/logging.h"
#else

#endif

uint32_t web_main_tile_num;
uint32_t web_setup_tile_num;

// app icon
icon_t *web = NULL;

// widget icon
icon_t *example_widget = NULL;

// declare you images or fonts you need
LV_IMG_DECLARE(web_64px);
LV_IMG_DECLARE(info_1_16px);

// declare callback functions for the app and widget icon to enter the app
static void enter_web_event_cb( lv_obj_t * obj, lv_event_t event );
static void enter_example_widget_event_cb( lv_obj_t * obj, lv_event_t event );

/*
 * setup routine for example app
 */
void web_setup( void ) {
    #if defined( ONLY_ESSENTIAL )
        return;
    #endif
    // register 2 vertical tiles and get the first tile number and save it for later use
    web_main_tile_num = mainbar_add_app_tile( 1, 1, "example app" );
    web_setup_tile_num = mainbar_add_setup_tile( 1, 1, "example app setup" );

    // register app icon on the app tile
    // set your own icon and register her callback to activate by an click
    // remember, an app icon must have an size of 64x64 pixel with an alpha channel
    // use https://lvgl.io/tools/imageconverter to convert your images and set "true color with alpha" to get fancy images
    // the resulting c-file can put in /app/examples/images/ and declare it like LV_IMG_DECLARE( your_icon );
    web = app_register( "myapp", &web_64px, enter_web_event_cb );
    app_set_indicator( web, ICON_INDICATOR_OK );

#ifdef EXAMPLE_WIDGET
    // register widget icon on the main tile
    // set your own icon and register her callback to activate by an click
    // remember, an widget icon must have an max size of 64x64 pixel
    // use https://lvgl.io/tools/imageconverter to convert your images and set "true color with alpha" to get fancy images
    // the resulting c-file can put in /app/examples/images/ and declare it like LV_IMG_DECLARE( your_icon );
    example_widget = widget_register( "myapp", &web_64px, enter_example_widget_event_cb );
    widget_set_indicator( example_widget, ICON_INDICATOR_UPDATE );
#endif // EXAMPLE_WIDGET

    // init main and setup tile, see web_main.cpp and web_setup.cpp
    web_main_setup( web_main_tile_num );
    web_setup_setup( web_setup_tile_num );
}

/*
 *
 */
uint32_t web_get_app_main_tile_num( void ) {
    return( web_main_tile_num );
}

/*
 *
 */
uint32_t web_get_app_setup_tile_num( void ) {
    return( web_setup_tile_num );
}

/*
 *
 */
static void enter_web_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_tilenumber( web_main_tile_num, LV_ANIM_OFF );
                                        statusbar_hide( true );
                                        app_hide_indicator( web );
                                        break;
    }    
}

/*
 *
 */
static void enter_example_widget_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       statusbar_hide( true );
                                        widget_hide_indicator( example_widget );
                                        mainbar_jump_to_tilenumber( web_main_tile_num, LV_ANIM_OFF );
                                        break;
    }    
}