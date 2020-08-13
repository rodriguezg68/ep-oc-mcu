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

#ifndef EP_OC_MCU_EXTENSIONS_ASYNCHSERIAL_ASYNCHSERIAL_H_
#define EP_OC_MCU_EXTENSIONS_ASYNCHSERIAL_ASYNCHSERIAL_H_


#include "platform/platform.h"

#if (DEVICE_SERIAL_ASYNCH)

#ifndef MBED_CONF_ASYNCH_SERIAL_RXBUF_SIZE
#define MBED_CONF_ASYNCH_SERIAL_RXBUF_SIZE 256
#endif

#include "drivers/SerialBase.h"
#include "platform/FileHandle.h"
#include "platform/NonCopyable.h"
#include "platform/PlatformMutex.h"
#include "platform/Callback.h"

class AsynchSerial :
        private mbed::SerialBase,
        public mbed::FileHandle,
        private mbed::NonCopyable<AsynchSerial> {

public:

    /**
     * Creates an AsynchSerial port, connected to the specified transmit and receive pins,
     * with a particular baud rate
     * @param tx Transmit pin
     * @param rx Receive pin
     * @param baud The baud rate of the serial port (optional, defaults to
     *             MBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE)
     */
    AsynchSerial(PinName tx, PinName rx,
            int baud = MBED_CONF_PLATFORM_DEFAULT_SERIAL_BAUD_RATE);

    ~AsynchSerial() override;

    /** Equivalent to POSIX poll(). Derived from FileHandle.
     *  Provides a mechanism to multiplex input/output over a set of file
     *  handles.
     *  The events that can be reported are POLLIN, POLLOUT, POLLHUP.
     */
    short poll(short events) const final;

    /* Resolve ambiguities versus our private SerialBase
     * (for writable, spelling differs, but just in case)
     */
    using FileHandle::readable;
    using FileHandle::writable;

    /** Write the contents of a buffer to a file
     *
     *  Follows POSIX semantics:
     *
     * * if blocking, block until all data is written
     * * if no data can be written, and non-blocking set, return -EAGAIN
     * * if some data can be written, and non-blocking set, write partial
     *
     *  @param buffer   The buffer to write from
     *  @param length   The number of bytes to write
     *  @return         The number of bytes written, negative error on failure
     */
    ssize_t write(const void *buffer, size_t length) override;

    /** Read the contents of a file into a buffer
     *
     *  Follows POSIX semantics:
     *
     *  * if no data is available, and non-blocking set return -EAGAIN
     *  * if no data is available, and blocking set, wait until data is
     *    available
     *  * If any data is available, call returns immediately
     *
     *  @param buffer   The buffer to read in to
     *  @param length   The number of bytes to read
     *  @return         The number of bytes read, 0 at end of file, negative
     *                  error on failure
     */
    ssize_t read(void *buffer, size_t length) override;

    /** Close a file
     *
     *  @return         0 on success, negative error code on failure
     */
    int close() override { return 0; }

    /** Check if the file in an interactive terminal device
     *
     *  @return         True if the file is a terminal
     *  @return         False if the file is not a terminal
     *  @return         Negative error code on failure
     */
    int isatty() override { return 1; }

    /** Move the file position to a given offset from from a given location
     *
     * Not valid for a device type FileHandle like BufferedSerial.
     * In case of BufferedSerial, returns ESPIPE
     *
     *  @param offset   The offset from whence to move to
     *  @param whence   The start of where to seek
     *      SEEK_SET to start from beginning of file,
     *      SEEK_CUR to start from current position in file,
     *      SEEK_END to start from end of file
     *  @return         The new offset of the file, negative error code on
     *                  failure
     */
    off_t seek(off_t offset, int whence) override { return -ESPIPE; }

    /** Flush any buffers associated with the file
     *
     *  @return         0 on success, negative error code on failure
     */
    int sync() override;

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

    /** Enable or disable input
     *
     * Control enabling of device for input. This is primarily intended
     * for temporary power-saving; the overall ability of the device to operate
     * for input and/or output may be fixed at creation time, but this call can
     * allow input to be temporarily disabled to permit power saving without
     * losing device state.
     *
     *  @param enabled      true to enable input, false to disable.
     *
     *  @return             0 on success
     *  @return             Negative error code on failure
     */
    int enable_input(bool enabled) override;

    /** Enable or disable output
     *
     * Control enabling of device for output. This is primarily intended
     * for temporary power-saving; the overall ability of the device to operate
     * for input and/or output may be fixed at creation time, but this call can
     * allow output to be temporarily disabled to permit power saving without
     * losing device state.
     *
     *  @param enabled      true to enable output, false to disable.
     *
     *  @return             0 on success
     *  @return             Negative error code on failure
     */
    int enable_output(bool enabled) override;

    /** Register a callback on state change of the file.
     *
     *  The specified callback will be called on state changes such as when
     *  the file can be written to or read from.
     *
     *  The callback may be called in an interrupt context and should not
     *  perform expensive operations.
     *
     *  Note! This is not intended as an attach-like asynchronous api, but
     *  rather as a building block for constructing  such functionality.
     *
     *  The exact timing of when the registered function
     *  is called is not guaranteed and susceptible to change. It should be
     *  used as a cue to make read/write/poll calls to find the current state.
     *
     *  @param func     Function to call on state change
     */
    void sigio(mbed::Callback<void()> func) override;

    /** Set the baud rate
     *
     *  @param baud   The baud rate
     */
    void set_baud(int baud);

    // Expose private SerialBase::Parity as BufferedSerial::Parity
    using SerialBase::Parity;
    using SerialBase::None;
    using SerialBase::Odd;
    using SerialBase::Even;
    using SerialBase::Forced1;
    using SerialBase::Forced0;

    /** Set the transmission format used by the serial port
     *
     *  @param bits The number of bits in a word (5-8; default = 8)
     *  @param parity The parity used (None, Odd, Even, Forced1, Forced0;
     *                default = None)
     *  @param stop_bits The number of stop bits (1 or 2; default = 1)
     */
    void set_format(
        int bits = 8, Parity parity = AsynchSerial::None, int stop_bits = 1
    );

#if DEVICE_SERIAL_FC
    // For now use the base enum - but in future we may have extra options
    // such as XON/XOFF or manual GPIO RTSCTS.
    using SerialBase::Flow;
    using SerialBase::Disabled;
    using SerialBase::RTS;
    using SerialBase::CTS;
    using SerialBase::RTSCTS;

    /** Set the flow control type on the serial port
     *
     *  @param type the flow control type (Disabled, RTS, CTS, RTSCTS)
     *  @param flow1 the first flow control pin (RTS for RTS or RTSCTS, CTS for
     *               CTS)
     *  @param flow2 the second flow control pin (CTS for RTSCTS)
     */
    void set_flow_control(Flow type, PinName flow1 = NC, PinName flow2 = NC);
#endif

protected:

    inline void api_lock() { _mutex.lock(); }
    inline void api_unlock() { _mutex.unlock(); }

    inline bool rx_buf_is_empty(void) const {
        /* If an RX operation is ongoing return true
         * If an RX operation is not ongoing and buffer length is 0 return true */
        return (_rx_ongoing || (!_rx_ongoing && (_rxlen == 0)));
    }

    void tx_irq(int event);
    void rx_irq(int event);

protected:

    /** Software serial buffer
     *  By default buffer size is 256 for RX. Configurable
     *  through mbed_app.json
     */
    char _rxbuf[MBED_CONF_ASYNCH_SERIAL_RXBUF_SIZE];

    /** Current length of RX buffer, not valid unless _rx_ongoing is false */
    size_t _rxlen = 0;

    /** Flags if asynchronous RX operation is ongoing */
    volatile bool _rx_ongoing = false;

    /** Flags if asynchronous TX operation is ongoing */
    volatile bool _tx_ongoing = false;

    PlatformMutex _mutex;

    mbed::Callback<void()> _sigio_cb;

    bool _blocking = true;

};

#endif // DEVICE_SERIAL_ASYNCH

#endif /* EP_OC_MCU_EXTENSIONS_ASYNCHSERIAL_ASYNCHSERIAL_H_ */
