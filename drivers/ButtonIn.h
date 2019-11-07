/**
 * @file ButtonIn.h
 * @brief A button input that handles long press and short press logic
 * 
 *  Created on: 5/1/2019
 *      Author: gdbeckstein
 */
#ifndef DRIVERS_BUTTONIN_H_
#define DRIVERS_BUTTONIN_H_

#include "drivers/Timeout.h"
#include "platform/Callback.h"
#include "platform/NonCopyable.h"

#define BUTTON_IN_DEFAULT_LONG_PRESS_DELAY_MS 3000

namespace ep
{

	/** \addtogroup drivers */

	/*
	 * A button input that handles long press and short press logic
	 *
	 * This is an abstract class. Concrete implementations must inherit from
	 * this class depending on the type of input (digital, comparator, etc)
	 */
	class ButtonIn : private mbed::NonCopyable<ButtonIn>
	{

		public:

			/**
			 * Constructor
			 * @param[in] active_low True if the underlying input is low when the button is pressed, false otherwise
			 * @param[in] sp_cb (optional) Short press callback
			 * @param[in] lp_cb (optional) Long press callback
			 */
			ButtonIn(bool active_low, mbed::Callback<void(ButtonIn*)> sp_cb = NULL,
					mbed::Callback<void(ButtonIn*)> lp_cb = NULL) : _is_active_low(active_low), _timeout(), _timeout_scheduled(false),
			_long_press_delay_ms(BUTTON_IN_DEFAULT_LONG_PRESS_DELAY_MS),
			_short_press_cb(sp_cb), _long_press_cb(lp_cb)
			{ }

			virtual ~ButtonIn() { }

			/**
			 * Read the status of the input
			 * @retval An integer representing the state of the underlying button input
			 * 0 - released, 1 - pressed
			 */
			virtual int status() = 0;

			/**
			 * An operator shorthand for status()
			 */
			operator int() { return status(); }

			/**
			 * Returns whether the button input is active low or not
			 * @retval true if the button is active low (that is, input is low when button is pressed)
			 */
			bool is_active_low(void)
			{
				return _is_active_low;
			}

			/**
			 * Sets the long press delay
			 * @param[in] long_press_delay_ms The delay for recognizing a long press in ms
			 */
			void set_long_press_delay(uint32_t long_press_delay_ms)
			{
				_long_press_delay_ms = long_press_delay_ms;
			}

			/**
			 * Attach a short press interrupt callback
			 * @param func A callback to be executed on a short press
			 *
			 * @note callback is called in the interrupt context
			 */
			void attach_short_press_callback(mbed::Callback<void(ButtonIn*)> func)
			{
				_short_press_cb = func;
			}

			/** Attach a long press interrupt callback
			 * @param[in] func A callback to be executed when the button is held for \p long_press_delay_ms
			 *
			 * @note callback is called in the interrupt context
			 */
			void attach_long_press_callback(mbed::Callback<void(ButtonIn*)> func)
			{
				_long_press_cb = func;
			}

			/**
			 * Internal long press callback
			 */
			void _internal_long_press_handler(void)
			{
				// Clear the flag
				_timeout_scheduled = false;

				// Execute the application long press handler
				if(_long_press_cb)
					_long_press_cb(this);
			}

			/**
			 * Called by child class when the underlying button is pressed
			 */
			void _internal_press_handler(void)
			{
				// Start a timeout for desired delay
				_timeout_scheduled = true;
				_timeout.attach_us(
						mbed::callback(this, &ButtonIn::_internal_long_press_handler),
						_long_press_delay_ms * 1000);
			}

			/**
			 * Called by child class when the underlying button is released
			 */
			void _internal_release_handler(void)
			{
				// Timeout is scheduled, cancel it and call short press handler
				if(_timeout_scheduled)
				{
					_timeout.detach();
					_timeout_scheduled = false;
					if(_short_press_cb)
						_short_press_cb(this);
				}
			}

		protected:

			/*!< Indicates whether this button is active high or active low */
			bool _is_active_low;

		private:

			/** Timeout for executing long press callbacks */
			mbed::Timeout _timeout;

			/** Indicates if a timeout has been scheduled */
			volatile bool _timeout_scheduled;

			/*!< Delay for recognizing a long press in milliseconds */
			uint32_t _long_press_delay_ms;

			/*!< Application short and long press callbacks */
			mbed::Callback<void(ButtonIn*)> _short_press_cb;
			mbed::Callback<void(ButtonIn*)> _long_press_cb;
	};
}

#endif /* DRIVERS_BUTTONIN_H_ */
