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

#ifndef EP_OC_MCU_DEVICES_THERMISTORNTC_TABLES_GE1923_H_
#define EP_OC_MCU_DEVICES_THERMISTORNTC_TABLES_GE1923_H_

#include "ValueMapping.h"

namespace ge1923 {

const float beta_value = 3957.0f;

/**
 * Operating Temperature: -30C to 80C
 * Temperature accuracy: +-0.34 @ 25C
 */
const ep::ValueMapping::value_map_entry_t calibration_table[] = {
        { 1071.0f,      85.0f },
        { 1257.0f,      80.0f },
        { 1482.0f,      75.0f },
        { 1754.0f,      70.0f },
        { 2085.0f,      65.0f },
        { 2490.0f,      60.0f },
        { 2989.0f,      55.0f },
        { 3606.0f,      50.0f },
        { 4373.0f,      45.0f },
        { 5331.0f,      40.0f },
        { 6536.0f,      35.0f },
        { 8060.0f,      30.0f },
        { 10000.0f,     25.0f },
        { 12486.0f,     20.0f },
        { 15695.0f,     15.0f },
        { 19869.0f,     10.0f },
        { 25338.0f,     5.0f  },
        { 32566.0f,     -0.0f },
        { 42193.0f,     -5.0f },
        { 55109.0f,     -10.0f },
        { 72592.0f,     -15.0f },
        { 96481.0f,     -20.0f },
        { 129449.0f,    -25.0f },
        { 175427.0f,    -30.0f },
        { 240264.0f,    -35.0f },
};

}

#endif /* EP_OC_MCU_DEVICES_THERMISTORNTC_TABLES_GE1923_H_ */
