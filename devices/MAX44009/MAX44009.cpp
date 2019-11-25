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

#include "MAX44009.h"
#include <math.h>

/** init MAX44009 class
 * @param *i2c pointer to I2C serial interface
 * @param addr sensor I2C address
 */
MAX44009::MAX44009(mbed::I2C &i2c, char addr) : _i2c(i2c), _addr(addr) {
}

/**
 * Set configuration register for the device
 * @param config desired configuration register bits
 * BIT 7 - CONT: 1 = continuous mode, 0 = single measurement
 * BIT 6 - MANUAL: 1 = CDR, TIM[2:0] set by user, 0 = CDR, TIM[2:0] set by internal autorange
 * BIT [5:4] - Not Used
 * BIT 3 - CDR: 1 = Current divided by 8. (High-brightness), 0 = Current not divided.
 * BIT [2:0] - TIM: Integration Time. See datasheet.
 */
void MAX44009::setConfig(char config) {
    char cmd[2];
    cmd[0] = MAX44009_CONFIG;
    cmd[1] = config;
    _i2c.write(_addr, cmd, 2);
}

/**
 * Get device INT_STATUS register
 * BIT 0 : 0 = No interrupt event occurred, 1 = Ambient light intensity is outside the threshold range.
 */
char MAX44009::getIntStatus() {
    char status;
    char cmd = INT_STATUS;
    _i2c.write(_addr, &cmd, 1, true);
    _i2c.read(_addr + 1, &status, 1);
    return status;
}

/**
 * Set device INT_ENABLE register
 * @param Enable  BIT 0 : 0 = INT pin and INTS bit not effected if an interrupt event occurred, 1 = INT pin pulled low and INTS bit is set if interrupt occurred. /
 */
void MAX44009::setIntEnable(bool Enable) {
    char cmd[2];
    cmd[0] = INT_ENABLE;
    cmd[1] = (char) 0x00 | Enable;
    _i2c.write(_addr, cmd, 2);
}

/**
 * Get raw reading over I2C
 * @param buff raw reading buffer
 */
void MAX44009::getRawReading(char buff[2]) {
    char cmd[2];

    cmd[0] = LUX_HIGH_B;
    cmd[1] = LUX_LOW_B;

    _i2c.write(_addr, &cmd[0], 1, true);
    _i2c.read(_addr + 1, &buff[0], 1);
    _i2c.write(_addr, &cmd[1], 1, true);
    _i2c.read(_addr + 1, &buff[1], 1);
}

/**
 * Get LUX reading from ADC
 */
double MAX44009::getLUXReading() {
    char buff[2];

    this->getRawReading(buff);
    return this->getLuxFromBuffReading(buff);
}

/**
 * Get the computed lux value for given buff reading
 */
double MAX44009::getLuxFromBuffReading(char *buff) {
    char exponent, mantissa;
    double lux;

    exponent = (char) (buff[0] & 0xF0) >> 4;
    mantissa = (char) (((buff[0] & 0x0F) << 4) | (buff[1] & 0x0F));

    lux = pow((double) 2, (double) exponent) * mantissa * 0.045;

    return lux;
}

/**
 * Set upper threshold
* @param threshold set upper threshold value. Further info, see datasheet
*/
void MAX44009::setUpperThreshold(char threshold) {
    char cmd[2];
    cmd[0] = UP_THRESH_HIGH_B;
    cmd[1] = threshold;
    _i2c.write(_addr, cmd, 2);
}

/**
 * Set lower threshold
* @param threshold set lower threshold value. Further info, see datasheet
*/
void MAX44009::setLowerThreshold(char threshold) {
    char cmd[2];
    cmd[0] = LOW_THRESH_HIGH_B;
    cmd[1] = threshold;
    _i2c.write(_addr, cmd, 2);
}

/**
 * Set Threshold time
* @param time set time to trigger interrupt if value is below or above threshold value. Further info, see datasheet
*/
void MAX44009::setThresholdTimer(char time) {
    char cmd[2];
    cmd[0] = THRESH_TIMER;
    cmd[1] = time;
    _i2c.write(_addr, cmd, 2);
}
