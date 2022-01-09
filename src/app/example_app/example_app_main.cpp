/****************************************************************************
 *   Aug 3 12:17:11 2020
 *   Copyright  2020  Dirk Brosswick
 *   Copyright  2021-2022  Pavel Machek
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

#include "example_app.h"
#include "example_app_main.h"

#include "hardware/touch.h"

#include "gui/mainbar/app_tile/app_tile.h"
#include "gui/mainbar/main_tile/main_tile.h"
#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"
#include "gui/widget_factory.h"
#include "gui/widget_styles.h"

#include "utils/uri_load/uri_load.h"

#ifdef NATIVE_64BIT
    #include "utils/logging.h"
#else

#endif

lv_obj_t *example_app_main_tile = NULL;

lv_task_t * _example_app_task;

LV_IMG_DECLARE(refresh_32px);
LV_IMG_DECLARE(download_32px);
LV_IMG_DECLARE(t_watch_2020_240px);

LV_FONT_DECLARE(Ubuntu_16px);
LV_FONT_DECLARE(Ubuntu_32px);
LV_FONT_DECLARE(Ubuntu_72px);

#define DELAY 1

#define S_MAIN 0
#define S_ABOUT 1
#define S_WEATHER 2
#define S_REMOTE 3
int state;
lv_obj_t *objects[128];

static void run_weather_task( lv_task_t * task );
static void run_remote_task( lv_task_t * task );
static void run_image_task( lv_task_t * task );

#define C_LONG 1
#define C_INIT 2

struct click {
	int x, y;
	int type;
	char cookie[128];
};

struct click click;

static void exit_example_app_main_event_cb( lv_obj_t * obj, lv_event_t event );
static void enter_example_app_setup_event_cb( lv_obj_t * obj, lv_event_t event );
void example_app_task( lv_task_t * task );

lv_style_t example_app_main_style,  example_app_big_style, example_app_small_style;

lv_obj_t *big_btn = NULL;

bool example_button_cb( EventBits_t event, void *arg ) {
  printf("button callback\n"); fflush(stdout);
    switch( event ) {
    case BUTTON_LEFT:   printf("left button\n"); fflush(stdout);
                            break;
    case BUTTON_RIGHT:  printf("right button\n"); fflush(stdout);
                            break;
    }
    return( true );
}

bool example_app_touch_event_cb( EventBits_t event, void *arg ) {
  printf("touch cb %d\n", event); fflush(stdout);
    switch( event ) {
        case( TOUCH_UPDATE ):
            break;
    }
    return( false );
}

static void clear_screen(void)
{
  int i;

  for (i=0; i<sizeof(objects)/sizeof(*objects); i++) {
    if (objects[i]) {
      lv_obj_del(objects[i]);
      objects[i] = NULL;
    }
  }
    
}

static void example_activate_cb( void ) {
  printf("activate\n"); fflush(stdout);
}

static void example_hibernate_cb( void ) {
  printf("hibernate\n"); fflush(stdout);
}

#define M_TEXT 1
#define M_BIG 2
#define M_SMALL 4
#define M_IMG 8

struct display_list {
  int x, y, sx, sy;
  int mode;
  char *text;
};

#define S 40

#if 0
struct display_list d_about[] = {
	{ .sx = 6*S, .sy = 2*S,
	  .mode = M_TEXT | M_BIG,
	  .text = "Hello,", },
	{ .y = 2*S, .sx = 6*S, .sy = 3*S,
	  .mode = M_TEXT | M_SMALL,
	  .text = "Even wristwatch should run free\n"
	          "software. Esp32 means it really\n"
	          "is a small computer." },
	{ .y = 4*S, .sx = 6*S, .sy = 2*S,
	  .mode = M_TEXT,
	  .text = "Good luck :-)\n[Close]" },	
};

struct display_list d_wait[] = {
	{ .sx = 6*S, .sy = 2*S,
	  .mode = M_TEXT | M_BIG,
	  .text = "Wait...", },
};

struct display_list d_main[] = {
	{ .sx = 6*S, .sy = S,
	  .mode = M_TEXT | M_SMALL,
	  .text = "Main menu", },
	{ .y = S, .sx = 6*S, .sy = 2*S,
	  .mode = M_TEXT,
	  .text = "[About]" },
	{ .y = 3*S, .sx = 6*S, .sy = 2*S,
	  .mode = M_TEXT,
	  .text = "[Weather]" },
	{ .y = 5*S, .sx = 6*S, .sy = 2*S,
	  .mode = M_TEXT,
	  .text = "[Remote]" },
};

struct display_list d_weather[] = {
	{ .sx = 6*S, .sy = S,
	  .mode = M_TEXT | M_SMALL,
	  .text = "Weather report", },
	{ .y = S, .sx = 6*S, .sy = 2*S,
	  .mode = M_TEXT | M_SMALL,
	  .text = "(weather goes here)" },
};
#else
struct display_list d_about[4], d_main[4], d_weather[4], d_wait[4], d_remote[40];

static int dl_parse(struct display_list *res, int num, const char *t);

static void dl_parse_all(void)
{
	int r;
	r = dl_parse(d_main, 4,
		     "0 0 240 40 5 Main menu\a"
		     "0 40 240 80 1 [About]\a"
		     "0 120 240 80 1 [Weather]\a"
		     "0 200 240 80 1 [Remote]\a"		     
		);
	r = dl_parse(d_about, 4, "0 0 240 40 3 About\a"
		                 "0 80 240 80 5 Even wristwatch should run free\n"
		     "software. Esp32 means it really\n"
		     "is a small computer.\a"
		     "0 160 240 160 8");
	
	r = dl_parse(d_weather, 4,
		     "0 0 240 40 5 Weather\a"
		     "0 40 240 200 5 (results go here)\a"
		);
	r = dl_parse(d_wait, 4, "0 0 240 40 5 Wait");
}


#endif

static lv_obj_t *lvo_img;

static void display(display_list *display, int num)
{
    for (int i=0; i<num; i++) {
	    struct display_list *l = display+i;
	    lv_obj_t *lvo = NULL;

	    if (l->mode & M_IMG) {
	      lvo = lv_img_create( example_app_main_tile, NULL );
	      lv_img_set_src( lvo, &download_32px );
	      lvo_img = lvo;
	    } else if (l->mode & M_TEXT) {
	      lvo = lv_label_create( example_app_main_tile, NULL);

	      if (l->mode & M_BIG)
		    lv_obj_add_style( lvo, LV_OBJ_PART_MAIN, &example_app_big_style  );
	      else if (l->mode & M_SMALL)
		lv_obj_add_style( lvo, LV_OBJ_PART_MAIN, &example_app_small_style  );
	      else
		    lv_obj_add_style( lvo, LV_OBJ_PART_MAIN, &example_app_main_style  );

	      lv_label_set_text(lvo, l->text);
	    } else {
	      objects[i] = NULL;
	      continue;
	    }
    //    lv_obj_align( lvo, example_app_main_tile, LV_ALIGN_IN_TOP_MID, 0, 0 );
	    lv_obj_set_width(lvo, l->sx);
	    lv_obj_set_height(lvo, l->sy);
	    lv_obj_set_pos(lvo, l->x, l->y);

	    objects[i] = lvo;
    }
}

static void exit_big_app_tile_event_cb( lv_obj_t * obj, lv_event_t event ) {
    int x, y;
    x = obj->coords.x1;
    y = obj->coords.y1;
    click.x = x;
    click.y = y;
    click.type = 0;
    printf("obj @ %d %d\n", x, y);
    switch( event ) {
        case( LV_EVENT_SHORT_CLICKED ):
	    printf("big_btn -- short\n"); fflush(stdout);
                                        break;
        case( LV_EVENT_LONG_PRESSED ):
	    printf("big_btn -- long\n"); fflush(stdout);
	    click.type |= C_LONG;
                                        break;
    default:
	    return;
    }

    clear_screen();
    switch (state) {
    case S_MAIN:
	    switch (y) {
	    case 0 ... 2*S-1:
		    state = S_ABOUT; display(d_about, sizeof(d_about)/sizeof(*d_about));
		    lv_task_create( run_image_task, DELAY, LV_TASK_PRIO_MID, NULL );
		    break;
	    case 2*S ... 4*S-1:
		    state = S_WEATHER; display(d_wait, sizeof(d_wait)/sizeof(*d_wait));
		    lv_task_create( run_weather_task, DELAY, LV_TASK_PRIO_MID, NULL );
		    break;
	    case 4*S ... 6*S:
	      click.type |= C_INIT;
		    state = S_REMOTE; display(d_wait, sizeof(d_wait)/sizeof(*d_wait));
		    lv_task_create( run_remote_task, DELAY, LV_TASK_PRIO_MID, NULL );
		    break;
	    }
	    break;
    case S_WEATHER:
    case S_ABOUT:
	    state = S_MAIN; display(d_main, sizeof(d_main)/sizeof(*d_main));
	    break;
    case S_REMOTE:
	    lv_task_create( run_remote_task, DELAY, LV_TASK_PRIO_MID, NULL );
	    break;
    }
}

void example_app_main_setup( uint32_t tile_num ) {
    int sx = lv_disp_get_hor_res( NULL ), sy = lv_disp_get_ver_res( NULL );

    example_app_main_tile = mainbar_get_tile_obj( tile_num );

#if 0    
    lv_obj_t * exit_btn = wf_add_exit_button( example_app_main_tile, exit_example_app_main_event_cb );
    lv_obj_align(exit_btn, example_app_main_tile, LV_ALIGN_IN_BOTTOM_LEFT, THEME_ICON_PADDING, -THEME_ICON_PADDING );

    lv_obj_t * setup_btn = wf_add_setup_button( example_app_main_tile, enter_example_app_setup_event_cb );
    lv_obj_align(setup_btn, example_app_main_tile, LV_ALIGN_IN_BOTTOM_RIGHT, -THEME_ICON_PADDING, -THEME_ICON_PADDING );
#endif

#if 0
    lv_style_copy( &example_app_main_style, APP_STYLE );
    lv_style_set_text_font( &example_app_main_style, LV_STATE_DEFAULT, &Ubuntu_16px);
    lv_obj_add_style( example_app_main_tile, LV_OBJ_PART_MAIN, &example_app_main_style );

    test_label = lv_label_create( example_app_main_tile, NULL);
    lv_obj_add_style( test_label, LV_OBJ_PART_MAIN, &example_app_main_style  );
    lv_label_set_text( test_label, "Even wristwatch\nshould run\nfree software.\nEsp32 means it\nis small compu-\nter really.");
    //    lv_obj_align( test_label, example_app_main_tile, LV_ALIGN_IN_TOP_MID, 0, 0 );
    lv_obj_set_width( test_label, sx);
    lv_obj_set_height( test_label, sy);
    lv_obj_set_pos( test_label, 0, 0);
#endif

    dl_parse_all();

    lv_style_copy( &example_app_main_style, APP_STYLE );
    lv_style_set_text_font( &example_app_main_style, LV_STATE_DEFAULT, &Ubuntu_32px);
    lv_obj_add_style( example_app_main_tile, LV_OBJ_PART_MAIN, &example_app_main_style );

    lv_style_copy( &example_app_small_style, APP_STYLE );
    lv_style_set_text_font( &example_app_small_style, LV_STATE_DEFAULT, &Ubuntu_16px);
    lv_obj_add_style( example_app_main_tile, LV_OBJ_PART_MAIN, &example_app_small_style );

    lv_style_copy( &example_app_big_style, APP_STYLE );
    lv_style_set_text_font( &example_app_big_style, LV_STATE_DEFAULT, &Ubuntu_72px);
    lv_obj_add_style( example_app_main_tile, LV_OBJ_PART_MAIN, &example_app_big_style );

    state = S_MAIN; display(d_main, sizeof(d_main)/sizeof(*d_main));    

    {
      int x, y;
      for (x=0; x<6; x++)
	for (y=0; y<6; y++) {
	  big_btn = lv_btn_create( example_app_main_tile, NULL );
	  lv_obj_set_width( big_btn, sx/6 );
	  lv_obj_set_height( big_btn, sy/6 );
	  lv_obj_set_pos( big_btn, (x*sx)/6, (y*sy)/6 );
	  lv_obj_add_protect( big_btn, LV_PROTECT_CLICK_FOCUS );
	  lv_obj_add_style( big_btn, LV_OBJ_PART_MAIN, &example_app_main_style );
	  lv_obj_set_event_cb( big_btn, exit_big_app_tile_event_cb );
	}
    }
    
#if 1

    // FIXME: should use _activate_cb
    _example_app_task = lv_task_create( example_app_task, 5000, LV_TASK_PRIO_MID, NULL );
#endif

    mainbar_add_tile_activate_cb( tile_num, example_activate_cb );
    mainbar_add_tile_hibernate_cb( tile_num, example_hibernate_cb );
    mainbar_add_tile_button_cb( tile_num, example_button_cb );
    
    //    touch_register_cb( TOUCH_UPDATE , example_app_touch_event_cb, "osm touch" );
}

static void enter_example_app_setup_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_to_tilenumber( example_app_get_app_setup_tile_num(), LV_ANIM_ON );
                                        statusbar_hide( true );
                                        break;
    }
}

static void exit_example_app_main_event_cb( lv_obj_t * obj, lv_event_t event ) {
    switch( event ) {
        case( LV_EVENT_CLICKED ):       mainbar_jump_back();
                                        break;
    }
}


char *skip_to(char *s, int c) {
    char *t;

    t = strchr(s, c);
    if (!t)
    	return s;
    return t+1;
}

static void run_weather_task( lv_task_t * task ) {
  //char url[] = "https://tgftp.nws.noaa.gov/data/observations/metar/decoded/LKPR.TXT";
	//char url[] = "https://tgftp.nws.noaa.gov/data/observations/metar/stations/LKPR.TXT";
	char url[] = "http://10.0.0.9:8000/LKPR.TXT";

    printf("Loading...\n"); fflush(stdout);
    
    uri_load_dsc_t *uri_load_dsc = uri_load_to_ram( url );

    if (!uri_load_dsc) {
	    printf("Some kind of error loading url\n");
	    d_weather[1].text = "Error loading url";
	    display(d_weather, sizeof(d_weather)/sizeof(*d_weather));
	    return;
    }

    printf("Got it... %d bytes, %s\n", uri_load_dsc->size, uri_load_dsc->data); fflush(stdout);
    clear_screen();
#define SIZE 1024
    static char data[SIZE];
    int s = uri_load_dsc->size;
    if (s > SIZE-1)
        s = SIZE-1;

    memcpy(data, uri_load_dsc->data, s);
    data[s] = 0;
    char *metar = skip_to(data, '\n');

    // LKPR 122000Z 27005KT CAVOK 05/03 Q1015 NOSIG

    metar = skip_to(metar, ' ');
    metar = skip_to(metar, ' ');

    {
    char *s = metar;
    while (*s) {
    	if (*s == ' ')
	    *s = '\n';
	s++;
    }
    }

    d_weather[1].text = data;
    display(d_weather, sizeof(d_weather)/sizeof(*d_weather));
    lv_task_del(task);
}

static void run_remote_task( lv_task_t * task ) {
	char url[128];
	int r;

	sprintf(url, "http://10.0.0.9:8000/cgi-bin/remote.py?x=%d&y=%d&type=%d&cookie=%s",
		click.x, click.y, click.type, click.cookie);

	printf("Loading...%s\n", url); fflush(stdout);
    
    uri_load_dsc_t *uri_load_dsc = uri_load_to_ram( url );

    if (!uri_load_dsc) {
	    printf("Some kind of error loading url\n");
	    d_weather[1].text = "Error loading url";
	    display(d_weather, sizeof(d_weather)/sizeof(*d_weather));
	    return;
    }

    printf("Got it... %d bytes, %s\n", uri_load_dsc->size, uri_load_dsc->data); fflush(stdout);
    clear_screen();
#define SIZE 1024
    static char data[SIZE], *t;
    int s = uri_load_dsc->size;
    if (s > SIZE-1)
        s = SIZE-1;

    memcpy(data, uri_load_dsc->data, s);
    data[s] = 0;

    t = strchr(data, '\n');
    if (!t) {
	    printf("No cookie\n");
	    d_weather[1].text = "No cookie";
	    display(d_weather, sizeof(d_weather)/sizeof(*d_weather));
	    return;
    }
    *t++ = 0;
    strcpy(click.cookie, data);
    
    r = dl_parse(d_remote, 40, t);
    if (r < 0) {
	    printf("Error parsing remote data\n");
	    d_weather[1].text = "Error parsing remote url";
	    display(d_weather, sizeof(d_weather)/sizeof(*d_weather));
	    return;
    }
    clear_screen();
    display(d_remote, sizeof(d_remote)/sizeof(*d_remote));
    lv_task_del(task);
}

static void run_image_task( lv_task_t * task ) {
	char url[128];
	int r;

	sprintf(url, "http://10.0.0.9:8000/remote.raw");

	printf("Loading...%s\n", url); fflush(stdout);
    
    uri_load_dsc_t *uri_load_dsc = uri_load_to_ram( url );

    if (!uri_load_dsc) {
	    printf("Some kind of error loading url\n");
	    d_weather[1].text = "Error loading url";
	    display(d_weather, sizeof(d_weather)/sizeof(*d_weather));
	    return;
    }

    printf("Got it... %d bytes\n", uri_load_dsc->size); fflush(stdout);

    printf("LV_COLOR_DEPTH is %d, need 32\n", LV_COLOR_DEPTH);

    lv_img_dsc_t raw_img;
    raw_img.header.always_zero = 0;
    raw_img.header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA;
    raw_img.header.w = 32;
    raw_img.header.h = 32;

    raw_img.data = uri_load_dsc->data;
    raw_img.data_size = uri_load_dsc->size;
      
    lv_img_set_src( lvo_img, &raw_img );

    printf("Image set\n", uri_load_dsc->size); fflush(stdout);
    
    lv_task_del(task);
}

static int dl_parse(struct display_list *res, int num, const char *arg)
{
	int i = 0;
	char *end;
	char *t = strdup(arg);
	printf("Parsing: %s\n", t); fflush(stdout);
	for (i=0; i<num; i++) {
		if (!t || !*t) {
			res->mode = 0;
			res++;
			continue;
		}

		int r;
		int num;
		end = strchr(t, '\a');
		if (end) {
			*end = 0;
		}

		r = sscanf(t, "%d%d%d%d%d%n", &res->x, &res->y, &res->sx, &res->sy, &res->mode, &num);
		res->text = t+num;
		
		if (r != 5) {
			printf("Could not parse: %d %s\n", r, t);
			return -1;
		}
		printf("Parsed: %s\n", res->text); fflush(stdout);
		res++;
		
		if (!end) {
			t = NULL;
			continue;
		}
		t = end+1;
	}
	return 0;
}

static void run_weather(void) {
	lv_task_create( run_weather_task, DELAY, LV_TASK_PRIO_MID, NULL );
}

void example_app_task( lv_task_t * task ) {
    static int time;
    char buf[1024];
#if 0
    printf("Tick: %d\n", time);
    sprintf(buf, "hell %d\nfoo\nbar\n", time++);
    lv_label_set_text(test_label, buf);
#endif
#if 0
    fetch_url();
#endif
}
