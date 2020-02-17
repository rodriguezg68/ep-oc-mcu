/*
 * ge1923.h
 *
 *  Created on: Feb 17, 2020
 *      Author: gdbeckstein
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
