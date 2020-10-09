/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
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
 */

#ifndef __BLE_UART_SERVICE_H__
#define __BLE_UART_SERVICE_H__

#include "platform/FileHandle.h"
#include "platform/PlatformMutex.h"

#include "ble/common/UUID.h"
#include "ble/BLE.h"
#include "ble/GattServer.h"
#include "ble/Gap.h"

#include "rtos/Queue.h"

#include <string>

#if BLE_FEATURE_GATT_SERVER

extern const uint8_t  UARTServiceBaseUUID[UUID::LENGTH_OF_LONG_UUID];
extern const uint16_t UARTServiceShortUUID;
extern const uint16_t UARTServiceTXCharacteristicShortUUID;
extern const uint16_t UARTServiceRXCharacteristicShortUUID;

extern const uint8_t  UARTServiceUUID[UUID::LENGTH_OF_LONG_UUID];
extern const uint8_t  UARTServiceUUID_reversed[UUID::LENGTH_OF_LONG_UUID];

extern const uint8_t  UARTServiceTXCharacteristicUUID[UUID::LENGTH_OF_LONG_UUID];
extern const uint8_t  UARTServiceRXCharacteristicUUID[UUID::LENGTH_OF_LONG_UUID];


/**
 * Maximum length of data (in bytes)
 * that the UART service module can transmit/receive
 * to/from the peer at one time.
 *
 * Typically MTU - 3 bytes for overhead
 */
#ifndef MBED_CONF_CORDIO_DESIRED_ATT_MTU
#define BLE_UART_SERVICE_MAX_DATA_LEN MBED_CONF_BLE_UART_SERVICE_BUFFER_SIZE
#else
#define BLE_UART_SERVICE_MAX_DATA_LEN (MBED_CONF_CORDIO_DESIRED_ATT_MTU - 3)
#endif

/**
 * TX and RX Queues
 */
#ifndef MBED_CONF_BLE_UART_SERVICE_TX_RX_QUEUE_SIZE
#define MBED_CONF_BLE_UART_SERVICE_TX_RX_QUEUE_SIZE 32
#endif

/**
 * @class UARTService.
 * @brief BLE Service to enable UART over BLE.
 *
 */
class UARTService : public mbed::FileHandle,
                    public ble::GattServer::EventHandler,
                    public ble::Gap::EventHandler {

public:

    /**
     * String with associated connection handle
     */
    class TransceivedString
    {
    public:

        TransceivedString(const char* s, size_t n, ble::connection_handle_t connection_handle = 0) : _str(s, n),
                _connection_handle(connection_handle), _is_global(false) { }

        /**
         * For TX Strings, if _is_global is true, the packet is sent
         * to all connection handles and connection_handle is ignored
         */
        void set_global(bool global) {
            _is_global = global;
        }

        bool is_global(void) const {
            return _is_global;
        }

        const std::string get_string(void) const {
            return _str;
        }

        ble::connection_handle_t get_connection_handle(void) const {
            return _connection_handle;
        }

    protected:

        std::string _str;
        ble::connection_handle_t _connection_handle;
        bool _is_global;

    };

public:

    /**
    * @param _ble
    *               BLE object for the underlying controller.
    */
    UARTService();

    virtual ~UARTService() { }

    void start(BLE &ble_interface);

    /**
     * We attempt to collect bytes before pushing them to the UART RX
     * characteristic; writing to the RX characteristic then generates
     * notifications for the client. Updates made in quick succession to a
     * notification-generating characteristic result in data being buffered
     * in the Bluetooth stack as notifications are sent out. The stack has
     * its limits for this buffering - typically a small number under 10.
     * Collecting data into the sendBuffer buffer helps mitigate the rate of
     * updates. But we shouldn't buffer a large amount of data before updating
     * the characteristic, otherwise the client needs to turn around and make
     * a long read request; this is because notifications include only the first
     * 20 bytes of the updated data.
     *
     * @param  _buffer The received update.
     * @param  length Number of characters to be appended.
     * @return        Number of characters appended to the rxCharacteristic.
     */
    ssize_t write(const void *_buffer, size_t length) override;

    ssize_t read(void *buffer, size_t size) override;

    off_t seek(off_t offset, int whence = SEEK_SET) override
    {
        /* Seeking is not support by this file handler */
        return -ESPIPE;
    }

    off_t size() override
    {
        /* Size is not defined for this file handle */
        return -EINVAL;
    }

    int isatty() override
    {
        /* File handle is used for terminal output */
        return true;
    }

    int close() override
    {
        return 0;
    }

    /** Set blocking or non-blocking mode
     *  The default is blocking.
     *
     *  @param blocking true for blocking mode, false for non-blocking mode.
     */
    int set_blocking(bool blocking) override
    {
        _blocking = blocking;
        return 0;
    }

    /** Check current blocking or non-blocking mode for file operations.
     *
     *  @return true for blocking mode, false for non-blocking mode.
     */
    bool is_blocking() const override
    {
        return _blocking;
    }

    void sigio(mbed::Callback<void()> func) override;

    /**
     * Set the application callback for when updates (notifications/indications)
     * are enabled/disabled.
     */
    void on_updates_changed(mbed::Callback<void(ble::connection_handle_t, bool)> updates_changed_cb) {
        _updates_changed_cb = updates_changed_cb;
    }

    /**
     * Function invoked when the server has sent data to a client as
     * part of a notification/indication.
     *
     * @note params has a temporary scope and should be copied by the
     * application if needed later
     */
    void onDataSent(const GattDataSentCallbackParams &params) override;

    /**
     * Function invoked when a client writes an attribute
     *
     * @note params has a temporary scope and should be copied by the
     * application if needed later
     */
    void onDataWritten(const GattWriteCallbackParams &params) override;

    /**
     * Function invoked when the GattServer instance is about
     * to be shut down. This can result in a call to reset() or BLE::reset().
     */
    void onShutdown(const GattServer &server) override {
        // TODO - flush? anything else before shutdown?
        (void)server;
    }

    /**
     * Function invoked when the client has subscribed to characteristic updates
     *
     * @note params has a temporary scope and should be copied by the
     * application if needed later
     */
    void onUpdatesEnabled(const GattUpdatesEnabledCallbackParams &params) override;

    /**
     * Function invoked when the client has unsubscribed to characteristic updates
     *
     * @note params has a temporary scope and should be copied by the
     * application if needed later
     */
    void onUpdatesDisabled(const GattUpdatesDisabledCallbackParams &params) override;

    /**
     * Called when connection attempt ends or an advertising device has been
     * connected.
     *
     * @see startAdvertising()
     * @see connect()
     *
     * @param event Connection event.
     */
    void onConnectionComplete(const ble::ConnectionCompleteEvent &event) override;

    /**
     * Called when a connection has been disconnected.
     *
     * @param event Details of the event.
     *
     * @see disconnect()
     */
    void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event) override;

protected:

    /**
     * Internal write processing function
     *
     * @retval True if txQueue is empty
     */
    bool _write(void);

protected:

    uint8_t connection_count;   /** Counter for number of current connections */
    uint8_t send_countdown;     /** Countdown for onDataSent events */
    bool sending;               /** True if currently sending notifications to connected peers */

    uint8_t             receiveBuffer[BLE_UART_SERVICE_MAX_DATA_LEN]; /**< The local buffer into which we receive
                                                                       *   inbound data before forwarding it to the
                                                                       *   application. */

    uint8_t             sendBuffer[BLE_UART_SERVICE_MAX_DATA_LEN];    /**< The local buffer into which outbound data is
                                                                       *   accumulated before being pushed to the
                                                                       *   rxCharacteristic. */

    GattCharacteristic  txCharacteristic; /**< From the point of view of the external client, this is the characteristic
                                           *   they'd write into in order to communicate with this application. */
    GattCharacteristic  rxCharacteristic; /**< From the point of view of the external client, this is the characteristic
                                           *   they'd read from in order to receive the bytes transmitted by this
                                           *   application. */

    GattAttribute::Handle_t rxCCCDHandle; /** Cached RX Characteristic CCCD handle */

    GattCharacteristic* _uart_characteristics[2];

    GattService _uart_service;

    GattServer* _server;

    /**
     * Application callback for when updates are enabled/disabled
     * Called with true as argument when updates are enabled
     * Called with false as argument when updates are disabled
     */
    mbed::Callback<void(ble::connection_handle_t, bool)> _updates_changed_cb = nullptr;

    rtos::Queue<TransceivedString, MBED_CONF_BLE_UART_SERVICE_TX_RX_QUEUE_SIZE> rxQueue; /** This is from our point of view (rxQueue = messages waiting to be read) */
    rtos::Queue<TransceivedString, MBED_CONF_BLE_UART_SERVICE_TX_RX_QUEUE_SIZE> txQueue; /** This is from our point of view (txQueue = messages waiting to be sent) */

    mbed::Callback<void()> _sigio_cb = nullptr;

    TransceivedString* next = nullptr;

    bool _blocking = true;

    PlatformMutex mutex;

};

#endif // BLE_FEATURE_GATT_SERVER

#endif /* #ifndef __BLE_UART_SERVICE_H__*/
