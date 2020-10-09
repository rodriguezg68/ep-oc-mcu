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

#if BLE_FEATURE_GATT_SERVER

#include "UARTService.h"

#include "mbed_trace.h"

#include "Kernel.h"

#include <algorithm>

#include "platform/mbed_assert.h"

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

void UARTService::onAttMtuChange(ble::connection_handle_t connectionHandle,
        uint16_t attMtuSize) {
    tr_debug("mtu changed to %u for connection handle %d", attMtuSize, connectionHandle);
    BleSerial* ser = get_ble_serial_handle(connectionHandle);
    if(ser) {
        ser->set_mtu(attMtuSize);
    }
}

void UARTService::onDataWritten(const GattWriteCallbackParams &params)
{
    if(params.handle == txCharacteristic.getValueHandle()) {
        BleSerial* ser = get_ble_serial_handle(params.connHandle);
        if(ser) {
            ser->on_data_written(mbed::make_const_Span(params.data, params.len));
        }
    }
}

void UARTService::onDataSent(const GattDataSentCallbackParams &params)
{
    if(params.attHandle == rxCharacteristic.getValueHandle()) {
        BleSerial* ser = get_ble_serial_handle(params.connHandle);
        if(ser) {
            ser->on_data_sent();
        }
    }
}

void UARTService::onUpdatesEnabled(
        const GattUpdatesEnabledCallbackParams &params) {
    if(params.attHandle == rxCCCDHandle) {
        BleSerial* ser = get_ble_serial_handle(params.connHandle);
        if(ser) {
            ser->on_updates_enabled();
        }
    }
}

void UARTService::onUpdatesDisabled(
        const GattUpdatesDisabledCallbackParams &params) {
    if(params.attHandle == rxCCCDHandle) {
        BleSerial* ser = get_ble_serial_handle(params.connHandle);
        if(ser) {
            ser->on_updates_disabled();
        }
    }
}

void UARTService::onConnectionComplete(
        const ble::ConnectionCompleteEvent& event) {
    // Create a new BleSerial handle for this connection
    BleSerial* ser = new BleSerial(*this, event.getConnectionHandle());
    _serial_handles.emplace_front(ser);

    tr_debug("serial handle (+): connection handle: %d", event.getConnectionHandle());
}

void UARTService::onDisconnectionComplete(
        const ble::DisconnectionCompleteEvent& event) {
    // Remove the disconnected serial handle and delete it
    BleSerial* ser = get_ble_serial_handle(event.getConnectionHandle());
    _serial_handles.remove(ser);

    delete ser;
    ser = nullptr;

    tr_debug("serial handle(-): connection handle: %d", event.getConnectionHandle());
}

UARTService::~UARTService() {
    delete_all_serial_handles();
}

void UARTService::onShutdown(const GattServer &server) {
    delete_all_serial_handles();
}

void UARTService::delete_all_serial_handles(void) {
    auto it = _serial_handles.begin();
    while(it != _serial_handles.end()) {
        // Delete all allocated serial handles
        delete *it;
        *it = nullptr;
        it++;
    }

    _serial_handles.clear();
}

UARTService::BleSerial* UARTService::get_ble_serial_handle(ble::connection_handle_t connection_handle) {

    auto iter =  std::find_if(_serial_handles.begin(),
            _serial_handles.end(),
            [connection_handle](BleSerial* ser) {
        return (ser->get_connection_handle() == connection_handle);
    });

    // Return nullptr if an associated BleSerial object wasn't found
    return (iter == _serial_handles.end()? nullptr : *iter);
}

UARTService::BleSerial::BleSerial(
        UARTService& service,
        ble::connection_handle_t connection_handle) :
                _service(service), _connection_handle(connection_handle),
                _mtu(BLE_UART_SERVICE_DEFAULT_MTU_SIZE) { }

UARTService::BleSerial::~BleSerial() {

    /** Deallocate TX buffer upon disconnection (destructor is called upon disconnection) */
    deallocate_tx_buffer();

}

ssize_t UARTService::BleSerial::write(const void *_buffer, size_t length)
{
    // Ignore if the client hasn't subscribed yet
    if(_txbuf == nullptr) {
        return 0;
    }

    mutex.lock();
    const uint8_t *buffer = static_cast<const uint8_t*>(_buffer);

    for(unsigned int i = 0; i < length; i++) {
        _txbuf->push(buffer[i]);
    }

    if(!_sending_data) {
        // Simulate a data sent event to start TX
        on_data_sent();
    }

    mutex.unlock();
    return length;
}

ssize_t UARTService::BleSerial::read(void *buffer, size_t size)
{

    size_t data_read = 0;

    uint8_t *ptr = static_cast<uint8_t *>(buffer);

    if (size == 0) {
        return 0;
    }

    mutex.lock();

    while(_rxbuf.empty()) {
        if(!_blocking) {
            return -EAGAIN;
        }
        mutex.unlock();
        //thread_sleep_for(1);
        mutex.lock();
    }

    while(data_read < size && !_rxbuf.empty()) {
        _rxbuf.pop(*ptr++);
        data_read++;
    }

    mutex.unlock();

    return data_read;
}

void UARTService::BleSerial::on_updates_enabled(void) {

    /** Allocate TX buffer now that someone is listening */
    _txbuf = new mbed::CircularBuffer<uint8_t, MBED_CONF_BLE_UART_SERVICE_TX_RX_BUFFER_SIZE>();

    /** Call application handler */
    if(_updates_changed_cb) {
        _updates_changed_cb(true);
    }
    tr_debug("updates enabled on connection handle %u", _connection_handle);
}

void UARTService::BleSerial::on_updates_disabled(void) {

    /** Deallocate TX buffer */
    deallocate_tx_buffer();

    /* Call application handler */
    if(_updates_changed_cb) {
        _updates_changed_cb(false);
    }
    tr_debug("updates disabled on connection handle %u", _connection_handle);
}

void UARTService::BleSerial::on_data_sent(void) {
    mutex.lock();

    bool was_full = _txbuf->full();

    // Delete the old gatt buffer and allocate a new one
    delete[] _gatt_tx_buf;
    _gatt_tx_buf = nullptr;

    // Check if the tx buffer is empty
    if(_txbuf->empty()) {
        // We're done sending
        _sending_data = false;
        mutex.unlock();
        return;
    }

    _gatt_tx_buf = new uint8_t[_mtu];

    MBED_ASSERT(_gatt_tx_buf != nullptr);

    int i = 0;
    while((i < _mtu) && (!_txbuf->empty())) {
        _txbuf->pop(_gatt_tx_buf[i++]);
    }

    // Transmit the buffer now
    ble_error_t err = _service.write(this, mbed::make_Span(_gatt_tx_buf, i));

    if(err) {
        tr_error("writing to connection %d failed: %u", _connection_handle, err);
    } else {
        tr_info("wrote %d bytes to connection %d", i, _connection_handle);
        _sending_data = true;
        if(was_full && !_txbuf->full()) {
            wake();
        }
    }

    mutex.unlock();
}

void UARTService::BleSerial::on_data_written(mbed::Span<const uint8_t> data) {

    mutex.lock();

    bool was_empty = _rxbuf.empty();

    for(int i = 0; i < data.size(); i++) {
        _rxbuf.push(data[i]);
    }

    // Report that data is ready to be read from the buffer
    if(was_empty && !_rxbuf.empty()) {
        wake();
    }

    mutex.unlock();
}

ble_error_t UARTService::write(BleSerial *ser, mbed::Span<const uint8_t> data) {

    return _server->write(ser->_connection_handle,
            rxCharacteristic.getValueHandle(),
            data.data(), data.size(), false);

}

#endif // BLE_FEATURE_GATT_SERVER
