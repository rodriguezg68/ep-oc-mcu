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

#ifndef BME680_H
#define BME680_H

#include "bme680_driver.h"
#include "bsec_integration.h"
#include "events/EventQueue.h"
#include "rtos/Thread.h"
#include "drivers/Timer.h"
#include "drivers/I2C.h"

#define BME680_DEFAULT_ADDRESS (0x76 << 1)  // The default I2C address (shifted for MBed 8 bit address)
//#define BME680_DEBUG_MODE  // Use this for enhance debug logs for I2C and more.


/**
 * BME680 Class for I2C usage.
 * Wraps the Bosch library for MBed usage.
 */
class BME680_BSEC {
public:
    // Singleton instance getter
    static BME680_BSEC* get_instance();

    // Initialization function
    bool init(mbed::I2C* i2c);

    /* Data getters and setters */
    float get_temperature();
    void set_temperature(float new_temperature);

    float get_pressure();
    void set_pressure(float new_pressure);

    float get_humidity();
    void set_humidity(float new_humidity);

    float get_gas_resistance();
    void set_gas_resistance(float new_gas_resistance);

    float get_co2_equivalent();
    void set_co2_equivalent(float new_co2_equivalent);

    float get_breath_voc_equivalent();
    void set_breath_voc_equivalent(float new_breath_voc_equivalent);

    float get_iaq_score();
    void set_iaq_score(float new_iaq_score);

    uint8_t get_iaq_accuracy();
    void set_iaq_accuracy(uint8_t new_iaq_accuracy);

    mbed::Timer *get_timer()
    {
        return &timer;
    }

    // BME680 - hardware interface
    static int8_t i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);
    static int8_t i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);
    static void delay_msec(uint32_t ms);
    static int64_t get_timestamp_us();

    // BSEC
    static uint32_t state_load(uint8_t *state_buffer, uint32_t n_buffer);
    static void state_save(const uint8_t *state_buffer, uint32_t length);
    static uint32_t config_load(uint8_t *config_buffer, uint32_t n_buffer);
    static void output_ready(int64_t timestamp, float iaq, uint8_t iaq_accuracy, float temperature, float humidity,
                  float pressure, float raw_temperature, float raw_humidity, float gas,
                  bsec_library_return_t bsec_status, float static_iaq, float co2_equivalent, float breath_voc_equivalent);
    static void bsec_loop_start();

private:
    /* Data members */
    float   temperature;
    float   pressure;
    float   humidity;
    float   gas_resistance;
    float   co2_equivalent;
    float   breath_voc_equivalent;
    float   iaq_score;
    uint8_t iaq_accuracy;

    // Thread for running the BSEC operations
    rtos::Thread bme680_thread;

    // Timer for getting the micro seconds since board start
    mbed::Timer timer;

    static void log(const char *format, ...);

    //static mbed::I2C* bme680_i2c;

    BME680_BSEC(){};
    BME680_BSEC(BME680_BSEC const&){};
    static BME680_BSEC* instance;
};

#endif
