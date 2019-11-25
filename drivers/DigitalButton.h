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

#ifndef DRIVERS_DIGITALBUTTON_H_
#define DRIVERS_DIGITALBUTTON_H_

#include "drivers/ButtonIn.h"
#include "drivers/DebounceIn.h"

namespace ep
{
	/** \addtogroup drivers */

	/*
	 * A button input child class based on the DebounceIn class
	 */
	class DigitalButton : public ButtonIn, private DebounceIn
	{
		public:

			/**
			 * Instantiate a DigitalButton
			 * @param[in] pin Pin to use as button input
			 * @param[in] active_low True if the underlying input is low when the button is pressed, false otherwise
			 * @param[in] sp_cb (optional) Short press callback
			 * @param[in] lp_cb (optional) Long press callback
			 * @param[in] debounce_ms (optional) The number of milliseconds to debounce
			 *
			 * @note callbacks are executed in the interrupt context
			 */
			DigitalButton(PinName pin, bool active_low, mbed::Callback<void(ButtonIn*)> sp_cb = NULL,
					mbed::Callback<void(ButtonIn*)> lp_cb = NULL, unsigned int debounce_ms = 100) :
			ButtonIn(active_low, sp_cb, lp_cb),
			DebounceIn(pin, debounce_ms)
			{
				/** Attach the ButtonIn handlers */

				/** If the button is active low, pressed = on_fall, released = on_rise */
				if(active_low)
				{
					this->fall(mbed::callback(this, &ButtonIn::_internal_press_handler));
					this->rise(mbed::callback(this, &ButtonIn::_internal_release_handler));
				}
				else
				{
					/** Otherwise pressed = on_rise, released = on_fall */
					this->rise(mbed::callback(this, &ButtonIn::_internal_press_handler));
					this->fall(mbed::callback(this, &ButtonIn::_internal_release_handler));
				}
			}

			virtual ~DigitalButton() { }

			/**
			 * Read the status of the input
			 * @retval An integer representing the state of the underlying button input
			 * 0 - released, 1 - pressed
			 */
			virtual int status()
			{
				// Call the underlying DebounceIn read function
				return this->read();
			}


	};
}


#endif /* DRIVERS_DIGITALBUTTON_H_ */
