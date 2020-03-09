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
     * API for a CallChainLink
     *
     * The callchain API can be extended by subclassing this class.
     *
     * eg: if you need to add more information to a callback (variable threshold, etc)
     * then you can create a subclass that adds this information.
     */
    template<typename... ArgTs>
    class CallChainLink
    {

    public:

        CallChainLink(mbed::Callback<void(ArgTs...)> cb) : cb(cb) {

        }

        virtual ~CallChainLink() { }

        virtual void call(ArgTs... args) {
            this->cb.call(args...);
        }

        virtual bool operator==(const CallChainLink<ArgTs...> &rhs) {
            // Compare based on Callback equivalency
            return (this->cb == rhs.cb);
        }

        virtual bool operator!=(const CallChainLink<ArgTs...> &rhs) {
            // Compare based on Callback equivalency
            return (this->cb != rhs.cb);
        }

    protected:

        mbed::Callback<void(ArgTs...)> cb;

    };

	/**
	 * A linked-list structure of Callbacks that are triggered
	 * by a common event.
	 *
	 * If the application needs to add more information to the CallChainLink
	 * (eg: an invidual threshold for each handler) it can do so by replacing
	 * this Link type
	 *
	 * @note: this API does NOT guarantee ANY specific order of execution!
	 */
	template<typename... ArgTs>
	class CallChain : private mbed::NonCopyable<CallChain<ArgTs...>>
	{

	public:

		CallChain() : chain() {
		}

		virtual ~CallChain() {
			this->detach_all();
		}

		/** Attach a callback to the callchain
		 * @param[in] callback Callback to attach to the callchain
		 */
		virtual void attach(const CallChainLink<ArgTs...>& callback) {

			/** Make sure a duplicate isn't being added */
			for(CallChainLink<ArgTs...> cb : chain) {
				if(cb == callback) {
					return;
				}
			}

			/** Made it here, add the callback to the end of the list */
			chain.push_front(callback);
		}

        /**
         * Attach a callback to the callchain, initializing with a Callback instance
         * @param[in] callback Callback to attach to the callchain
         */
        virtual void attach(const mbed::Callback<void(ArgTs...)>& cb) {
            this->attach(CallChainLink<ArgTs...>(cb));
        }

		/**
		 * Detach
		 * @param[in] callback Callback to remove from the callchain
		 *
		 * @note: The callback object does not have to be the same exact
		 * object. Equivalency is based on memory comparison, not pointer comparison
		 */
		virtual void detach(const CallChainLink<ArgTs...>& callback) {
			chain.remove(callback);
		}

        /**
         * Detach, initializing with a Callback instance
         * @param[in] callback Callback to remove from the callchain
         *
         * @note: The callback object does not have to be the same exact
         * object. Equivalency is based on memory comparison, not pointer comparison
         */
        virtual void detach(const mbed::Callback<void(ArgTs...)>& cb) {
            this->detach(CallChainLink<ArgTs...>(cb));
        }

		void detach_all(void) {
			chain.clear();
		}

		/**
		 * Invoke all callbacks in this chain
		 * @param[in] args Arguments to pass to each callback in the chain
		 */
		void call(ArgTs... args) {

			for(CallChainLink<ArgTs...> cb : chain) {
				cb.call(args...);
			}
		}

		void operator()(ArgTs... args) {
			call(args...);
		}

	protected:

		std::forward_list<CallChainLink<ArgTs...>> chain; /** Singly-linked list to store callbacks */
	};
}


#endif /* EP_OC_MCU_PLATFORM_CALLCHAIN_H_ */
