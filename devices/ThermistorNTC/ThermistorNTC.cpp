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


ep::ThermistorNTC::ThermistorNTC(PinName adc_pin, float r_fixed, ValueMapping* map) : adc_in(adc_pin),
        r_to_t_map(map), beta_val(0.0f), r_fixed_ohms(r_fixed), r_room_temp_ohms(0.0f) {
}

ep::ThermistorNTC::ThermistorNTC(PinName adc_pin, float r_fixed, float beta, float r_room_temp) : adc_in(adc_pin),
        r_to_t_map(NULL), beta_val(beta), r_fixed_ohms(r_fixed), r_room_temp_ohms(r_room_temp) {
}

float ep::ThermistorNTC::read_adc(void) {

    /** Sample the adc to get the normalized (0 to 1) voltage */
    float adc_sample = 1.0f;

    /** If averaging is enabled, take a few samples (might block for a little longer)*/
#if MBED_CONF_THERMISTOR_NTC_AVERAGED_SAMPLES
    for(int i = 0; i < MBED_CONF_THERMISTOR_NTC_AVERAGED_SAMPLES; i++) {
        adc_sample += adc_in.read();
    }

    adc_sample /= MBED_CONF_THERMISTOR_NTC_AVERAGED_SAMPLES;
#else
    adc_sample = adc_in.read();
#endif

    return adc_sample;
}

float ep::ThermistorNTC::get_temperature(void) {

    float adc_sample = this->read_adc();

    // Make sure we don't divide by zero (NTC is open circuit!)
    if(adc_sample == 0.0f) {
        adc_sample = 0.000001f;
    }

    /**
     * Calculate the thermistor's resistance in ohms
     * R_thermistor = R_fixed * ((1/adc_sample)-1)
     */
    float r_thermistor = (r_fixed_ohms*((1.0f/adc_sample)-1));

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

