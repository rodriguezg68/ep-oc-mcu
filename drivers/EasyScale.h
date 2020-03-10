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
 *
 *  Driver for interfacing with devices
 *  using Texas Instrument's 1-wire EasyScale
 *  protocol such as TPS61158 backlight driver
 *
 */

#ifndef DRIVERS_EASYSCALE_H_
#define DRIVERS_EASYSCALE_H_

#include "platform/platform.h"

#include "drivers/DigitalInOut.h"

class EasyScale
{
	public:

		static uint8_t DEVICE_ADDRESS_TPS61158;		/** Device address for TPS61158 backlight driver IC */

	public:

		/**
		 * Instantiate an EasyScale 1-wire protocol instance
		 * @param[in] ctrl_pin Pin used for EasyScale data transfer
		 */
		EasyScale(PinName ctrl_pin);

		~EasyScale() { }

		/**
		 * Powers on devices connected to the EasyScale control pin
		 * Takes care of configuring connected devices to
		 * use EasyScale as the control input
		 */
		void power_on(void);

		/**
		 * Shuts down devices connected to the EasyScale control pin
		 */
		void shutdown(void);

		/**
		 * Sets the brightness
		 * @param[in] brightness Brightness setting ranging from 0 (off) to 31 (full brightness)
		 * @param[in] addr Device address to send to
		 * @note This class contains static device addresses for various TI chips
		 *
		 * @retval 0 if brightness setting was acknowledged, 1 otherwise
		 */
		bool set_brightness(uint8_t brightness, uint8_t addr = DEVICE_ADDRESS_TPS61158);

	protected:

		/**
		 * Writes a byte to the EasyScale bus
		 * @param[in] data_byte Byte to write to bus
		 */
		void write_byte(uint8_t data_byte);

		/**
		 * Writes a low bit to the bus
		 * (Logic 0: t_low >= 2 * t_high)
		 */
		void send_low(void);

		/**
		 * Writes a high bit to the bus
		 * (Logic 1: t_high >= 2 * t_low)
		 */
		void send_high(void);

	protected:

		/** Digital Input/Output for EasyScale control pin */
		mbed::DigitalInOut es_ctrl_pin;


};

#endif /* DRIVERS_EASYSCALE_H_ */
