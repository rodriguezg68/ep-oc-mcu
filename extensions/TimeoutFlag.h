/*
 * Mbed-OS Microcontroller Library
 * Copyright (c) 2021 Embedded Planet
 * Copyright (c) 2021 ARM Limited
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
 * limitations under the License
 */

#ifndef EP_OC_MCU_EXTENSIONS_TIMEOUTFLAG_H_
#define EP_OC_MCU_EXTENSIONS_TIMEOUTFLAG_H_

#include "drivers/Timeout.h"
#include "platform/Callback.h"

#include <chrono>

/* TODO extend this so that it supports both volatile bool and rtos flags. Use templating and create generic flag API + default implementations */

/** Class encapsulating a flag that is set/reset by an IRQ timeout */
class TimeoutFlag
{
public:

    TimeoutFlag() {
    }

    /**
     * Start the timeout. If the timeout expires before being reset, the internal
     * flag will be set to true. Calling this before the timeout occurs resets the timeout with
     * the new timeout duration
     * @param[in] timeout Chrono duration of timeout
     */
    void start(std::chrono::microseconds timeout) {
        _flag = false;
        _timeout.detach();
        _timeout.attach(mbed::callback(this, &TimeoutFlag::on_timeout), timeout);
    }

    /**
     * Stop the timeout. The internal flag will be unchanged if the timeout has not yet occurred
     */
    void stop() {
        _timeout.detach();
    }

    /**
     * Returns true if the internal flag is set. ie: the timeout has occurred.
     */
    bool is_set() const {
        return _flag;
    }

protected:

    void on_timeout() {
        _flag = true;
    }

protected:

    volatile bool _flag = false;
    mbed::Timeout _timeout;

};



#endif /* EP_OC_MCU_EXTENSIONS_TIMEOUTFLAG_H_ */
