/*
 * Built with ARM Mbed-OS
 *
 * Copyright (c) 2018-2020 George Beckstein
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
 *  Created on: Sep 22, 2018
 *      Author: gdbeckstein
 */

#include "EasyScale.h"
#include "platform/mbed_wait_api.h"

/**
 * EasyScale max frequency: 100kHz
 * (Logic 0: t_low >= 2 * t_high)
 * (Logic 1: t_high >= 2 * t_low)
 *
 * These delays set it to ~83kHz
 */
#define EASYSCALE_LONG_DELAY_US 40
#define EASYSCALE_SHORT_DELAY_US 10

uint8_t EasyScale::DEVICE_ADDRESS_TPS61158 = 0x58;

EasyScale::EasyScale(PinName ctrl_pin) :
							es_ctrl_pin(ctrl_pin, PIN_OUTPUT, PullNone, 0)
{
}

void EasyScale::power_on(void)
{

	/**
	 * 1. Pull CTRL pin high to enable the TPS61158 and to start
	 * 	the 1-wire detection window.
	 *
	 * 2. After the EasyScale detect delay (tes_delay, 100us) expires, drive CTRL
	 * 	low for more than the EasyScale detection time (tes_detect, 450 us)
	 *
	 * 3. The CTRL pin has to be low for more than EasyScale detection time
	 * 	before the EasyScale detection window (tes_win, 3.5ms) expires.
	 * 	EasyScale detection window starts from the first
	 * 	CTRL pin low-to-high transition
	 */
	es_ctrl_pin.write(1);
	wait_us(150);
	es_ctrl_pin.write(0);
	wait_us(500);
	es_ctrl_pin.write(1);
	wait_us(10);
}

void EasyScale::shutdown(void)
{
	es_ctrl_pin.write(0);
}

bool EasyScale::set_brightness(uint8_t brightness, uint8_t addr)
{

	// Mask input data (only 5 LSB are valid)
	// Also sets both register address bits (5, 6) to 0
	brightness &= 0x1F;
	// Add request for acknowledge bit (RFA, bit 7)
	brightness |= 0x80;

	// Write device address to bus
	write_byte(addr);

	// Send End of Stream (EOS)
	es_ctrl_pin.write(0);
	wait_us(5);

	// Send Start
	es_ctrl_pin.write(1);
	wait_us(5);

	// Write data to bus
	write_byte(brightness);

	// Send End of Stream (EOS)
	es_ctrl_pin.write(0);
	wait_us(5);

	// Check the acknowledge bit (active low output)
	es_ctrl_pin.input();
	es_ctrl_pin.mode(PullUp);
	wait_us(10);
	bool ack = !es_ctrl_pin.read();

	// Wait for ACK condition to go away to avoid driving the slave's output
	wait_us(900);
	// Set the pin back to an output and idle the bus
	es_ctrl_pin.output();
	//es_ctrl_pin.mode(PullNone);
	es_ctrl_pin.write(1);

	return ack;

}

void EasyScale::write_byte(uint8_t data_byte)
{
	// Shift out MSB first
	for(int i = 7; i >= 0; i--)
	{
		// If the bit is set, send a high
		if((data_byte & (0x01 << i)))
			send_high();
		else
			send_low();
	}
}

void EasyScale::send_low(void)
{
	es_ctrl_pin.write(0);
	wait_us(EASYSCALE_LONG_DELAY_US);
	es_ctrl_pin.write(1);
	wait_us(EASYSCALE_SHORT_DELAY_US);
}

void EasyScale::send_high(void)
{
	es_ctrl_pin.write(0);
	wait_us(EASYSCALE_SHORT_DELAY_US);
	es_ctrl_pin.write(1);
	wait_us(EASYSCALE_LONG_DELAY_US);
}
