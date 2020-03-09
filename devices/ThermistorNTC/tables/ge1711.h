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

#ifndef EP_OC_MCU_DEVICES_THERMISTORNTC_TABLES_GE1711_H_
#define EP_OC_MCU_DEVICES_THERMISTORNTC_TABLES_GE1711_H_

#include "ValueMapping.h"

namespace ge1711 {

const float beta_value = 3977.0f;

/**
 * Operating Temperature: -40C to 180C
 */
const ep::ValueMapping::value_map_entry_t calibration_table[] = {
        { 96.07f,       180.0f },
        { 678.1f,       100.0f },
        { 1070.0f,      85.0f  },
        { 10000.0f,     25.0f  },
        { 32639.0f,     0.0f   },
        { 129925.0f,   -25.0f  },
        { 333562.0f,   -40.0f  },
};

}

#endif /* EP_OC_MCU_DEVICES_THERMISTORNTC_TABLES_GE1711_H_ */
