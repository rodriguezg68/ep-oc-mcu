/*
 * MAX44009 Ambient Light Sensor with ADC library
 *
 *
 * Copyright (c) 2013 Davy Van Belle, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
 * and associated documentation files (the "Software"), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, 
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or 
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/** @file
 * @brief MAX44009 I2C
 */

#ifndef MAX44009_H
#define MAX44009_H

#include "drivers/I2C.h"

#define INT_STATUS 0x00
#define INT_ENABLE 0x01

#define MAX44009_CONFIG 0x02

#define LUX_HIGH_B 0x03
#define LUX_LOW_B  0x04

#define UP_THRESH_HIGH_B 0x05
#define LOW_THRESH_HIGH_B 0x06

#define THRESH_TIMER 0x07

#define MAX44009_I2C_ADDR_0 0x94 /** I2C Address option 0 (A0 Pin connected to GND) */
#define MAX44009_I2C_ADDR_1 0x96 /** I2C Address option 1 (A0 Pin connected to VCC) */


/** MAX44009 class 
 */
class MAX44009 {
public:
    MAX44009(mbed::I2C &i2c, char addr = MAX44009_I2C_ADDR_0);

    void setConfig(char config);

    char getIntStatus();

    void setIntEnable(bool Enable);

    void getRawReading(char buff[2]);

    double getLUXReading();

    double getLuxFromBuffReading(char *buff);

    void setUpperThreshold(char threshold);

    void setLowerThreshold(char threshold);

    void setThresholdTimer(char time);

private:
    mbed::I2C& _i2c;
    char _addr;

};

#endif    
