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

/** Note: Requires CMSIS DSP library, see README.md */

#include "arm_math.h"

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
     * @note: The reason this is a "fast" value mapping is that the x-values
     * are evenly spaced, allowing the index of the y-value to be determined in a single operation.
     *
     * If x-values are not evenly spaced, the algorithm must first search the table and find
     * the closest x values to the input x value. See "ValueMapping.h" for that kind of implementation
     */
    class FastValueMapping {

    public:

        /**
         * Initialize a value mapping instance
         * @param[in] initial_x First x value of data in the table
         * @param[in] x_spacing Spacing of X values for table
         * @param[in] y_table Table of y values
         *
         * @note The y_values table should be aligned such that the first
         * value in the y_table is the expected output for initial_x, the second
         * value in the y_table is the expected output for initial_x + x_spacing,
         * and so on.
         */
        FastValueMapping(float initial_x, float x_spacing, mbed::Span y_table) :
        x0(initial_x), delta_x(x_spacing), table(y_table) { }

        virtual ~FastValueMapping() {
        }

        /**
         * Get the corresponding value to the input x
         * @param[in] x Input X value
         *
         * @retval y_value Interpolated output Y value based on table
         */
        virtual float get_value(float x) = 0;

    protected:

        float x0;
        float delta_x;
        mbed::Span table;

    };

    /**
     * Linear Interpolation Value Mapping
     *
     */
    class FastLinearlyInterpolatedValueMapping : public FastValueMapping {

    public:

        /**
         * Initialize a value mapping instance
         * @param[in] initial_x First x value of data in the table
         * @param[in] x_spacing Spacing of X values for table
         * @param[in] y_table Table of y values
         *
         * @note The y_values table should be aligned such that the first
         * value in the y_table is the expected output for initial_x, the second
         * value in the y_table is the expected output for initial_x + x_spacing,
         * and so on.
         */
        FastLinearlyInterpolatedValueMapping(float initial_x, float x_spacing, mbed::Span y_table) :
            FastValueMapping(initial_x, x_spacing, y_table) {
            // Fill out the instance information
            instance.x1 = initial_x;
            instance.xSpacing = x_spacing;
            instance.nValues = y_table.size();
            instance.pYData = y_table.data();
        }

        virtual ~FastLinearlyInterpolatedValueMapping() {
        }

        /**
         * Get the corresponding value to the input x
         * @param[in] x Input X value
         *
         * @retval y_value Interpolated output Y value based on table
         */
        virtual float get_value(float x) {
            return arm_linear_interp_f32(&instance, x);
        }


    protected:

        arm_linear_interp_instance_f32 instance;


    };
}


#endif /* DES0569_BSP_EP_CORE_EP_OC_MCU_EXTENSIONS_VALUEMAPPING_H_ */
