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
 *	EasyScale example program
 *
 *	Compile for NRF52840_DK
 *
 *  Created on: Sep 22, 2018
 *      Author: gdbeckstein
 */

#include "devices/EasyScale.h"

#include "rtos/ThisThread.h"

#define EASYSCALE_CTRL_PIN P0_3

EasyScale backlight_ctrl(EASYSCALE_CTRL_PIN);

int main(void)
{
	backlight_ctrl.power_on();

	// Fade brightness up and down
	uint8_t brightness = 0;
	bool going_up = true;
	while(true)
	{
		backlight_ctrl.set_brightness(brightness,
				EasyScale::DEVICE_ADDRESS_TPS61158);

		if(going_up)
		{
			brightness++;
			if(brightness >= 31)
				going_up = false;
		}
		else
		{
			brightness--;
			if(brightness == 0)
				going_up = true;
		}
                rtos::ThisThread::sleep_for(250ms);
	}
}


