/**
 * ep-oc-mcu
 * Embedded Planet Open Core for Microcontrollers
 * 
 * Built with ARM Mbed-OS
 *
 * Copyright (c) 2021 Embedded Planet, Inc.
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

#ifndef EP_OC_MCU_EXTENSIONS_TELIT_ME310_GNSS_H_
#define EP_OC_MCU_EXTENSIONS_TELIT_ME310_GNSS_H_

// Only compile this if the config is enabled
#if TELIT_ME310_GNSS_ENABLED

#include "GNSS.h"
#include "ATHandler.h"

namespace ep
{

/**
 * Logically abstraction of the Telit ME310 GNSS controller
 */
class TELIT_ME310_GNSS : public GNSS  {
public:
    /**
     * Default constructor which
     * 
     * @param _at_handler   Pointer to an ATHandler
     */
    TELIT_ME310_GNSS(ATHandler *_at_handler);

private:
    ATHandler *at_handler;    
};

}

#endif /* TELIT_ME310_GNSS_ENABLED */

#endif /* EP_OC_MCU_EXTENSIONS_TELIT_ME310_GNSS_H_ */