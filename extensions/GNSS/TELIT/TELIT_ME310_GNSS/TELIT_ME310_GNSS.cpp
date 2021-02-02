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

// Only compile this if the config is enabled
#if TELIT_ME310_GNSS_ENABLED

#include "TELIT_ME310_GNSS.h"

using namespace ep;
using namespace std::chrono_literals;

TELIT_ME310_GNSS::TELIT_ME310_GNSS(ATHandler *_at_handler) :
    at_handler(_at_handler)
{
    
}

#endif /* TELIT_ME310_GNSS_ENABLED */