/**
 * ep-oc-mcu
 * Embedded Planet Open Core for Microcontrollers
 *
 * Built with ARM Mbed-OS
 *
 * Copyright (c) 2019 Embedded Planet, Inc.
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

#ifndef SI7021_H
#define SI7021_H

#include "drivers/I2C.h"

/** Si7012 Read Temperature Command */
#define READ_TEMP        0xE0 /* Read previous T data from RH measurement command*/
/** Si7012 Read RH Command */
#define READ_RH          0xE5 /* Perform RH (and T) measurement. */

/** Si7012 Read ID */
#define READ_ID1_1       0xFA
#define READ_ID1_2       0x0F
#define READ_ID2_1       0xFC
#define READ_ID2_2       0xC9

/** Si7012 Read Firmware Revision */
#define READ_FWREV_1     0x84
#define READ_FWREV_2     0xB8

/** I2C device address for Si7021 */
#define ADDR    0x80

/** I2C device frequency for Si7021 */
#define FREQ    100000

/** Device ID value for Si7021 */
#define DEVICE_ID 0x15

class Si7021
{
public:
	Si7021(mbed::I2C& i2c);
    ~Si7021();
    
    /*
     * Get last measured temperature data
     * return: int32_t = temperature in millidegrees centigrade
     */
    int32_t get_temperature();

    /*
     * Get last measured relative humidity data
     * return: uint32_t = relative humidity value in milli-percent
     */
    uint32_t get_humidity();
    
    /*
     * Perform measurement.
     * Asynchronous callback can be provided (type void (*)(void)).
     * return: 0 if successful, else one of the defined error codes.
     */
    bool measure();
    
    /*
     * Check if the sensor is active and responding. This will update the get_active value.
     * Asynchronous callback can be provided (type void (*)(void)).
     * return: 0 if successful, else one of the defined error codes.
     */
    bool check();
    
private:
    mbed::I2C& i2c;
    
    uint8_t  rx_buff[8];
    uint8_t  tx_buff[2];

    uint32_t rhData;
    int32_t  tData;
};

#endif
