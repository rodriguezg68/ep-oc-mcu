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

#ifndef EP_OC_MCU_PLATFORM_CALLCHAIN_H_
#define EP_OC_MCU_PLATFORM_CALLCHAIN_H_

#include "platform/Callback.h"
#include "platform/NonCopyable.h"

#include <forward_list>


namespace ep {


	/**
	 * A linked-list structure of Callbacks that are triggered
	 * by a common event.
	 *
	 * @note: this API does NOT guarantee ANY specific order of execution!
	 */
	template<typename... ArgTs>
	class CallChain : private mbed::NonCopyable<CallChain<ArgTs...>>
	{

	protected:

		/*
		 * Callbacks in the CallChain may not return values -- it wouldn't
		 * make much sense (how do you determine which value to use?)
		 *
		 * So restrict CallChain to only Callbacks that return void
		 */
		using LinkCallback = mbed::Callback<void(ArgTs...)>;

	public:

		CallChain() : chain() {
		}

		~CallChain() {
			this->detach_all();
		}

		/** Attach a callback to the callchain
		 * @param[in] callback Callback to attached to the callchain
		 */
		void attach(const LinkCallback& callback) {

			/** Make sure a duplicate isn't being added */
			for(LinkCallback cb : chain) {
				if(cb == callback) {
					return;
				}
			}

			/** Made it here, add the callback to the end of the list */
			chain.push_front(callback);
		}

		/**
		 * Detach
		 * @param[in] callback Callback to remove from the callchain
		 *
		 * @note: The callback object does not have to be the same exact
		 * object. Equivalency is based on memory comparison, not pointer comparison
		 */
		void detach(const LinkCallback& callback) {
			chain.remove(callback);
		}

		void detach_all(void) {
			chain.clear();
		}

		/**
		 * Invoke all callbacks in this chain
		 * @param[in] args Arguments to pass to each callback in the chain
		 */
		void call(ArgTs... args) {

			for(LinkCallback cb : chain) {
				cb(args...);
			}
		}

		void operator()(ArgTs... args) {
			call(args...);
		}

	private:

		std::forward_list<LinkCallback> chain; /** Singly-linked list to store callbacks */
	};
}


#endif /* EP_OC_MCU_PLATFORM_CALLCHAIN_H_ */
