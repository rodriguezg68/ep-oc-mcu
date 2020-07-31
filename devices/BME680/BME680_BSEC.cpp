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

#if defined(MBED_CONF_RTOS_PRESENT) // For baremetal builds

#include "BME680_BSEC.h"
#include "rtos/Mutex.h"
#include "rtos/ThisThread.h"
#include "platform/Callback.h"
#include "bsec_integration.h"
#include "bsec_interface.h"

BME680_BSEC *BME680_BSEC::instance = NULL;
static mbed::I2C* bme680_i2c;

rtos::Mutex mutex;

BME680_BSEC *BME680_BSEC::get_instance()
{
    if (!instance) {
		instance = new BME680_BSEC;
		bme680_i2c = NULL;
    }

    return instance;
}

bool BME680_BSEC::init(mbed::I2C* i2c)
{
	bme680_i2c = i2c;
    static volatile bool initialized = false;
    mutex.lock();

    // Only do the initialization once
    if (!initialized) {
        timer.start();

        /* Call to the function which initializes the BSEC library
        * Switch on low-power mode and provide no temperature offset */
        log("BME680: Load BME680 / BSEC library \r\n");
        return_values_init ret;
        ret = bsec_iot_init(BSEC_SAMPLE_RATE_LP, 0.0f, i2c_write, i2c_read, delay_msec, state_load, config_load);
        if (ret.bme680_status) {
            /* Could not intialize BME680 */
            printf("BME680: Could not intialize BME680 (%d) \r\n", (int)ret.bme680_status);
            mutex.unlock();
            return false;
        } else if (ret.bsec_status) {
            /* Could not intialize BSEC library */
            printf("BME680: Could not intialize BSEC library (%d) \r\n", (int)ret.bsec_status);
            mutex.unlock();
            return false;
        } else {
            // Start a thread with the BME680/BSEC loop
            log("BME680: BME680 / BSEC library loaded successfully \r\n");
            bme680_thread.start(mbed::callback(&BME680_BSEC::bsec_loop_start));
            initialized = true;
            mutex.unlock();
            return true;
        }
    }
    mutex.unlock();
    return true;
}

/**
 * Reads 8 bit values over I2C
 * @param dev_id Device ID (8 bits I2C address)
 * @param reg_addr Register address to read from
 * @param reg_data Read data buffer
 * @param len Number of bytes to read
 * @return 0 on success, non-zero for failure
 */
int8_t BME680_BSEC::i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{

	if(bme680_i2c == NULL) {
		return 1;
	}

    int8_t result;
    char data[1];

    data[0] = (char) reg_addr;

    log("[0x%X] I2C $%X => ", dev_id >> 1, data[0]);

    result = bme680_i2c->write(dev_id, data, 1);
    log("[W: %d] ", result);

    result = bme680_i2c->read(dev_id, (char *) reg_data, len);

    for (uint8_t i = 0; i < len; i++) {
        log("0x%X ", reg_data[i]);
    }

    log("[R: %d, L: %d] \r\n", result, len);

    return result;
}

/**
 * Writes 8 bit values over I2C
 * @param dev_id Device ID (8 bits I2C address)
 * @param reg_addr Register address to write to
 * @param reg_data Write data buffer
 * @param len Number of bytes to write
 * @return 0 on success, non-zero for failure
 */
int8_t BME680_BSEC::i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
	if(bme680_i2c == NULL) {
		return 1;
	}

    int8_t result;
    char data[len + 1];

    data[0] = (char) reg_addr;

    for (uint8_t i = 1; i < len + 1; i++) {
        data[i] = (char) reg_data[i - 1];
    }

    log("[0x%X] I2C $%X <= ", dev_id >> 1, data[0]);

    result = bme680_i2c->write(dev_id, data, len + 1);

    for (uint8_t i = 1; i < len + 1; i++) {
        log("0x%X ", data[i]);
    }

    log("[W: %d, L: %d] \r\n", result, len);

    return result;
}

void BME680_BSEC::delay_msec(uint32_t ms)
{
    log(" * wait %d ms ... \r\n", ms);
    rtos::ThisThread::sleep_for(ms);
}

void BME680_BSEC::log(const char *format, ...)
{
#ifdef BME680_DEBUG_MODE
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
#endif
}

/*!
 * @brief           Capture the system time in microseconds
 * @return          system_current_time    current system timestamp in microseconds
 */
int64_t BME680_BSEC::get_timestamp_us()
{
    BME680_BSEC *current_instance = BME680_BSEC::get_instance();
    int64_t system_current_time = current_instance->get_timer()->read_high_resolution_us();
    return (int64_t)(system_current_time);
}

/*!
 * @brief           Load previous library state from non-volatile memory
 * @param[in,out]   state_buffer    buffer to hold the loaded state string
 * @param[in]       n_buffer        size of the allocated state buffer
 * @return          number of bytes copied to state_buffer
 */
uint32_t BME680_BSEC::state_load(uint8_t *state_buffer, uint32_t n_buffer)
{
    // Load a previous library state from non-volatile memory, if available.
    //
    // Return zero if loading was unsuccessful or no state was available,
    // otherwise return length of loaded state string.
    //
    // Not implemented...
    return 0;
}

/*!
 * @brief           Save library state to non-volatile memory
 * @param[in]       state_buffer    buffer holding the state to be stored
 * @param[in]       length          length of the state string to be stored
 * @return          none
 */
void BME680_BSEC::state_save(const uint8_t *state_buffer, uint32_t length)
{
    // Save the string some form of non-volatile memory, if possible.
    //
    // Not implemented...
}

/*!
 * @brief           Load library config from non-volatile memory
 * @param[in,out]   config_buffer    buffer to hold the loaded state string
 * @param[in]       n_buffer        size of the allocated state buffer
 * @return          number of bytes copied to config_buffer
 */
uint32_t BME680_BSEC::config_load(uint8_t *config_buffer, uint32_t n_buffer)
{
    // Load a library config from non-volatile memory, if available.
    //
    // Return zero if loading was unsuccessful or no config was available,
    // otherwise return length of loaded config string.
    //
    // Not implemented...
    return 0;
}

/* This function is the repeating call from the BME680 sensor and BSEC library to fill the sensor values into the system variables and calculate the IAQ score
 * @brief           Handling of the ready outputs
 * @param[in]       timestamp       time in nanoseconds
 * @param[in]       iaq             IAQ signal
 * @param[in]       iaq_accuracy    accuracy of IAQ signal
 * @param[in]       temperature     temperature signal
 * @param[in]       humidity        humidity signal
 * @param[in]       pressure        pressure signal
 * @param[in]       raw_temperature raw temperature signal
 * @param[in]       raw_humidity    raw humidity signal
 * @param[in]       raw_gas         raw gas sensor signal
 * @param[in]       bsec_status     value returned by the bsec_do_steps() call
 * @return          none
 */
void BME680_BSEC::output_ready(int64_t timestamp, float iaq, uint8_t iaq_accuracy, float temperature, float humidity,
                          float pressure, float raw_temperature, float raw_humidity, float gas,
                          bsec_library_return_t bsec_status, float static_iaq, float co2_equivalent, float breath_voc_equivalent)
{
    BME680_BSEC *current_instance = BME680_BSEC::get_instance();

    // Set the system variables
    current_instance->set_temperature(temperature);
    current_instance->set_pressure(pressure);
    current_instance->set_humidity(humidity);
    current_instance->set_gas_resistance(gas);
    current_instance->set_co2_equivalent(co2_equivalent);
    current_instance->set_breath_voc_equivalent(breath_voc_equivalent);
    current_instance->set_iaq_score(iaq);
    current_instance->set_iaq_accuracy(iaq_accuracy);

#ifdef BME680_DEBUG_MODE
    // Define the IAQ index
    char *bme680_iaq_rating_txt = "";
    if (iaq_accuracy == 0) {
        bme680_iaq_rating_txt = "???";
    } else if (current_instance->get_iaq_score() >= 300.00f) {
        bme680_iaq_rating_txt = "Hazardous";
    } else if (current_instance->get_iaq_score() >= 200.00f) {
        bme680_iaq_rating_txt = "Very Unhealthy";
    } else if (current_instance->get_iaq_score() >= 150.00f) {
        bme680_iaq_rating_txt = "Unhealthy";
    } else if (current_instance->get_iaq_score() >= 100.00f) {
        bme680_iaq_rating_txt = "Little bad";
    } else if (current_instance->get_iaq_score() >=  50.00f) {
        bme680_iaq_rating_txt = "Average";
    } else if (current_instance->get_iaq_score() >=   0.00f) {
        bme680_iaq_rating_txt = "Good";
    } else {
        bme680_iaq_rating_txt = "???";
    }

    // Print the results
    log("BME680 sensor data: Temperature = %.2f *C | Pressure = %.2f hPa | Humidity = %.2f %% | Gas = %.2f KOhms \r\n", current_instance->get_temperature(), current_instance->get_pressure() / 100.00f, current_instance->get_humidity(), current_instance->get_gas_resistance() / 1000.00f);
    log("BME680 air quality: Score = %.2f | Accuracy = %d | Rating = %s \r\n", current_instance->get_iaq_score(), current_instance->get_iaq_accuracy(), bme680_iaq_rating_txt);
#endif
}

/*!
 * @brief       Call to endless loop BSEC function which reads and processes data based on sensor settings
 * @return          none
 */
void BME680_BSEC::bsec_loop_start()
{
    /* Call to endless loop function which reads and processes data based on sensor settings */
    /* State is saved every 10.000 samples, which means every 10.000 * 3 secs = 500 minutes  */
    bsec_iot_loop(delay_msec, get_timestamp_us, output_ready, state_save, 10000);
}

/* This function gets the current temperature value in degrees Celsius
 * @brief           Gets the current temperature value
 * @return          The current temperature value
 */
float BME680_BSEC::get_temperature()
{
    return temperature;
}

/* This function sets the current temperature value in degrees Celsius
 * @brief           Sets the current temperature value
 * @param[in]       new_temperature     new temperature value
 * @return          none
 */
void BME680_BSEC::set_temperature(float new_temperature)
{
    temperature = new_temperature;
}

/* This function gets the current pressure value in Pascals
 * @brief           Gets the current pressure value
 * @return          The current pressure value
 */
float BME680_BSEC::get_pressure()
{
    return pressure;
}

/* This function sets the current pressure value in Pascals
 * @brief           Sets the current pressure value
 * @param[in]       new_pressure     new pressure value
 * @return          none
 */
void BME680_BSEC::set_pressure(float new_pressure)
{
    pressure = new_pressure;
}

/* This function gets the current relative humidity value as a percentage
 * @brief           Gets the current relative humidity value
 * @return          The current humidity value
 */
float BME680_BSEC::get_humidity()
{
    return humidity;
}

/* This function sets the current relative humidity value as a percentage
 * @brief           Sets the current relative humidity value
 * @param[in]       new_humidity     new humidity value
 * @return          none
 */
void BME680_BSEC::set_humidity(float new_humidity)
{
    humidity = new_humidity;
}

/* This function gets the current gas resistance value in Ohms
 * @brief           Gets the current gas resistance value
 * @return          The current gas resistance value
 */
float BME680_BSEC::get_gas_resistance()
{
    return gas_resistance;
}

/* This function sets the current gas resistance value in Ohms
 * @brief           Sets the current gas resistance value
 * @param[in]       new_gas_resistance     new gas resistance value
 * @return          none
 */
void BME680_BSEC::set_gas_resistance(float new_gas_resistance)
{
    gas_resistance = new_gas_resistance;
}

/* This function gets the current CO2 equivalents value in ppm
 * @brief           Gets the current CO2 equivalents value
 * @return          The current CO2 equivalents value
 */
float BME680_BSEC::get_co2_equivalent()
{
    return co2_equivalent;
}

/* This function sets the current CO2 equivalents value in ppm
 * @brief           Sets the current CO2 equivalents value
 * @param[in]       new_co2_equivalent     new CO2 equivalents value
 * @return          none
 */
void BME680_BSEC::set_co2_equivalent(float new_co2_equivalent)
{
    co2_equivalent = new_co2_equivalent;
}

/* This function gets the current b-VOC equivalents value in ppm
 * @brief           Gets the current b-VOC equivalents value
 * @return          The current b-VOC equivalents value
 */
float BME680_BSEC::get_breath_voc_equivalent()
{
    return breath_voc_equivalent;
}

/* This function sets the current b-VOC equivalents value in ppm
 * @brief           Sets the current b-VOC equivalents value
 * @param[in]       new_breath_voc_equivalent     new b-VOC equivalents value
 * @return          none
 */
void BME680_BSEC::set_breath_voc_equivalent(float new_breath_voc_equivalent)
{
    breath_voc_equivalent = new_breath_voc_equivalent;
}

/* This function gets the current IAQ score value
 * @brief           Gets the current IAQ score value
 * @return          The current IAQ score value
 */
float BME680_BSEC::get_iaq_score()
{
    return iaq_score;
}

/* This function sets the current IAQ score value
 * @brief           Sets the current IAQ score value
 * @param[in]       new_iaq_score     new IAQ score value
 * @return          none
 */
void BME680_BSEC::set_iaq_score(float new_iaq_score)
{
    iaq_score = new_iaq_score;
}

/* This function gets the current IAQ accuracy value
 * @brief           Gets the current IAQ accuracy value
 * @return          The current IAQ accuracy value
 */
uint8_t BME680_BSEC::get_iaq_accuracy()
{
    return iaq_accuracy;
}

/* This function sets the current IAQ accuracy value
 * @brief           Sets the current IAQ accuracy value
 * @param[in]       new_iaq_accuracy     new IAQ accuracy value
 * @return          none
 */
void BME680_BSEC::set_iaq_accuracy(uint8_t new_iaq_accuracy)
{
    iaq_accuracy = new_iaq_accuracy;
}

#endif //defined(MBED_CONF_RTOS_PRESENT)
