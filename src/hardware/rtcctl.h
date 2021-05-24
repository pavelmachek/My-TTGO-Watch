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

#ifndef _RTCCTL_H
    #define _RTCCTL_H

    #include "TTGO.h"
    #include "callback.h"
    #include <time.h>

    #define RTCCTL_ALARM_OCCURRED    _BV(0)     /** @brief event mask for alarm occurred */
    #define RTCCTL_ALARM_TERM_SET    _BV(1)     /** @brief event mask for alarm set */     
    #define RTCCTL_ALARM_DISABLED    _BV(2)     /** @brief event mask for alarm disabled */
    #define RTCCTL_ALARM_ENABLED     _BV(3)     /** @brief event mask for alarm enabled */

    typedef struct{
        bool enable;
        struct tm alarm;
    } rtcctl_alarm_t;

    typedef struct{
        int32_t entrys;
        rtcctl_alarm_t *rtcctl_alarm;
    } rtcctl_alarm_table_t;

    /**
     * @brief setup rtc controller routine
     */
    void rtcctl_setup( void );
    /**
     * @brief rtc controller loop routine
     */
    void rtcctl_loop( void );
    /**
     * @brief registers a callback function which is called on a corresponding event
     * 
     * @param   event           possible values: RTCCTL_ALARM, RTCCTL_ALARM_SET, RTCCTL_ALARM_ENABLE and RTCCTL_ALARM_DISABLE
     * @param   callback_func   pointer to the callback function 
     * @param   id              program id
     * 
     * @return  true if success, false if failed
     */
    bool rtcctl_register_cb( EventBits_t event, CALLBACK_FUNC callback_func, const char *id );

    rtcctl_alarm_t *rtcctl_add_alarm_entry( void );

#endif // _RTCCTL_H
