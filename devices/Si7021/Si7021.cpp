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

#include "Si7021.h"

Si7021::Si7021(mbed::I2C& i2c) : i2c(i2c) { }

Si7021::~Si7021()
{
    
}

int32_t Si7021::get_temperature()
{
    return tData;
}

uint32_t Si7021::get_humidity()
{
    return rhData;
}

bool Si7021::measure()
{
    tx_buff[0] = READ_RH;
    if(i2c.write(ADDR, (char*)tx_buff, 1) != 0) return 0;
    if(i2c.read(ADDR, (char*)rx_buff, 2) != 0) return 0;
    
    rhData = ((uint32_t)rx_buff[0] << 8) + (rx_buff[1] & 0xFC);
    rhData = (((rhData) * 15625L) >> 13) - 6000;
    
    tx_buff[0] = READ_TEMP;
    if(i2c.write(ADDR, (char*)tx_buff, 1) != 0) return 0;
    if(i2c.read(ADDR, (char*)rx_buff, 2) != 0) return 0;
    
    tData = ((uint32_t)rx_buff[0] << 8) + (rx_buff[1] & 0xFC);
    tData = (((tData) * 21965L) >> 13) - 46850;
    
    return 1;
}

bool Si7021::check()
{
    tx_buff[0] = READ_ID2_1;
    tx_buff[1] = READ_ID2_2;
    if(i2c.write(ADDR, (char*)tx_buff, 2) != 0) return 0;
    if(i2c.read(ADDR, (char*)rx_buff, 8) != 0) return 0;
    
    if(rx_buff[0] == DEVICE_ID)
        return true;
    else return 0;
}
