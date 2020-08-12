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

#ifndef EP_OC_MCU_DEVICES_RESISTORDIVIDER_RESISTORDIVIDER_H_
#define EP_OC_MCU_DEVICES_RESISTORDIVIDER_RESISTORDIVIDER_H_

#include "platform/mbed_version.h"

/** The target must have an ADC to use this resistor divider class */
#if DEVICE_ANALOGIN

/**
 * The adc reference voltage configuration was not introduced until 
 * Mbed-OS 6, so we must retarget this macro to maintain compatibility
 * with Mbed-OS 5.
 *
 * This setting sets the voltage used to scale an ADC reading
 * Defaults to 3.3V
 * May be reconfigured by adding a the following to your mbed_app.json:
 * 
 * "config": {
 *  "default-adc-vref": {
 *   "value": 5.0f
 *  }
 * }
 *
 */
 #if MBED_MAJOR_VERSION == 5
    #ifndef MBED_CONF_TARGET_DEFAULT_ADC_VREF
        #ifndef MBED_CONF_APP_DEFAULT_ADC_VREF
            #define MBED_CONF_TARGET_DEFAULT_ADC_VREF 3.3f
        #else
            #define MBED_CONF_TARGET_DEFAULT_ADC_VREF MBED_CONF_APP_DEFAULT_ADC_VREF 
        #endif
    #endif 
 #endif

#include "drivers/AnalogIn.h"

namespace ep
{

    class ResistorDivider {

    public:

        static constexpr float UnknownVal = -1.0f;

    public:

        /**
         * Create a resistor divider input using the given AnalogIn object
         *
         * A resistor divider is a basic circuit that divides an input voltage, Vin,
         * down to an output voltage, Vout, using two resistors, Rpu and Rpd (pull-up/pull-down)
         *
         * Example circuit diagram:
         *
         *
         * Vin <----+
         *           |
         *         +-+-+
         *         |   |
         *         |   | Rpu
         *         +-+-+
         *           |
         *           |+------> Vout
         *           |
         *         +-+-+
         *         |   |
         *         |   | Rpd
         *         |   |
         *         +-+-+
         *           |
         *           |
         *         +---+
         *          +-+
         *           +
         *
         *
         * The mathematical relationship is described by the equation below:
         *
         * Vout = Rpd/(Rpu + Rpd) * Vin
         *
         * Given two other variables in this equation it is possible to
         * determine the third by measuring Vout with an analog-to-digital convert (ADC)
         *
         * @param[in] adc_in AnalogIn object to take voltage measurements of Vout with
         * @param[in] r_pd Pull-down resistor's known resistance (in ohms, <= 0.0f if unknown)
         * @param[in] r_pu Pull-up resistior's known resistance (in ohms, <= 0.0f if unknown)
         * @param[in] vin Input voltage to the divider circuit (in volts, <= 0.0f if unknown)
         *
         * @note vin defaults to the reference voltage used by the `AnalogIn` object
         *
         * @note Exactly 2 of the given parameters MUST be non-zero (known/fixed) or the application
         * will assert at runtime!
         *
         */
        ResistorDivider(mbed::AnalogIn& adc_in, float r_pd,
                float r_pu = UnknownVal, float vin_volts = MBED_CONF_TARGET_DEFAULT_ADC_VREF);

        /**
         * Returns the known or calculated resistance
         * of the pull-up resistor in the divider circuit (in ohms)
         *
         * @returns pull-up resistor's value in ohms
         */
        float get_R_pu_ohms(void);

        /**
         * Returns the known or calculated resistance
         * of the pull-down resistor in the divider circuit (in ohms)
         *
         * @returns pull-down resistor's value in ohms
         */
        float get_R_pd_ohms(void);

        /**
         * Returns the known or calculated voltage
         * of V_in in the divider circuit (in volts)
         *
         * @returns V_in voltage of divider circuit
         */
        float get_Vin_volts(void);

    protected:

        mbed::AnalogIn& adc_in; /** AnalogIn object used to take measurements of Vout with */

        float r_pu;         /** Given value of Rpu in the divider circuit (0.0f if unknown) */
        float r_pd;         /** Given value of Rpd in the divider circuit (0.0f if unknown) */
        float vin_volts;    /** Given value of Vin in the divider circuit (0.0f if unknown) */

    };
}

#endif /** DEVICE_ANALOGIN */


#endif /* EP_OC_MCU_DEVICES_RESISTORDIVIDER_RESISTORDIVIDER_H_ */
