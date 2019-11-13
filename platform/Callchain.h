/*
 * Callchain.h
 *
 *  Created on: Nov 13, 2019
 *      Author: becksteing
 */

#ifndef EP_OC_MCU_PLATFORM_CALLCHAIN_H_
#define EP_OC_MCU_PLATFORM_CALLCHAIN_H_

#include "platform/Callback.h"

#include <type_traits>



namespace ep
{

	/**
	 * A linked-list structure of Callbacks that are triggered
	 * in the sequence they are added.
	 */
	template<class T>
	class Callchain
	{
		/** Callchain should only be used for Callback types */
		static_assert(std::is_base_of<mbed::Callback, T>::value,
				"T must be a type of Callback");

	protected:

		/** Linked-list Callchain element */
		template<class T>
		class CallchainElement
		{

		public:
			CallchainElement(T* cb) : callback(cb) {
				next_element = NULL;
			}

			/**
			 * Attaches another callback in the chain
			 * @param[in] cb Callback to chain next
			 * @retval The new chain link
			 */
			CallchainElement<T>* attach_next(T* cb) {
				next_element = new CallchainElement<T>(cb);
				return next_element;
			}

			/**
			 * Returns the next chain link or NULL if this is the last link
			 * @retval next Next callback link, or NULL if reached the end
			 */
			CallchainElement<T>* next(void) {
				return next_element;
			}

			void set_next(CallchainElement<T>* element) {
				next_element = element;
			}

		private:
			T* callback;
			CallchainElement<T>* next_element;
		};

	public:

		Callchain() : head(NULL) {
		}

		~Callchain() {
			this->detach_all();
		}

		/** Attach a callback to the callchain
		 * @param[in] callback Callback to attached to the callchain
		 */
		void attach(T* callback) {

			/** Iterate to the end of the callchain */
			CallchainElement<T>* element = head;

			/** Create the first element if it doesn't exist */
			if(!element) {
				element = new CallchainElement<T>(callback);
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
		void detach(T* callback) {

			/** Iterate to the element with the matching callback */
			CallchainElement<T> *prev_element, *curr_element = head;

			/** No elements to remove */
			if(!curr_element) {
				return;
			}

			while(curr_element->next()) {

				/** Break out of the loop once we reach the desired element */
				if(curr_element == callback) {
					break;
				}

				prev_element = curr_element;
				curr_element = curr_element->next();
			}

			/** Once we get here, we either found the desired element
			 * or reached the end of the chain, make sure it's the desired element
			 */

			// Reached the end of the chain without finding the desired callback, return
			if(curr_element != callback) {
				return;
			}

			// Delete the current element and patch the chain
			prev_element->set_next(curr_element->next());
			delete curr_element;

		}

		void detach_all(void) {

			/** Iterate through all elements and delete them */
			CallchainElement<T>* prev_element, element = head;

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

	private:

		CallchainElement<T>* head; /** First element in the callchain */


	};
}


#endif /* EP_OC_MCU_PLATFORM_CALLCHAIN_H_ */
