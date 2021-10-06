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

#ifndef EP_OC_MCU_EXTENSIONS_PERSISTENTVARIABLE_H_
#define EP_OC_MCU_EXTENSIONS_PERSISTENTVARIABLE_H_

#include "PersistentArray.h"

// TODO think about checking if the value is the same during ::set to prevent unnecessary writes to flash

namespace ep
{

/**
 * Special case of PersistentArray with only 1 value
 * Adds assignment and evaluation operators
 */
template<typename T>
class PersistentVariable : public PersistentArray<T, 1>
{

public:

    /** Initialize a persistent variable with a default value
     * @param[in] default_value The default value of this persistent variable
     * @param[in] key Key to store the persistent variable under
     * @param[in] flags Creation flags for kvstore API (eg: WRITE_ONCE_FLAG)
     *
     * @note This value is only used if the persistent variable has not
     * been accessed before or if the kvstore is unavailable for some reason
     */
    PersistentVariable(T default_value, const char* key, uint32_t flags = 0) :
        PersistentArray<T, 1>(default_value, key, flags) {
    }

    /** Destructor */
    ~PersistentVariable(void) {
    }

    /**
     * Assignment operator for updating the value stored in persistent
     * memory (if available)
     */
    T &operator= (const T &rhs) {
        this->set(rhs);
        return *this->_array;
    }

    /** Evaluation operator
     *
     * Reads underlying persistent memory and returns current value (or default)
     */
    operator T() {
        this->get();
        return *this->_array;
    }

    /**
     * Alias for PersistentArray::get that returns a simple type
     */
    T get() {
        PersistentArray<T, 1>::get();
        return *this->_array;
    }

    /**
     * Alias for PersistentArray::set that accepts a simple type
     */
    void set(T val) {
        PersistentArray<T, 1>::set(mbed::make_Span<1, T>(&val));
    }

};

}

#endif /* EP_OC_MCU_EXTENSIONS_PERSISTENTVARIABLE_H_ */
