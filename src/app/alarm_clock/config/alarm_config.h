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
#ifndef _ALARM_CONFIG_H
    #define _ALARM_CONFIG_H

    #include "utils/basejsonconfig.h"

    #define ALARM_JSON_COFIG_FILE           "/alarm.json"   /** @brief defines json config file name */
    #define MAX_ALARM_ENTRYS                3               /** @brief max alarm entrys */

    /**
     * @brief alarm clock entry config
     */
    typedef struct {
        bool enable;                    /** @brief enable alarm */
        int32_t hour;                   /** @brief alarm hour */
        int32_t min;                    /** @brief alarm min */
        bool day[ 7 ];                  /** @brief alarm day */
    } alarm_entry_t;

    /**
     * @brief alarm structure
     */
    typedef struct {
        alarm_entry_t alarm_entry[ MAX_ALARM_ENTRYS ];
    } alarm_entrys_table_t;
    
    /**
     * @brief alarm config structure
     */
    class alarm_config_t : public BaseJsonConfig {
        public:
        alarm_config_t();

        alarm_entrys_table_t alarm_table;
        
        protected:
        ////////////// Available for overloading: //////////////
        virtual bool onLoad( JsonDocument& document );
        virtual bool onSave( JsonDocument& document );
        virtual bool onDefault( void );
        virtual size_t getJsonBufferSize() { return 1000; }
    } ;

#endif // _ALARM_CONFIG_H