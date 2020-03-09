/**
 * ep-oc-mcu
 * Embedded Planet Open Core for Microcontrollers
 *
 * Built with ARM Mbed-OS
 *
 * Copyright (c) 2019-2020 Embedded Planet, Inc.
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

#include "ThermistorNTC.h"

#include "math.h"

#if DEVICE_ANALOGIN

#define ROOM_TEMP_KELVIN 298.15f

#define KELVIN_TO_CELSIUS(x) x-273.15f;

using namespace ep;

ThermistorNTC::ThermistorNTC(ResistorDivider& r_div, float r_fixed,
        ValueMapping* map, bool ntc_is_pull_up) : r_div(r_div),
        r_to_t_map(map), beta_val(0.0f), r_fixed_ohms(r_fixed),
        r_room_temp_ohms(0.0f), ntc_is_pull_up(ntc_is_pull_up) {
}

ThermistorNTC::ThermistorNTC(ResistorDivider& r_div, float r_fixed, float beta,
        float r_room_temp, bool ntc_is_pull_up) : r_div(r_div),
        r_to_t_map(NULL), beta_val(beta), r_fixed_ohms(r_fixed),
        r_room_temp_ohms(r_room_temp), ntc_is_pull_up(ntc_is_pull_up) {
}

float ThermistorNTC::get_temperature(void) {

    // Get the thermistor's resistance
    float r_thermistor;
    if(ntc_is_pull_up) {
        r_thermistor = r_div.get_R_pu_ohms();
    } else {
        r_thermistor = r_div.get_R_pd_ohms();
    }

    // If a table was given in the constructor, use the lookup table method
    if(r_to_t_map != NULL) {
        return r_to_t_map->lookup(r_thermistor);
    } else {
        /**
         * Otherwise, just calculate using β (beta) equation
         * 1/T = 1/TO + (1/β) ⋅ ln(R/RO)
         *
         * (Beta is generally given to result with temperature in Kelvin)
         */
        float temp_k = (beta_val * ROOM_TEMP_KELVIN)/
                (beta_val + (ROOM_TEMP_KELVIN * logf(r_thermistor/r_room_temp_ohms)));

        return KELVIN_TO_CELSIUS(temp_k);
    }
}


#endif /** DEVICE_ANALOGIN */

