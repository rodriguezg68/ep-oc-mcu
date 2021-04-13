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
#if TELIT_ME910_GNSS_ENABLED

#include "TELIT_ME910_GNSS.h"
#include "libnmea/src/parsers/parse.h"
#include "TinyGPSplus.h"

#include <cstdlib>

#define TRACE_GROUP   "GNSS"

using namespace ep;
using namespace std::chrono_literals;

TELIT_ME910_GNSS::TELIT_ME910_GNSS()
{
    dev = CellularDevice::get_target_default_instance();
    at_handler = dev->get_at_handler();

    // Set up URC callbacks
    at_handler->set_urc_handler(GPGGA_SENTENCE_URC_PREFIX, mbed::Callback<void()>(this, &TELIT_ME910_GNSS::urc_gpgga));
    at_handler->set_urc_handler(GNRMC_SENTENCE_URC_PREFIX, mbed::Callback<void()>(this, &TELIT_ME910_GNSS::urc_gnrmc));
    at_handler->set_urc_handler(GPGSV_SENTENCE_URC_PREFIX, mbed::Callback<void()>(this, &TELIT_ME910_GNSS::urc_gpgsv));

}

void TELIT_ME910_GNSS::urc_gpgga()
{
    char sentence[100];
    strcpy(sentence, GPGGA_SENTENCE_URC_PREFIX);

    at_handler->lock();

    // Temporarily set delimiter to '\n' to grab the whole sentence
    at_handler->set_delimiter('\n');

    // Offset by the length of the sentence prefix
    at_handler->read_string(sentence + strlen(GPGGA_SENTENCE_URC_PREFIX), sizeof(sentence) - strlen(GPGGA_SENTENCE_URC_PREFIX));

    // Reset the delimiter
    at_handler->set_default_delimiter();
    at_handler->unlock();

    // Loop through the sentence and pass it to the TinyGPSPlus library
    char *iterator;
    for (iterator = sentence; *iterator != '\0'; iterator++) {
        values.encode(*iterator);
    }
}

void TELIT_ME910_GNSS::urc_gnrmc()
{
    char sentence[100];
    strcpy(sentence, GNRMC_SENTENCE_URC_PREFIX);

    at_handler->lock();

    // Temporarily set delimiter to '\n' to grab the whole sentence
    at_handler->set_delimiter('\n');

    // Offset by the length of the sentence prefix
    at_handler->read_string(sentence + strlen(GNRMC_SENTENCE_URC_PREFIX), sizeof(sentence) - strlen(GNRMC_SENTENCE_URC_PREFIX));

    // Reset the delimiter
    at_handler->set_default_delimiter();
    at_handler->unlock();

    // Loop through the sentence and pass it to the TinyGPSPlus library
    char *iterator;
    for (iterator = sentence; *iterator != '\0'; iterator++) {
        values.encode(*iterator);
    }
}

void TELIT_ME910_GNSS::urc_gpgsv()
{
    char sentence[100];

    at_handler->lock();

    // Temporarily set delimiter to '\n' to grab the whole sentence
    at_handler->set_delimiter('\n');

    // Offset by the length of the sentence prefix
    at_handler->read_string(sentence, sizeof(sentence));

    // Reset the delimiter
    at_handler->set_default_delimiter();
    at_handler->unlock();

    // Get the number of this GPGSV message (we only care about the first one)
    char* field_start = sentence;
    char* end_ptr;
    field_start = strchr(sentence, ',') + 1;
    if(!(strtol(field_start, &end_ptr, 10) == 1)) {
        return;
    }

    // The next field will be the satellites in view
    field_start = strchr(field_start, ',') + 1;
    satellites_in_view = strtol(field_start, &end_ptr, 10);

}

void TELIT_ME910_GNSS::init()
{
    at_handler->set_at_timeout(500, true);

    // Check if the device is ready
    if (dev->is_ready() != NSAPI_ERROR_OK) {
        dev->hard_power_on();
        ThisThread::sleep_for(250ms);
        dev->soft_power_on();
        ThisThread::sleep_for(10s);
        dev->init();
    }
}

GNSS::GNSSError TELIT_ME910_GNSS::enable()
{
    at_handler->lock();

    at_handler->at_cmd_discard("$GPSP", "=", "%d", 1);
    if (at_handler->unlock_return_error() == NSAPI_ERROR_OK) {
        return GNSS_ERROR_OK;
    }

    return GNSS_ERROR_UNKNOWN_ERROR;
}

GNSS::GNSSError TELIT_ME910_GNSS::disable()
{
    at_handler->lock();

    at_handler->at_cmd_discard("$GPSP", "=", "%d", 0);
    if (at_handler->unlock_return_error() == NSAPI_ERROR_OK) {
        return GNSS_ERROR_OK;
    }

    return GNSS_ERROR_UNKNOWN_ERROR;
}

GNSS::PositionInfo TELIT_ME910_GNSS::get_current_position(bool use_urcs)
{
    PositionInfo position_info;
    nmea_position latitude_pos;
    nmea_position longitude_pos;

    if (use_urcs) {
        // Fix is valid, so fill in the data values
        // Latitude
        // nmea_position latitude_pos;
        latitude_pos.degrees = abs(values.location.rawLat().deg);
        latitude_pos.minutes = (values.location.rawLat().billionths / 1000000000.0) * 60; // Convert to minutes
        latitude_pos.cardinal = values.location.rawLat().negative ? NMEA_CARDINAL_DIR_SOUTH : NMEA_CARDINAL_DIR_NORTH;
        position_info.Latitude = latitude_pos;

        // Longitude
        // nmea_position longitude_pos;
        longitude_pos.degrees = abs(values.location.rawLng().deg);
        longitude_pos.minutes = (values.location.rawLng().billionths / 1000000000.0) * 60; // Convert to minutes
        longitude_pos.cardinal = values.location.rawLng().negative ? NMEA_CARDINAL_DIR_WEST : NMEA_CARDINAL_DIR_EAST;
        position_info.Longitude = longitude_pos;

        // Horizontal dilution of precision
        position_info.HorizontalDilutionOfPrecision = values.hdop.value() / 100.0;

        // Altitude
        position_info.Altitude = values.altitude.meters();

        // Fix type (fix is 3D if altitude value is valid)
        if(values.location.isValid()) {
            position_info.Fix = values.altitude.isValid() ? FIX_TYPE_3D : FIX_TYPE_2D;
        } else {
            /* In this case, only some values will be valid (eg: Satellites in View) */
            position_info.Fix = FIX_TYPE_INVALID;
        }

        // Course over ground
        position_info.CourseOverGround = values.course.deg();

        // Speed over ground
        position_info.SpeedOverGround = values.speed.kmph();

        // Number of satellites
        if(satellites_in_view) {
            position_info.NumberOfSatellites = (uint8_t) satellites_in_view;
        }
        else if(values.satellites.isValid()) {
            position_info.NumberOfSatellites = values.satellites.value();
        } else {
            position_info.NumberOfSatellites = 0;
        }

        // Timestamp
        position_info.UtcTimestamp = as_unix_time(
                                        values.date.year(),
                                        values.date.month(),
                                        values.date.day(),
                                        values.time.hour(),
                                        values.time.minute(),
                                        values.time.second());
        return position_info;
    }

    at_handler->lock();
    at_handler->cmd_start_stop("$GPSACP", "");
    at_handler->resp_start("$GPSACP:");
    int read_len = 0;
    // uint16_t n_vals;
    // char *values[255];
    position_info.Fix = FIX_TYPE_UNKNOWN;

    // Get timestamp
    char utc_timestamp[11];
    read_len = at_handler->read_string(utc_timestamp, sizeof(utc_timestamp));
    if (read_len <= 0) {
        position_info.Fix = FIX_TYPE_INVALID;
    }
    
    // Get latitude
    char latitude[12];
    read_len = at_handler->read_string(latitude, sizeof(latitude));
    if (read_len > 0) {
        latitude_pos.cardinal = nmea_cardinal_direction_parse(&latitude[read_len - 1]);
        latitude[read_len - 1] = '\0';
        nmea_position_parse(latitude, &latitude_pos);
        position_info.Latitude = latitude_pos;
    } else {
        position_info.Fix = FIX_TYPE_INVALID;
    }
    
    // Get longitude
    char longitude[13];
    read_len = at_handler->read_string(longitude, sizeof(longitude));
    if (read_len > 0) {
        longitude_pos.cardinal = nmea_cardinal_direction_parse(&longitude[read_len - 1]);
        longitude[read_len - 1] = '\0';
        nmea_position_parse(longitude, &longitude_pos);
        position_info.Longitude = longitude_pos;
    } else {
        position_info.Fix = FIX_TYPE_INVALID;
    }

    // Get HDOP
    char hdop[4];
    read_len = at_handler->read_string(hdop, sizeof(hdop));
    if (read_len > 0) {
        position_info.HorizontalDilutionOfPrecision = TinyGPSPlus::parseDecimal(hdop) / 100.0;
    } else {
        position_info.Fix = FIX_TYPE_INVALID;
    }

    // Get altitude
    char altitude[4];
    read_len = at_handler->read_string(altitude, sizeof(altitude));
    if (read_len > 0) {
        position_info.Altitude = TinyGPSPlus::parseDecimal(altitude) / 100.0;
    } else {
        position_info.Fix = FIX_TYPE_INVALID;
    }

    // Get fix
    switch (at_handler->read_int()) {
        default:
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
    }

    // Get course over ground
    char cog[7];
    read_len = at_handler->read_string(cog, sizeof(cog));
    if (read_len > 0) {
        position_info.CourseOverGround = TinyGPSPlus::parseDecimal(cog) / 100.0;
    } else {
        position_info.Fix = FIX_TYPE_INVALID;
    }

    // Get speed over ground (km/hr)
    char kmhr[6];
    read_len = at_handler->read_string(kmhr, sizeof(kmhr));
    if (read_len > 0) {
        position_info.SpeedOverGround = TinyGPSPlus::parseDecimal(kmhr) / 100.0;
    } else {
        position_info.Fix = FIX_TYPE_INVALID;
    }

    // Skip over speed over ground in knots
    at_handler->skip_param();

    // Get date
    char date[7];
    read_len = at_handler->read_string(date, sizeof(date));
    if (read_len <= 0) {
        position_info.Fix = FIX_TYPE_INVALID;
    }

    // Get number of satellites
    int num_satellites = at_handler->read_int();
    position_info.NumberOfSatellites = num_satellites == -1 ? 0 : num_satellites;

    at_handler->resp_stop();
    at_handler->unlock();

    // Build timestamp
    if (strlen(date) == 6 && strlen(utc_timestamp) == 10) {
        std::string date_string = std::string(date);
        std::string time_string = std::string(utc_timestamp);

        int year = atoi(date_string.substr(4, 2).c_str()) + 2000;
        int month = atoi(date_string.substr(2, 2).c_str());
        int mday = atoi(date_string.substr(0, 2).c_str());
        int hour = atoi(time_string.substr(0, 2).c_str());
        int minute = atoi(time_string.substr(2, 2).c_str());
        int second = (int)std::lround(atof(time_string.substr(4, 2).c_str()));
        position_info.UtcTimestamp = as_unix_time(year,
                                                  month,
                                                  mday,
                                                  hour,
                                                  minute,
                                                  second);
    } else {
        position_info.UtcTimestamp = 0;
    }

    return position_info;    
}

GNSS::GNSSError TELIT_ME910_GNSS::set_gnss_priority(GNSSPriority desired_priority)
{
    at_handler->lock();

    at_handler->at_cmd_discard("$GPSCFG", "=", "%d%d", GPSCFG_SET_WWAN_GNSS_PRIORITY, desired_priority);
    if (at_handler->unlock_return_error() == NSAPI_ERROR_OK) {
        return GNSS_ERROR_OK;
    }

    return GNSS_ERROR_UNKNOWN_ERROR;
}

GNSS::GNSSError TELIT_ME910_GNSS::set_edrx_parameters(ME910eDRXMode mode, ME910eDRXAcT access_technology, char *req_edrx, char *req_pag_time_window)
{
    at_handler->lock();

    at_handler->at_cmd_discard("#CEDRXS", "=", "%d%d%s%s",
        mode, access_technology, req_edrx, req_pag_time_window);
    if (at_handler->unlock_return_error() == NSAPI_ERROR_OK) {
        return GNSS_ERROR_OK;
    }

    return GNSS_ERROR_UNKNOWN_ERROR;
}

GNSS::GNSSError TELIT_ME910_GNSS::configure_gnss_data_stream(ME910NMEAStreamMode mode,
    bool gga, bool gll, bool gsa, bool gsv, bool rmc, bool vtg)
{
    at_handler->lock();

    at_handler->at_cmd_discard("$GPSNMUN", "=", "%d%d%d%d%d%d%d", mode, gga, gll, gsa, gsv, rmc, vtg);
    if (at_handler->unlock_return_error() == NSAPI_ERROR_OK) {
        return GNSS_ERROR_OK;
    }

    return GNSS_ERROR_UNKNOWN_ERROR;
}

time_t TELIT_ME910_GNSS::as_unix_time(int year, int mon, int mday, int hour, int min, int sec)
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

int TELIT_ME910_GNSS::nmea_position_parse(char *s, nmea_position *pos)
{
	char *cursor;

	pos->degrees = 0;
	pos->minutes = 0;

	if (s == NULL || *s == '\0') {
		return -1;
	}

	/* decimal minutes */
	if (NULL == (cursor = strchr(s, '.'))) {
		return -1;
	}

	/* minutes starts 2 digits before dot */
	cursor -= 2;
	pos->minutes = atof(cursor);
	*cursor = '\0';

	/* integer degrees */
	cursor = s;
	pos->degrees = atoi(cursor);

	return 0;
}

nmea_cardinal_t TELIT_ME910_GNSS::nmea_cardinal_direction_parse(char *s)
{
	if (NULL == s || '\0'== *s) {
		return NMEA_CARDINAL_DIR_UNKNOWN;
	}

	switch (*s) {
	case NMEA_CARDINAL_DIR_NORTH:
		return NMEA_CARDINAL_DIR_NORTH;
	case NMEA_CARDINAL_DIR_EAST:
		return NMEA_CARDINAL_DIR_EAST;
	case NMEA_CARDINAL_DIR_SOUTH:
		return NMEA_CARDINAL_DIR_SOUTH;
	case NMEA_CARDINAL_DIR_WEST:
		return NMEA_CARDINAL_DIR_WEST;
	default:
		break;
	}

	return NMEA_CARDINAL_DIR_UNKNOWN;
}

/**
 * Splits a string by space.
 *
 * string is the string to split, will be manipulated. Needs to be
 *        null-terminated.
 * values is a char pointer array that will be filled with pointers to the
 *        splitted values in the string.
 * max_values is the maximum number of values to be parsed.
 *
 * Returns the number of values found in string.
 */
int TELIT_ME910_GNSS::split_string_by_space(char *string, char **values, int max_values)
{
	int i = 0;

	values[i++] = string;
	while (i < max_values && NULL != (string = strchr(string, ' '))) {
		*string = '\0';
		values[i++] = ++string;
	}

	return i;
}

#endif /* TELIT_ME910_GNSS_ENABLED */
