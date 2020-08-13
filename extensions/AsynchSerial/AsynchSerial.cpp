/**
 * ep-oc-mcu
 * Embedded Planet Open Core for Microcontrollers
 *
 * Built with ARM Mbed-OS
 *
 * Copyright (c) 2019-2020 Embedded Planet, Inc.
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

#include "AsynchSerial.h"

#include "platform/mbed_critical.h"
#include "rtos/ThisThread.h"
#include <chrono>

using namespace std::chrono;

AsynchSerial::AsynchSerial(PinName tx, PinName rx, int baud) : SerialBase(tx, rx, baud) {
}

AsynchSerial::~AsynchSerial() {
}

short AsynchSerial::poll(short events) const {

    short revents = 0;
    // Check the Circular Buffer if space available for writing out

    if (!rx_buf_is_empty()) {
        revents |= POLLIN;
    }

    if(!_tx_ongoing) {
        revents |= POLLOUT;
    }

    return revents;
}

ssize_t AsynchSerial::write(const void* buffer, size_t length) {

    while(true) {
        int retval = SerialBase::write((const uint8_t*) buffer,
                length, mbed::callback(this, &AsynchSerial::tx_irq));

        if(retval != 0) {
            // Failed to initiate tx
            if(!_blocking) {
                // Not blocking - give up and return an error
                return -EAGAIN;
            } else {
                // Blocking - wait for a bit and try again
                rtos::ThisThread::sleep_for(1ms);
            }
        }
        else {
            // Success, break out of loop
            _tx_ongoing = true;
            break;
        }
    }

    // If blocking, wait until tx is complete
    if(_blocking) {
        while(_tx_ongoing) {
            rtos::ThisThread::sleep_for(1ms);
        }
    }

    return length;
}

ssize_t AsynchSerial::read(void* buffer, size_t length) {

    size_t data_read = 0;
    uint8_t* ptr = static_cast<uint8_t*>(buffer);

    if (length == 0) {
        return 0;
    }

    api_lock();

    // Attempt to read from the buffer
    if(!rx_buf_is_empty()) {

        // RX buffer has contents, read until it's empty or output buffer is full
        size_t min = length;
        if(_rxlen < min) {
            min = _rxlen;
        }

        while(data_read < min) {
            ptr[data_read] = _rxbuf[data_read];
            data_read++;
        }

        api_unlock();
        return data_read;
    }

    // RX buffer is empty, we must initiate an RX operation
    while(true) {
        int retval = SerialBase::read((uint8_t*) _rxbuf, length,
                mbed::callback(this, &AsynchSerial::rx_irq));

        if(retval != 0) {
            // Failed to initiate rx
            if(!_blocking) {
                // Not blocking - give up and return error
                api_unlock();
                return -EAGAIN;
            } else {
                // Blocking - delay and try again
                rtos::ThisThread::sleep_for(1ms);
            }
        } else {
            // Success, break out of loop
            _rxlen = length;
            _rx_ongoing = true;
            break;
        }
    }

    // If blocking, wait until RX transfer is complete
    if(_blocking) {
        while(_rx_ongoing) {
            rtos::ThisThread::sleep_for(1ms);
        }
        api_unlock();
        return _rxlen;
    }

    api_unlock();

    // If not blocking, tell the caller to try again later
    return -EAGAIN;

}

int AsynchSerial::sync() {
    api_lock();
    while(_tx_ongoing) {
        api_unlock();
        rtos::ThisThread::sleep_for(1ms);
        api_lock();
    }
    api_unlock();

    return 0;
}

int AsynchSerial::enable_input(bool enabled) {
    api_lock();
    SerialBase::enable_input(enabled);
    api_unlock();

    return 0;
}

int AsynchSerial::enable_output(bool enabled) {
    api_lock();
    SerialBase::enable_output(enabled);
    api_unlock();

    return 0;
}

void AsynchSerial::sigio(mbed::Callback<void()> func) {
    core_util_critical_section_enter();
    _sigio_cb = func;
    if (_sigio_cb) {
        short current_events = poll(0x7FFF);
        if (current_events) {
            _sigio_cb();
        }
    }
    core_util_critical_section_exit();
}

void AsynchSerial::set_baud(int baud) {
    SerialBase::baud(baud);
}

void AsynchSerial::set_format(int bits, Parity parity, int stop_bits) {
    api_lock();
    SerialBase::format(bits, parity, stop_bits);
    api_unlock();
}

#if DEVICE_SERIAL_FC
void AsynchSerial::set_flow_control(Flow type, PinName flow1, PinName flow2) {
    api_lock();
    SerialBase::set_flow_control(type, flow1, flow2);
    api_unlock();
}

void AsynchSerial::tx_irq(int event) {

    // Reset the _tx_ongoing flag
    _tx_ongoing = false;

    // Call the application sigio handler
    if(_sigio_cb) {
        _sigio_cb();
    }
}

void AsynchSerial::rx_irq(int event) {
    // Reset the _rx_ongoing flag
    _rx_ongoing = false;

    // Call the application sigio handler
    if(_sigio_cb) {
        _sigio_cb();
    }
}

#endif


