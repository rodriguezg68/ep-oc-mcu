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
     */
    template<typename... ArgTs>
    class CallChainLink
    {

    public:

        virtual ~CallChainLink() { }

        virtual void call(ArgTs... args) = 0;

    };

    /**
     * Default basic callback CallChainLink
     *
     * @note The inherintance order matters in this case as
     * both mbed::Callback and ep::CallChainLink define a method named "call".
     * In this case we want to use the one defined by mbed::Callback.
     */
    template<typename... ArgTs>
    class CallbackChainLink : public mbed::Callback<void(ArgTs...)>, public CallChainLink<ArgTs...>
    { };


	/**
	 * A linked-list structure of Callbacks that are triggered
	 * by a common event.
	 *
	 * The Link template parameter defaults to a basic CallbackChainLink.
	 *
	 * If the application needs to add more information to the CallChainLink
	 * (eg: an invidual threshold for each handler) it can do so by replacing
	 * this Link type
	 *
	 * @note: this API does NOT guarantee ANY specific order of execution!
	 */
	template<typename... ArgTs, typename Link = CallbackChainLink<ArgTs...>>
	class CallChain : private mbed::NonCopyable<CallChain<ArgTs..., Link>>
	{

	    /** Restrict the Link template parameter to subclasses of CallChainLink */
	    static_assert(std::is_base_of<CallChainLink, Link>::value, "Link must be derived from CallChainLink");

	public:

		CallChain() : chain() {
		}

		virtual ~CallChain() {
			this->detach_all();
		}

		/** Attach a callback to the callchain
		 * @param[in] callback Callback to attached to the callchain
		 */
		virtual void attach(const Link& callback) {

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
		virtual void detach(const Link& callback) {
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

			for(Link cb : chain) {
				cb.call(args...);
			}
		}

		void operator()(ArgTs... args) {
			call(args...);
		}

	protected:

		std::forward_list<Link> chain; /** Singly-linked list to store callbacks */
	};

	/**
	 * Template-specialized CallChain with the basic CallbackChainLink
	 */
	template<typename... ArgTs>
	class CallChain<ArgTs... , CallbackChainLink<ArgTs...>> {

	public:

        /** Attach a callback to the callchain
         * @param[in] callback Callback to attached to the callchain
         */
        virtual void attach(const ) {

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
        virtual void detach(const Link& callback) {
            chain.remove(callback);
        }

	};
}


#endif /* EP_OC_MCU_PLATFORM_CALLCHAIN_H_ */
