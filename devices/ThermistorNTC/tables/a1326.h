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

#ifndef EP_OC_MCU_DEVICES_THERMISTORNTC_TABLES_A1326_H_
#define EP_OC_MCU_DEVICES_THERMISTORNTC_TABLES_A1326_H_

#include "ValueMapping.h"

namespace a1326 {

//const float beta_value = 3977.0f;

/**
 * Operating Temperature: -40C to 150C
 */
const ep::ValueMapping::value_map_entry_t calibration_table[] = {
        { 46.74f,       150.0f },
        { 86.96f,       125.0f },
        { 175.3f,       100.0f },
        { 279.1f,       85.0f  },
        { 2752.0f,      25.0f  },
        { 4373.0f,      15.0f  },
        { 9255.0f,      0.0f   },
        { 37991.0f,     -25.0f },
        { 99318.0f,     -40.0f },
};

}

#endif /* EP_OC_MCU_DEVICES_THERMISTORNTC_TABLES_A1326_H_ */
