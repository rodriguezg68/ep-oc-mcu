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

#ifndef EP_OC_MCU_EXTENSIONS_PERSISTENTARRAY_H_
#define EP_OC_MCU_EXTENSIONS_PERSISTENTARRAY_H_

#include "kvstore_global_api.h"
#include "platform/mbed_error.h"
#include "platform/mbed_assert.h"
#include "platform/Span.h"
#include "platform/mbed_critical.h"
#include "mbed-trace/mbed_trace.h"

#include <cstring>

#include <stdio.h>

namespace ep {

/**
 * Templatized Persistent Array built on top of Mbed's
 * KVStore API. If KVStore is not available the API will fall back to
 * non-volatile storage with a default initialized
 */
template<typename T, ptrdiff_t N>
class PersistentArray
{
public:

    /**
     * Initialize a persistent array with a default array of values
     * @param[in] default_array Array of default values to use
     * @param[in] key Key to use for kvstore
     *
     * @note This value is only used if the persistent variable has not
     * been accessed before or if the kvstore is unavailable for some reason
     */
    PersistentArray(mbed::Span<T,N> default_array, const char *key, uint32_t flags = 0) : _key(key), _flags(flags) {
        memcpy(_array, default_array.data(), N*sizeof(T));
    }

    /**
     * Initialize a persistent array with a default value
     * @param[in] default_value The default value of the array. Every array element will be set to this value.
     * @param[in] key Key to use for kvstore
     *
     * @note This value is only used if the persistent variable has not
     * been accessed before or if the kvstore is unavailable for some reason
     */
    PersistentArray(T default_value, const char *key, uint32_t flags = 0) : _key(key), _flags(flags) {
        memset(_array, default_value, N*sizeof(T));
    }

    /** Destructor */
    ~PersistentArray(void) {
    }

    /**
     * Attempts to get the underlying value from KVStore
     * @retval value Value obtained from KVStore of default if unavailable
     * @note Interrupt safe. A cached value will be returned if called from an interrupt.
     */
    mbed::Span<T,N> get(void)   {

        /* If we're in an ISR, just return the cached value */
        if(!core_util_is_isr_active()) {

            // Try to access the KVStore partition
            size_t actual_size;
            int err = kv_get(_key, _array, N*sizeof(T), &actual_size);

            /** If we weren't able to get the variable,
                attempt to set the default value in KVStore */
            if(err == MBED_ERROR_ITEM_NOT_FOUND) {

                mbed_tracef(TRACE_LEVEL_WARN, "PARR", "item \"%s\" not found in kvstore, attempting to set...", _key);

                this->set(mbed::make_Span(_array));

                // Now try to get the key... if this doesn't work we will return default
                err = kv_get(_key, _array, N*sizeof(T), &actual_size);
            }

            if(err) {
                mbed_tracef(TRACE_LEVEL_WARN, "PARR", "could not get item \"%s\" from kvstore: %d", _key, err);
            }

            if(actual_size != N*sizeof(T)) {
                mbed_tracef(TRACE_LEVEL_WARN, "PARR", "actual size (%u) of kvstore entry did not match expected size (%u)", actual_size, N*sizeof(T));
            }
        }

        return mbed::make_Span(_array);
    }

    /**
     * Attempts to set the underlying value in KVStore
     * @param[in] value Value to set
     *
     * @note Not interrupt safe
     */
    void set(mbed::Span<T,N> new_value)  {

        memcpy(_array, new_value.data(), new_value.size()*sizeof(T));

        /**
         * Try to access the KVStore partition
         * If this doesn't work value stays default
         */
        int err = kv_set(_key, _array, N*sizeof(T), _flags);

        if(err) {
            mbed_tracef(TRACE_LEVEL_WARN, "PARR", "could not set entry \"%s\": %d", _key, err);
        }
    }

    /**
     * Checks if the given PersistentVariable already exists in KVStore
     * @return true if exists, false if not
     * @note Not interrupt safe
     */
    bool exists()  {
        /* This API cannot be called from an ISR, assert here if so
         * Otherwise it will assert later on and be harder to find. */
        assert(!core_util_is_isr_active());
        size_t actual_size;
        int err = kv_get(_key, _array, sizeof(T), &actual_size);
        if(err) {
            // TODO this can also return false if a different error occurs other than "doesn't exist"
            return false;
        } else {
            return true;
        }
    }

protected:

    T _array[N];
    const char *_key;

    uint32_t _flags;
};

}

#endif /* EP_OC_MCU_EXTENSIONS_PERSISTENTARRAY_H_ */
