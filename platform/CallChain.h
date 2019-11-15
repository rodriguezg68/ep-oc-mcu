/*
 * Callchain.h
 *
 *  Created on: Nov 13, 2019
 *      Author: becksteing
 */

#ifndef EP_OC_MCU_PLATFORM_CALLCHAIN_H_
#define EP_OC_MCU_PLATFORM_CALLCHAIN_H_

#include "platform/Callback.h"
#include "platform/NonCopyable.h"

//#include <type_traits>



namespace ep {


	/**
	 * A linked-list structure of Callbacks that are triggered
	 * in the sequence they are added.
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

		/** Linked-list CallChain element */
		class CallChainLink
		{

		public:


			CallChainLink(const LinkCallback& cb) : callback(cb) {
				next_element = NULL;
			}

			/**
			 * Attaches another callback in the chain
			 * @param[in] cb Callback to chain next
			 */
			void attach_next(const LinkCallback& cb) {
				next_element = new CallChainLink(cb);
			}

			/**
			 * Returns the next chain link or NULL if this is the last link
			 * @retval next Next callback link, or NULL if reached the end
			 */
			CallChainLink* next(void) {
				return next_element;
			}

			void set_next(CallChainLink* element) {
				next_element = element;
			}

			LinkCallback& get_callback(void) {
				return callback;
			}

			void operator()(ArgTs... args) const {
				callback(args...);
			}

		private:
			LinkCallback callback;
			CallChainLink* next_element;
		};

	public:

		CallChain() : head(NULL) {
		}

		CallChain(LinkCallback& first_cb) {
			head = new CallChainLink(first_cb);
		}

		~CallChain() {
			this->detach_all();
		}

		/** Attach a callback to the callchain
		 * @param[in] callback Callback to attached to the callchain
		 */
		void attach(const LinkCallback& callback) {

			/** Iterate to the end of the callchain */
			CallChainLink* element = head;

			/** Create the first element if it doesn't exist */
			if(!element) {
				element = new CallChainLink(callback);
				return;
			}

			while(element->next()) {
				element = element->next();
			}

			/** element will now be the last link in the chain, add here */
			element->attach_next(callback);

		}

		/**
		 * Detach
		 * @param[in] callback Callback to remove from the callchain
		 *
		 * @note: The callback object does not have to be the same exact
		 * object. Equivalency is based on memory comparison, not pointer comparison
		 */
		void detach(const LinkCallback& callback) {

			/** Iterate to the element with the matching callback */
			CallChainLink *prev_element, *curr_element = head;

			/** No elements to remove */
			if(!curr_element) {
				return;
			}

			while(curr_element->next()) {

				/** Break out of the loop once we reach the desired element */
				if(curr_element->get_callback() == callback) {
					break;
				}

				prev_element = curr_element;
				curr_element = curr_element->next();
			}

			/** Once we get here, we either found the desired element
			 * or reached the end of the chain, make sure it's the desired element
			 */

			// Reached the end of the chain without finding the desired callback, return
			if(curr_element->get_callback() != callback) {
				return;
			}

			// Delete the current element and patch the chain
			prev_element->set_next(curr_element->next());
			delete curr_element;

		}

		void detach_all(void) {

			/** Iterate through all elements and delete them */
			CallChainLink *prev_element, *element = head;

			/** No elements to remove */
			if(!element) {
				return;
			}

			while(element->next()) {
				prev_element = element;
				element = element->next();
				delete prev_element;
			}

			/** Delete the last element */
			delete element;

		}

		/**
		 * Invoke all callbacks in this chain
		 * @param[in] args Arguments to pass to each callback in the chain
		 */
		void call(ArgTs... args) {
			/** Iterate to the end of the callchain */
			CallChainLink* element = head;

			if(!element) {
				return;
			}

			do {
				*element(args...); // Call the callback with given args
				element = element->next();
			} while(element);
		}

		void operator()(ArgTs... args) {
			call(args...);
		}

	private:

		CallChainLink* head; /** First element in the callchain */


	};
}


#endif /* EP_OC_MCU_PLATFORM_CALLCHAIN_H_ */
