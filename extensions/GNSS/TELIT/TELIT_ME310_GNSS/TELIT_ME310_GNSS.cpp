/**
 * ep-oc-mcu
 * Embedded Planet Open Core for Microcontrollers
 * 
 * Built with ARM Mbed-OS
 *
 * Copyright (c) 2021 Embedded Planet, Inc.
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// Only compile this if the config is enabled
#if TELIT_ME310_GNSS_ENABLED

#include "TELIT_ME310_GNSS.h"
#include "libnmea/src/parsers/parse.h"

#define TRACE_GROUP   "GNSS"

using namespace ep;
using namespace std::chrono_literals;

TELIT_ME310_GNSS::TELIT_ME310_GNSS()
{
    dev = CellularDevice::get_target_default_instance();
    at_handler = dev->get_at_handler();
}

void TELIT_ME310_GNSS::init()
{
    // Check if the device is ready
    if (dev->is_ready() != NSAPI_ERROR_OK) {
        dev->soft_power_on();
        ThisThread::sleep_for(10s);
        dev->init();
    }
}

GNSS::GNSSError TELIT_ME310_GNSS::enable()
{
    at_handler->lock();

    at_handler->at_cmd_discard("$GPSP", "=", "%d", 1);
    if (at_handler->unlock_return_error() == NSAPI_ERROR_OK) {
        return GNSS_ERROR_OK;
    }

    return GNSS_ERROR_UNKNOWN_ERROR;
}

GNSS::GNSSError TELIT_ME310_GNSS::disable()
{
    at_handler->lock();

    at_handler->at_cmd_discard("$GPSP", "=", "%d", 0);
    if (at_handler->unlock_return_error() == NSAPI_ERROR_OK) {
        return GNSS_ERROR_OK;
    }

    return GNSS_ERROR_UNKNOWN_ERROR;
}

GNSS::PositionInfo TELIT_ME310_GNSS::get_current_position()
{
    PositionInfo position_info;

    at_handler->lock();

    at_handler->cmd_start("AT$GPSACP");
    at_handler->cmd_stop();
    at_handler->resp_start("$GPSACP:");
    
    // Get timestamp
    std::string utc_time_stamp;
    int utc_time_stamp_len = at_handler->read_string((char *)utc_time_stamp.c_str(), 11);
    if (utc_time_stamp_len <= 0) {
        tr_warn("Could not read position");

        // Mark as an invalid fix to show an error
        position_info.Fix = FIX_TYPE_INVALID;
        at_handler->resp_stop();
        at_handler->unlock();
        return position_info;
    }

    // Get latitude
    std::string latitude;
    int latitude_len = at_handler->read_string((char *)latitude.c_str(), 12);
    if (latitude_len > 0) {
        // Latitude position
        nmea_position latitude_pos;

        // Parse degrees/minutes
        nmea_position_parse((char *)latitude.c_str(), &latitude_pos);

        // Parse cardinal direction
        std::string lat_cardinal = latitude.substr(latitude_len - 1, 1);
        latitude_pos.cardinal = nmea_cardinal_direction_parse((char *)lat_cardinal.c_str());

        position_info.Latitude = latitude_pos;
    }

    // Get longitude
    std::string longitude;
    int longitude_len = at_handler->read_string((char *)longitude.c_str(), 13);
    if (longitude_len > 0) {
        // Longitude position
        nmea_position longitude_pos;

        // Parse degrees/minutes
        nmea_position_parse((char *)longitude.c_str(), &longitude_pos);

        // Parse cardinal direction
        std::string long_cardinal = longitude.substr(longitude_len - 1, 1);
        longitude_pos.cardinal = nmea_cardinal_direction_parse((char *)long_cardinal.c_str());

        position_info.Longitude = longitude_pos;
    }

    // Get horizontal dilution of precision
    char hdop[4];
    int hdop_len = at_handler->read_string(hdop, sizeof(hdop));
    if (hdop_len > 0) {
        position_info.HorizontalDilutionOfPrecision = atof(hdop);
    }

    // Get altitude
    char altitude[4];
    int altitude_len = at_handler->read_string(altitude, sizeof(altitude));
    if (altitude_len > 0) {
        position_info.Altitude = atof(altitude);
    }

    // Get fix
    int fix = at_handler->read_int();
    switch(fix) {
        case 0:
        case 1:
            position_info.Fix = FIX_TYPE_INVALID;
            break;
        case 2:
            position_info.Fix = FIX_TYPE_2D;
            break;
        case 3:
            position_info.Fix = FIX_TYPE_3D;
            break;
        default:
            position_info.Fix = FIX_TYPE_UNKNOWN;
            break;
    }

    // Get course over ground
    char cog[7];
    int cog_len = at_handler->read_string(cog, sizeof(cog));
    if (cog_len > 0) {
        position_info.CourseOverGround = atof(cog);
    }

    // Get speed over ground (km/hr)
    char kmhr[6];
    int kmhr_len = at_handler->read_string(kmhr, sizeof(kmhr));
    if (kmhr_len > 0) {
        position_info.SpeedOverGround = atof(kmhr);
    }

    // Get speed over ground (knots) -> skip because we don't care about knots
    at_handler->skip_param();

    // Get date
    std::string date;
    int date_len = at_handler->read_string((char *)date.c_str(), 7);

    // Get total number of satellites
    position_info.NumberOfSatellites = at_handler->read_int();

    // Build timestamp
    if ((date_len == 6) && (utc_time_stamp_len == 10)) {
        int year = atoi(date.substr(4, 2).c_str()) + 2000;
        int month = atoi(date.substr(2, 2).c_str());
        int mday = atoi(date.substr(0, 2).c_str());
        int hour = atoi(utc_time_stamp.substr(0, 2).c_str());
        int minute = atoi(utc_time_stamp.substr(2, 2).c_str());
        int second = (int)std::lround(atof(utc_time_stamp.substr(4, 2).c_str()));
        position_info.UtcTimestamp = as_unix_time(year, month, mday, hour, minute, second);
    }

    at_handler->resp_stop();
    at_handler->unlock();

    return position_info;
}

GNSS::GNSSError TELIT_ME310_GNSS::set_gnss_priority(GNSSPriority desired_priority)
{
    at_handler->lock();

    at_handler->at_cmd_discard("$GPSCFG", "=", "%d%d", GPSCFG_SET_WWAN_GNSS_PRIORITY, desired_priority);
    if (at_handler->unlock_return_error() == NSAPI_ERROR_OK) {
        return GNSS_ERROR_OK;
    }

    return GNSS_ERROR_UNKNOWN_ERROR;
}

time_t TELIT_ME310_GNSS::as_unix_time(int year, int mon, int mday, int hour, int min, int sec)
{
    struct tm   t;
    t.tm_year = year - 1900;
    t.tm_mon =  mon - 1;        // convert to 0 based month
    t.tm_mday = mday;
    t.tm_hour = hour;
    t.tm_min = min;
    t.tm_sec = sec;
    t.tm_isdst = -1;            // Is Daylight saving time on? 1 = yes, 0 = no, -1 = unknown

    return mktime(&t);          // returns seconds elapsed since January 1, 1970 (begin of the Epoch)
}


#endif /* TELIT_ME310_GNSS_ENABLED */