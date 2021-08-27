/*
 * Mbed-OS Microcontroller Library
 * Copyright (c) 2021 Embedded Planet
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
 * limitations under the License
 */

#ifndef _EP_OC_MCU_EXTENSIONS_GNSS_GNSS_UTILS_H_
#define _EP_OC_MCU_EXTENSIONS_GNSS_GNSS_UTILS_H_

#include "nmea.h"

/* Converts a cardinal nmea position to a single float value */
float nmea_position_to_float(nmea_position pos) {
    float sign = 1.0f;
    if(pos.cardinal == 'S' || pos.cardinal == 'W') {
        sign = -1.0f;
    }
    return (((float) pos.degrees + (pos.minutes/60.0f)) * sign);
}



#endif /* _EP_OC_MCU_EXTENSIONS_GNSS_GNSS_UTILS_H_ */
