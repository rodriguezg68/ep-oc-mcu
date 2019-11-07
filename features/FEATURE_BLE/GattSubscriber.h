/**
 * @file GattSubscriber.h
 * @brief Brief Description
 * 
 * Detailed Description
 *
 * Link to [markdown like this](@ref PageTag)
 * Make sure you tag the markdown page like this:
 * # Page title {#PageTag}
 * 
 * <a href='MyPDF.pdf'> Link to PDF documents like this</a>
 * If you add document files, make sure to add them into a directory inside a "docs" folder
 * And then run hud-devices/tools/copy-dox-files.py 
 *
 * To use images, make sure they're in an "images" folder and follow the doxygen user manual to add images.
 * You must run copy-dox-files.py after adding images as well.
 *
 * @copyright Copyright &copy; 2018 Heads Up Display, Inc.
 *
 *  Created on: Jul 13, 2018
 *      Author: gdbeckstein
 */
#ifndef INCLUDE_BLE_CLIENTS_GATTSUBSCRIBER_H_
#define INCLUDE_BLE_CLIENTS_GATTSUBSCRIBER_H_

#include "platform/Callback.h"

#include "drivers/Timeout.h"

#include "ble/BLE.h"
#include "ble/GattClient.h"
#include "ble/Gap.h"
#include "ble/UUID.h"
#include "ble/DiscoveredCharacteristic.h"
#include "ble/DiscoveredCharacteristicDescriptor.h"

#include "events/EventQueue.h"

#define GATT_SUBSCRIBER_TIMEOUT_US 5000000
#define GATT_SUBSCRIBER_RETRY_MS	  2000

class GattSubscriber
{
	public:

		typedef enum subscriber_state_t
		{
			STATE_INITIALIZED,
			STATE_DISCOVERING_SERVICE,
			STATE_DISCOVERING_CHARACTERISTICS,
			STATE_DISCOVERING_DESCRIPTORS,
			STATE_SUBSCRIBING,
			STATE_SUBSCRIBED,
			STATE_FAILED,
			STATE_SHUTTING_DOWN
		} subscriber_state_t;

		typedef enum subscription_t
		{
			NO_SUBSCRIPTION			= 0x00,
			SUBSCRIPTION_NOTIFY		= BLE_HVX_NOTIFICATION,
			SUBSCRIPTION_INDICATE	= BLE_HVX_INDICATION
		} subscription_t;

		// Errors match up with the state in which they happened
		typedef enum result_status_t
		{
			SUBSCRIBE_SERVICE_DISCOVERY_FAILED			= STATE_DISCOVERING_SERVICE,
			SUBSCRIBE_CHARACTERISTIC_DISCOVERY_FAILED = STATE_DISCOVERING_CHARACTERISTICS,
			SUBSCRIBE_DESCRIPTOR_DISCOVERY_FAILED		= STATE_DISCOVERING_DESCRIPTORS,
			SUBSCRIBE_FAIL										= STATE_SUBSCRIBING,
			SUBSCRIBE_SUCCESS
		} result_status_t;

		typedef struct char_query_t
		{
			UUID uuid;	/**!< Associated BLE UUID for the desired GattCharacteristic */
			//bool required;	/**!< Is the existence of the characteristic required? */
			subscription_t subscription;	/**!< Desired type of subscription */
		} char_query_t;

		typedef struct subscription_spec_t
		{
				UUID service_uuid;		/**!< Associated BLE service UUID */
				const char_query_t* characteristics;	/**!< BLE Characteristics queries */
				uint8_t num_characteristics;	/**!< Number of characteristics in query */
		} subscription_spec_t;

		typedef struct subscription_char_t
		{
				DiscoveredCharacteristic characteristic;
				GattAttribute::Handle_t descriptor;
		} subscription_char_t;

		typedef struct result_t
		{
				result_status_t status; /*!< Status of the result */
				UUID service_uuid;	/*!< UUID of the associated Gatt Service */
				subscription_char_t* discovered_chars; /*!< Discovered characteristics and their descriptors */
				uint8_t num_chars; /*!< Number of discovered characteristics */
		} result_t;

	public:

		GattSubscriber(events::EventQueue& queue);

		~GattSubscriber();

		/**
		 * Starts discovery and subscription process.
		 * The GattSubscriber will take the provided subscription spec
		 * and first try to discover the specified Gatt Service
		 * on the connected peer (specified by connection_handle).
		 *
		 * If the service is found, the GattSubscriber will then attempt
		 * to discover any specified characteristics within that service.
		 * If any of the required characteristics aren't found, failure will
		 * be reported.
		 *
		 * Once all required characteristics have been discovered, the GattSubscriber
		 * will attempt to subscribe to any characteristics specified. To that end,
		 * the GattSubscriber will attempt to discover the descriptors of these
		 * specified characteristics.
		 *
		 * If all subscriptions are made successfully, success will be returned.
		 *
		 * Results are communicated to the application via the result_cb argument.
		 *
		 * @note this is an asynchronous call and will return immediatly
		 * @note characteristics that require encryption will not be
		 * visible on the peer unless the application first pairs!
		 *
		 * @param[in] result_cb Application callback to communicate discovery results
		 * @param[in] spec Struct specifying services and characteristics to discover/subscribe
		 * @param[in] connection_handle Handle of the peer connection to perform discovery on
		 * @param[in] max_retries (default: 3) max number of operation retries before reporting failure
		 */
		void discover_and_subscribe(mbed::Callback<void(result_t)> result_cb,
				subscription_spec_t* spec, Gap::Handle_t* connection_handle,
				int max_retries = 3);

		/**
		 * Gets the state of the discovery and subscription process
		 *
		 * @retval state Internal state of discovery process
		 */
		subscriber_state_t get_state() { return _state; }

		/** Resets the subscriber state machine to its initial state */
		void reset(void);


	private:

		/**
		 * Updates the internal state based on callback events
		 * from the BLE stack
		 *
		 * @param[in] success true if last state's operation was successful
		 */
		void update_state(bool success);

		/** Starts service discovery */
		void start_service_discovery(void);

		/** Starts characteristic discovery */
		void start_characteristic_discovery(void);

		/** Start descriptor discovery */
		void start_descriptor_discovery(void);

		/* Initiates subscribing */
		void subscribe(void);

		/***** BLE Stack Callbacks ******/

		void service_discovered_cb(const DiscoveredService* service);

		void discovery_termination_cb(Gap::Handle_t handle);

		void characteristic_discovered_cb(const DiscoveredCharacteristic* characteristic);

		void descriptor_discovery_cb(
				const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t* params);

		void descriptor_discovery_termination_cb(
				const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t* params);

		void on_written_cb(const GattWriteCallbackParams* params);

		void on_disconnect(const Gap::DisconnectionCallbackParams_t* params);

		/***** Timeout *******/

		void timeout_cb(void);

		void reset_timeout(void);

	private:

		/** EventQueue context to execute on */
		events::EventQueue& _queue;

		/** State of the subscriber state machine */
		subscriber_state_t _state;

		/** Application result callback */
		mbed::Callback<void(result_t)> _result_cb;

		/** Subscription specifications */
		subscription_spec_t* _spec;

		/** Handle to peer connection */
		Gap::Handle_t* _connection_handle;

		/** Maximum number of retries */
		int _max_retries;

		/** Number of retries left before failure */
		int _retries_left;

		/** Timeout timer for retries */
		mbed::Timeout _timeout;

		/** Local result variable */
		result_t _result;

		/** Character array for result */
		subscription_char_t* _discovered_chars;

		/**
		 * Ignore next discovery termination callback
		 * (Even on success, discovery termination cb is executed)
		 */
		volatile bool _ignore_termination_cb;

		/** Dynamic array of flags for characteristics */
		bool* _char_flags;

		/** Index of current characteristic for descriptor discovery */
		uint8_t _idx_current_char;


};



#endif /* INCLUDE_BLE_CLIENTS_GATTSUBSCRIBER_H_ */
