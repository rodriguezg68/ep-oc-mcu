/**
 * BME680.h
 *
 * BME680 driver written for use with Mbed
 * (C) Embedded Planet, Inc
 */

#ifndef BME680_H
#define BME680_H

#include "bme680.h"
#include "drivers/I2C.h"

#define BME680_DEFAULT_ADDRESS (0x76 << 1)  // The default I2C address (shifted for MBed 8 bit address)

/**
 * BME680 Class for I2C usage.
 * Wraps the Bosch library for Mbed usage.
 */
class BME680 {
public:
    BME680(mbed::I2C* i2c, uint8_t addr = BME680_DEFAULT_ADDRESS);

    bool begin();

    bool setTemperatureOversampling(uint8_t os);

    bool setPressureOversampling(uint8_t os);

    bool setHumidityOversampling(uint8_t os);

    bool setIIRFilterSize(uint8_t fs);

    bool setGasHeater(uint16_t heaterTemp, uint16_t heaterTime);

    bool performReading();

    bool isGasHeatingSetupStable();

    int16_t getRawTemperature();
    uint32_t getRawPressure();
    uint32_t getRawHumidity();
    uint32_t getRawGasResistance();

    float getTemperature();

    float getPressure();

    float getHumidity();

    float getGasResistance();

private:
    bool _filterEnabled, _tempEnabled, _humEnabled, _presEnabled, _gasEnabled;
    struct bme680_dev gas_sensor;
    struct bme680_field_data data;
    uint8_t _adr;

    // BME680 - hardware interface
    static int8_t i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);

    static int8_t i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);

    static void delay_msec(uint32_t ms);
};

#endif
