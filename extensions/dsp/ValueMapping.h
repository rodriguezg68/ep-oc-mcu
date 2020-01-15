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
#ifndef EP_OC_MCU_VALUEMAPPING_H_
#define EP_OC_MCU_VALUEMAPPING_H_

// Note: Does NOT require CMSIS DSP library

#include "platform/Span.h"

namespace ep
{
    /**
     * Abstract class that maps values in one domain to values
     * in another domain. eg: ADC counts to battery level remaining
     *
     * Subclasses may redefine how "in between" values are treated. This allows
     * different use cases to use the correct interpolation for their problem.
     *
     * @note: If the x-values in your data are evenly spaced, you can use a faster
     * implementation available. See "FastValueMapping.h" for more information
     */
    class ValueMapping {

    public:

        typedef struct value_map_entry_t {
            float x;
            float y;
        } value_map_entry_t;

    public:

        /**
         * Initialize a value mapping instance
         * @param[in] value_map Table of x and y values
         */
        ValueMapping(mbed::Span<value_map_entry_t> value_map) : table(value_map) { }

        virtual ~ValueMapping() {
        }

        /**
         * Get the corresponding value to the input x
         * @param[in] x Input X value
         *
         * @retval y_value Interpolated output Y value based on table
         */
        virtual float get_value(float x) = 0;

    protected:

        mbed::Span<value_map_entry_t> table;

    };

    /**
     * Linear Interpolation Value Mapping
     *
     */
    class LinearlyInterpolatedValueMapping : public ValueMapping {

    public:

        /**
         * Initialize a value mapping instance
         * @param[in] value_map Table of x and y values
         */
        LinearlyInterpolatedValueMapping(mbed::Span<value_map_entry_t> value_map) :
            ValueMapping(value_map) {
        }

        virtual ~LinearlyInterpolatedValueMapping() {
        }

        /**
         * Get the corresponding value to the input x
         * @param[in] x Input X value
         *
         * @retval y_value Interpolated output Y value based on table
         */
        virtual float get_value(float x) {

            // Below the range of the table
            if(x <= table[0].x) {
                return table[0].y;
            }

            // Above the range of the table
            if(x >= table[table.size()].x) {
                return table[table.size()].y;
            }

            // Find the closest values to the input x value
            int x0_index, x1_index;
            for(int i = 0; i < table.size(); i++) {
                if(table[i].x <= x && x <= table[i+1].x) {
                    x0_index = i;
                    x1_index = i+1;
                    break;
                }
            }

            float x0, x1, y0, y1;
            x0 = table[x0_index].x;
            y0 = table[x0_index].y;

            x1 = table[x1_index].x;
            y1 = table[x1_index].y;

            // Return the linear interpolation between these two values based on the given x
            return (y0 + ((x-x0) * ((y1-y0)/(x1-x0))));

        }

    };
}


#endif /* DES0569_BSP_EP_CORE_EP_OC_MCU_EXTENSIONS_VALUEMAPPING_H_ */
