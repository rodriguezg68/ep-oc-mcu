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

/**
 * Made with help from this guide:
 *
 * https://www.digikey.com/en/maker/projects/how-to-measure-temperature-with-an-ntc-thermistor/4a4b326095f144029df7f2eca589ca54
 */

#ifndef EP_OC_MCU_DEVICES_THERMISTORNTC_THERMISTORNTC_H_
#define EP_OC_MCU_DEVICES_THERMISTORNTC_THERMISTORNTC_H_

/** The target must have an ADC to use this thermistor class */
#if DEVICE_ANALOGIN

#include "drivers/AnalogIn.h"

#include "ValueMapping.h"

namespace ep
{

    /**
     * Class representing an NTC thermistor
     *
     * It allows you to read a relative temperature using a resistor divider consisting
     * of a fixed resistor (R_fixed) and a thermistor that's resistance varies with temperature.
     *
     * Example Circuit Diagram:
     *
     *                     ^ VCC
     *                     |
     *                     |   ^
     *                     |  /
     *                 +---+---+
     *                 |    /  |
     *                 | R_NTC |
     *                 |  /    |
     *                 +---+---+
     *                  /  |
     * +-------+           |
     * |ADC_PIN+-----------+
     * +-------+           |
     *                 +---+---+
     *                 |       |
     *                 |R_fixed|
     *                 |       |
     *                 +---+---+
     *                     |
     *                     |
     *                  +-----+ GND
     *                    +-+
     *                     -
     *
     * Note: For this API the NTC MUST be the pull-up resistor in the voltage divider
     *
     * Typically R_fixed is selected to be equal to R_NTC @ room temperature (25.0C)
     *
     * Several resistance-to-temperature conversion strategies are supported, including a
     * calibration lookup table as well as a simpler, but potentially less accurate,
     * direct calculation using beta values from the thermistor's datasheet.
     *
     * Example:
     * @code
     * #include "ThermistorNTC.h"
     * #include "mbed_wait_api.h"
     * #include "ge1923.h"
     *
     * #define NTC_ADC_PIN A0
     *
     * ep::ThermistorNTC ntc(NTC_ADC_PIN, 10000.0f, 3957.0f, 10000.0f);
     *
     * // Alternatively using a calibration table:
     * ep::LinearlyInterpolatedValueMapping ge1923_map(
     *         mbed::make_const_Span(ge1923::calibration_table));
     *
     * ep::ThermistorNTC ntc(NTC_ADC_PIN, 10000.0f, &ge1923_map);
     *
     * int main(void) {
     *     while(true) {
     *         printf("temperature: %.2fC\r\n", ntc.get_temperature());
     *         wait_us(1000000);
     *     }
     * }
     * @endcode
     */
    class ThermistorNTC
    {
    public:

        /**
         * Constructor for temperature conversion using a calibrated lookup
         * table.
         *
         * @param[in] adc_pin Pin to collect analog voltage readings
         * @param[in] r_fixed Fixed resistance in voltage divider sense circuit (ohms)
         * @param[in] map ValueMapping object that provides the resistance (ohms) to temperature (C) table
         *
         * @note See the ValueMapping API documentation for how to create an appropriate
         * ValueMapping object to pass in
         *
         */
        ThermistorNTC(PinName adc_pin, float r_fixed, ValueMapping *map);

        /**
         * Constructor for temperature conversion using a direct calculation
         * from a beta value given in the thermistor's datasheet.
         *
         * @param[in] adc_pin Pin to collect analog voltage readings
         * @param[in] r_fixed Fixed resistance in voltage divider sense circuit (ohms)
         * @param[in] beta Beta value for thermistor given by device's datasheet
         * @param[in] r_room_temp Nominal resistance of NTC thermistor (ohms) @ room temperature (25C)
         *
         * @note This may be less accurate than the lookup table approach but is far
         * easier to implement
         *
         */
        ThermistorNTC(PinName adc_pin, float r_fixed, float beta, float r_room_temp);

        virtual ~ThermistorNTC() { }

        /**
         * Read the current temperature indicated by the NTC thermistor
         *
         * @retval Temperature in degrees celsius
         *
         * @note If the temperature returned is extremely low (eg: -40C) the thermistor may
         * be open circuit!
         *
         * A subclass with a different calculation method may redefine this
         *
         */
        virtual float get_temperature(void);

    protected:

        /**
         * Internal function to read the ADC with configured averaging strategy (see mbed_lib.json)
         *
         * @retval Normalized (0 to 1) adc reading
         */
        float read_adc(void);

    protected:

        mbed::AnalogIn adc_in;

        ValueMapping *r_to_t_map;   /** ValueMapping for resistance to temperature, if given */
        float beta_val;             /** Beta value for thermistor given by datasheet, if given */
        float r_fixed_ohms;         /** Fixed resistance in voltage divider sense circuit in ohms */
        float r_room_temp_ohms;     /** Nominal temperature of thermistor at room temp in ohms, if given */

    };

}

#endif

#endif /* EP_OC_MCU_DEVICES_THERMISTORNTC_THERMISTORNTC_H_ */
