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
 * Derived from source code originally written by Andy Kirkham
 */

/**
 * Modified by becksteing (aglass0fmilk) 5/2/2019
 * - Changed to use low power ticker
 */

#if defined(DEVICE_LPTICKER) || defined(DEVICE_LOWPOWERTIMER)

#ifndef DEBOUNCEIN_H
#define DEBOUNCEIN_H

#include "platform/Callback.h"
#include "drivers/LowPowerTicker.h"
#include "drivers/DigitalIn.h"

namespace ep
{

	/** DebounceIn adds mechanical switch debouncing to DigitialIn.
	 *
	 * Example:
	 * @code
	 * #include "mbed.h"
	 * #include "DebounceIn.h"
	 *
	 * DebounceIn  d(p5);
	 * DigitialOut led1(LED1);
	 * DigitialOut led2(LED2);
	 *
	 * int main() {
	 *     while(1) {
	 *         led1 = d;
	 *         led2 = d.read();
	 *     }
	 * }
	 * @endcode
	 *
	 * @see set_debounce_us() To change the sampling frequency.
	 * @see set_samples() To alter the number of samples.
	 *
	 * Users of this library may also be interested in PinDetect library:-
	 * @see http://mbed.org/users/AjK/libraries/PinDetect/latest
	 *
	 * This example shows one input displayed by two outputs. The input
	 * is debounced by the default 10ms.
	 */

	class DebounceIn : public mbed::DigitalIn
	{
		public:

			/** Sets the debounce time in milliseconds
			 * @note granularity of only 10ms is supported (rounds down)
			 * @param debounce_ms
			 */
			void set_debounce(unsigned int debounce_ms) {
				_samples = (debounce_ms / 10);
				if (_samples == 0)
					_samples = 1;
			}

			/** Attach a function to call when a rising edge occurs on the debounced input
			 *
			 *  @param func A pointer to a void function, or 0 to set as none
			 *
			 *  @note called in the interrupt context
			 */
			void rise(mbed::Callback<void()> func) {
				_rise = func;
			}

			/** Attach a function to call when a falling edge occurs on the debounced input
			 *
			 *  @param func A pointer to a void function, or 0 to set as none
			 *
			 *  @note called in the interrupt context
			 */
			void fall(mbed::Callback<void()> func) {
				_fall = func;
			}

			/** read
			 *
			 * Read the value of the debounced pin.
			 */
			int read(void) {
				return _shadow;
			}

			/** operator int()
			 *
			 * Read the value of the debounced pin.
			 */
			operator int() {
				return read();
			}

			/** Constructor
			 *
			 * @param PinName pin The pin to assign as an input.
			 * @param debounce_ms The number of milliseconds to debounce
			 */
			DebounceIn(PinName pin, unsigned int debounce_ms = 100) : mbed::DigitalIn(pin) {
				_counter = 0;
				set_debounce(debounce_ms);
				set_debounce_us(10000);

				_prevShadow = mbed::DigitalIn::read();
			};

		protected:
			void _callback(void)
			{
				if (mbed::DigitalIn::read())
				{
					if (_counter < _samples) {
						_counter++;
					}

					if (_counter == _samples)
					{
						_shadow = 1;
						if (_shadow != _prevShadow && _rise)
						{
							_prevShadow = _shadow;
							_rise();
						}
					}
				}
				else
				{
					if (_counter > 0) {
						_counter--;
					}

					if (_counter == 0)
					{
						_shadow = 0;
						if (_shadow != _prevShadow && _fall)
						{
							_prevShadow = _shadow;
							_fall();
						}
					}
				}
			}

			/** set_debounce_us
			 *
			 * Sets the debounce sample period time in microseconds, default is 1000 (1ms)
			 *
			 * @param sample_period_us The debounce sample period time to set.
			 */
			void set_debounce_us(unsigned int sample_period_us) {
				_ticker.attach_us(mbed::callback(this, &DebounceIn::_callback), sample_period_us);
			}

			/** set_samples
			 *
			 * Defines the number of samples before switching the shadow
			 * definition of the pin.
			 *
			 * @param num_samples The number of samples.
			 */
			void set_samples(unsigned int num_samples) {
				_samples = num_samples;
			}

			mbed::LowPowerTicker _ticker;
			int _shadow;
			int _prevShadow;
			int _counter;
			int _samples;
			mbed::Callback<void()> _rise;
			mbed::Callback<void()> _fall;
	};
}

#endif

#endif /** defined(DEVICE_LPTICKER) || defined(DEVICE_LOWPOWERTIMER) */

