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

#ifndef EP_OC_MCU_EXTENSIONS_TELIT_ME310_GNSS_H_
#define EP_OC_MCU_EXTENSIONS_TELIT_ME310_GNSS_H_

// Only compile this if the config is enabled
#if TELIT_ME310_GNSS_ENABLED

#include "GNSS.h"
#include "ATHandler.h"
#include "CellularDevice.h"
#include "mbed_trace.h"

namespace ep
{

static const int GPSCFG_SET_WWAN_GNSS_PRIORITY  = 0;
static const int GPSCFG_SET_TBF                 = 1;
static const int GPSCFG_SET_CONSTELLATION       = 2;

/**
 * Logical abstraction of the Telit ME310 GNSS controller
 */
class TELIT_ME310_GNSS : public GNSS  {
public:
    /**
     * GNSS priority
     */
    enum GNSSPriority {
        PRIORITY_GNSS = 0,
        PRIORITY_WWAN = 1
    };

    /**
     * Default constructor
     */
    TELIT_ME310_GNSS();

    /**
     * Initialize the connection with the GNSS controller. This
     * function should be called before enable/disable.
     */
    void init();

    /**
     * Enable the Telit ME310 GNSS controller
     * 
     * @return success
     */
    GNSSError enable();

    /**
     * Disable the Telit ME310 GNSS controller
     * 
     * @return success
     */
    GNSSError disable();

    /**
     * Retrieve the current position
     * 
     * @return The current position info
     */
    PositionInfo get_current_position();

    /**
     * Set GNSS controller priority (GNSS or WWAN)
     * 
     * @param desired_priority  Priority setting desired
     * @return success
     */
    GNSSError set_gnss_priority(GNSSPriority desired_priority);

private:
    CellularDevice *dev;
    ATHandler *at_handler;

    /**
     * Convert year, month, day, hour, min, sec to unix timestamp
     * 
     * @param year  Year
     * @param mon   Month
     * @param mday  Day of the month
     * @param hour  Hours
     * @param min   Minutes
     * @param sec   Seconds
     * @return Unix timestamp
     */
    time_t as_unix_time(int year, int mon, int mday, int hour, int min, int sec);
};

}

#endif /* TELIT_ME310_GNSS_ENABLED */

#endif /* EP_OC_MCU_EXTENSIONS_TELIT_ME310_GNSS_H_ */