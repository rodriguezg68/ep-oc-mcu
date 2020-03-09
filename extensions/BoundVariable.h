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
#ifndef DES0569_BSP_EP_CORE_EP_OC_MCU_EXTENSIONS_BOUNDVARIABLE_H_
#define DES0569_BSP_EP_CORE_EP_OC_MCU_EXTENSIONS_BOUNDVARIABLE_H_

#include "extensions/CallChain.h"

namespace ep {
/**
 * A BoundVariable is a variable that, when modified,
 * executes a callchain of handlers to notify interested parties
 * of the change.
 */
template<typename T>
class BoundVariable {
public:

    /** Empty constructor */
    BoundVariable() {
    }

    /** Initialize constructor */
    BoundVariable(const T& value) :
            _value(value) {
    }

    T &operator=(const T &rhs) {
        this->set(rhs);
        return _value;
    }

    operator T() {
        return _value;
    }

    T get(void) {
        return _value;
    }

    void set(T new_value) {
        _value = new_value;
        _callchain.call(_value);
    }

    void attach(const mbed::Callback<void(T)>& cb) {
        _callchain.attach(cb);
    }

    void detach(const mbed::Callback<void(T)>& cb) {
        _callchain.detach(cb);
    }

protected:

    T _value;
    ep::CallChain<T> _callchain;

};

}

#endif /* DES0569_BSP_EP_CORE_EP_OC_MCU_EXTENSIONS_BOUNDVARIABLE_H_ */
