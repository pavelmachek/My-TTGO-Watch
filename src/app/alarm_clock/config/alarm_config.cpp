/****************************************************************************
 *   Aug 11 17:13:51 2020
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
#include "alarm_config.h"

alarm_config_t::alarm_config_t() : BaseJsonConfig( ALARM_JSON_COFIG_FILE ) {}

bool alarm_config_t::onSave( JsonDocument& doc ) {
    for( int entry = 0 ; entry < MAX_ALARM_ENTRYS; entry++ ) {
        doc["alarm"][ entry ]["enable"] = alarm_table.alarm_entry[ entry ].enable;
        doc["alarm"][ entry ]["hour"] = alarm_table.alarm_entry[ entry ].hour;
        doc["alarm"][ entry ]["min"] = alarm_table.alarm_entry[ entry ].min;
        for( int day = 0 ; day < 7 ; day++ ) {
            doc["alarm"][ entry ]["day"][ day ] = alarm_table.alarm_entry[ entry ].day[ day ];
        }
    }
    return true;
}

bool alarm_config_t::onLoad( JsonDocument& doc ) {

    for( int entry = 0 ; entry < MAX_ALARM_ENTRYS; entry++ ) {
        alarm_table.alarm_entry[ entry ].enable = doc["alarm"][ entry ]["enable"] | false;
        alarm_table.alarm_entry[ entry ].hour = doc["alarm"][ entry ]["hour"] | 0;
        alarm_table.alarm_entry[ entry ].min = doc["alarm"][ entry ]["min"] | 0;
        for( int day = 0 ; day < 7 ; day++ ) {
            alarm_table.alarm_entry[ entry ].day[ day ] = doc["alarm"][ entry ]["day"][ day ] | false;
        }
    }
    return true;
}

bool alarm_config_t::onDefault( void ) {
    for( int entry = 0 ; entry < MAX_ALARM_ENTRYS; entry++ ) {
        alarm_table.alarm_entry[ entry ].enable = false;
        alarm_table.alarm_entry[ entry ].hour = 0;
        alarm_table.alarm_entry[ entry ].min = 0;
        for( int day = 0 ; day < 7 ; day++ ) {
            alarm_table.alarm_entry[ entry ].day[ day ] = false;
        }
    }
    return true;
}