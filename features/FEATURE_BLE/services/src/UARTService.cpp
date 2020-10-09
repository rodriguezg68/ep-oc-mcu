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

// TODO - write to individual connections
// TODO - blocking/non-blocking modes
// TODO - read from individual connections

#if BLE_FEATURE_GATT_SERVER

#include "UARTService.h"

#include "mbed_trace.h"

#include "Kernel.h"

#define TRACE_GROUP "btuart"

#define DURATION_ZERO rtos::Kernel::Clock::duration_u32::zero()

const uint8_t  UARTServiceBaseUUID[UUID::LENGTH_OF_LONG_UUID] = {
    0x6E, 0x40, 0x00, 0x00, 0xB5, 0xA3, 0xF3, 0x93,
    0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E,
};
const uint16_t UARTServiceShortUUID                 = 0x0001;
const uint16_t UARTServiceTXCharacteristicShortUUID = 0x0002;
const uint16_t UARTServiceRXCharacteristicShortUUID = 0x0003;
const uint8_t  UARTServiceUUID[UUID::LENGTH_OF_LONG_UUID] = {
    0x6E, 0x40, (uint8_t)(UARTServiceShortUUID >> 8), (uint8_t)(UARTServiceShortUUID & 0xFF), 0xB5, 0xA3, 0xF3, 0x93,
    0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E,
};
const uint8_t  UARTServiceUUID_reversed[UUID::LENGTH_OF_LONG_UUID] = {
    0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0,
    0x93, 0xF3, 0xA3, 0xB5, (uint8_t)(UARTServiceShortUUID & 0xFF), (uint8_t)(UARTServiceShortUUID >> 8), 0x40, 0x6E
};
const uint8_t  UARTServiceTXCharacteristicUUID[UUID::LENGTH_OF_LONG_UUID] = {
    0x6E, 0x40, (uint8_t)(UARTServiceTXCharacteristicShortUUID >> 8), (uint8_t)(UARTServiceTXCharacteristicShortUUID & 0xFF), 0xB5, 0xA3, 0xF3, 0x93,
    0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E,
};
const uint8_t  UARTServiceRXCharacteristicUUID[UUID::LENGTH_OF_LONG_UUID] = {
    0x6E, 0x40, (uint8_t)(UARTServiceRXCharacteristicShortUUID >> 8), (uint8_t)(UARTServiceRXCharacteristicShortUUID & 0xFF), 0xB5, 0xA3, 0xF3, 0x93,
    0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E,
};

UARTService::UARTService():
    connection_count(0),
    send_countdown(0),
    sending(false),
    receiveBuffer(),
    sendBuffer(),
    txCharacteristic(UARTServiceTXCharacteristicUUID, receiveBuffer, 1, BLE_UART_SERVICE_MAX_DATA_LEN,
                     GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE | GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE_WITHOUT_RESPONSE),
    rxCharacteristic(UARTServiceRXCharacteristicUUID, sendBuffer, 1, BLE_UART_SERVICE_MAX_DATA_LEN, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),
    rxCCCDHandle(0),
    _uart_service(UARTServiceUUID,
                _uart_characteristics,
                sizeof(_uart_characteristics)/
                sizeof(_uart_characteristics[0])),
   _server(nullptr) {
    _uart_characteristics[0] = &txCharacteristic;
    _uart_characteristics[1] = &rxCharacteristic;
}

void UARTService::start(BLE &ble_interface)
{
    _server = &ble_interface.gattServer();
    _server->addService(_uart_service);

    // Cache the CCCD handle
    for(int i = 0; i < rxCharacteristic.getDescriptorCount(); i++) {
        GattAttribute* desc = rxCharacteristic.getDescriptor(i);
        if(desc->getUUID() == UUID(0x2902)) {
            rxCCCDHandle = desc->getHandle();
            tr_info("uart service cccd handle: %u",
                    rxCCCDHandle);
        }
    }
}

ssize_t UARTService::write(const void *_buffer, size_t length)
{
    mutex.lock();
    ssize_t result = length;
    const char *buffer = static_cast<const char*>(_buffer);
    if (connection_count) {

        TransceivedString* tx_string = new TransceivedString(buffer,
                length);
        tx_string->set_global(true);

        /**
         * If there was a problem inserting the message into the queue,
         * make sure to cleanup the dynamically allocated string object
         */
        if (!txQueue.try_put_for(DURATION_ZERO, tx_string)) {
            delete tx_string;
            tr_warning("failed to insert string into tx queue, dropping data");
            result = 0;
        }

        // Start processing the tx queue if not already
        if(!sending) {
            _write();
        }

    }
    mutex.unlock();
    return result;
}

ssize_t UARTService::read(void *buffer, size_t size)
{
    mutex.lock();

    // TODO read

    mutex.unlock();

    return 0; // TODO read
}

void UARTService::sigio(mbed::Callback<void()> func)
{
    _sigio_cb = func;
}

void UARTService::onDataWritten(const GattWriteCallbackParams &params)
{
    mutex.lock();
    if(params.handle == txCharacteristic.getValueHandle()) {
        // Allocate a string to store the incoming data
        TransceivedString* rx_string = new TransceivedString((const char*)params.data, params.len,
                params.connHandle);

        /**
         * If there was a problem inserting the message into the queue,
         * make sure to cleanup the dynamically allocated string object
         */
        if(!rxQueue.try_put_for(DURATION_ZERO, rx_string)) {
            delete rx_string;
            tr_warning("failed to insert string into rx queue, dropping data");
        } else {
            if(_sigio_cb != nullptr) {
                _sigio_cb();
            }
        }
    }
    mutex.unlock();
}

void UARTService::onDataSent(const GattDataSentCallbackParams &params)
{
    mutex.lock();

    if(params.attHandle == rxCharacteristic.getValueHandle()) {
        tr_debug("rx characteristic data sent to connection handle %u", params.connHandle);

        send_countdown--;
        if(send_countdown == 0) {

            if(_sigio_cb != nullptr) {
                _sigio_cb();
            }

            // Process next packet in tx Queue
            if(_write()) {
               // If the queue is empty, unflag sending
                sending = false;
            }

        }
    }

    mutex.unlock();
}

void UARTService::onUpdatesEnabled(
        const GattUpdatesEnabledCallbackParams &params) {
    if(params.attHandle == rxCCCDHandle) {
        if(_updates_changed_cb) {
            _updates_changed_cb(params.connHandle, true);
        }
        tr_debug("updates enabled for ble uart on connection handle %u", params.connHandle);
    }
}

void UARTService::onUpdatesDisabled(
        const GattUpdatesDisabledCallbackParams &params) {
    if(params.attHandle == rxCCCDHandle) {
        if(_updates_changed_cb) {
            _updates_changed_cb(params.connHandle, false);
        }
        tr_debug("updates disabled for ble uart on connection handle %u", params.connHandle);
    }
}

bool UARTService::_write(void)
{
    // Delete the previous message we sent if there is one
    delete next;
    next = nullptr;

    if(txQueue.try_get_for(DURATION_ZERO, &next)) {
        if(next->is_global()) {
            send_countdown = connection_count;
            _server->write(rxCharacteristic.getValueHandle(),
                    (const uint8_t*)next->get_string().c_str(),
                    next->get_string().length());
        } else {
            send_countdown = 1;
            _server->write(next->get_connection_handle(),
                    rxCharacteristic.getValueHandle(),
                    (const uint8_t*)next->get_string().c_str(),
                    next->get_string().length());
        }
        return false;
    } else {
        return true;
    }
}

void UARTService::onConnectionComplete(
        const ble::ConnectionCompleteEvent& event) {
    connection_count++;
    tr_debug("connection count(+): %u", connection_count);
}

void UARTService::onDisconnectionComplete(
        const ble::DisconnectionCompleteEvent& event) {
    connection_count--;
    tr_debug("connection count(-): %u", connection_count);
}

#endif // BLE_FEATURE_GATT_SERVER
