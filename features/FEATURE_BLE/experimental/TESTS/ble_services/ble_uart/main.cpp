/*
 * Copyright (c) 2020 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

#if 1

#include "rtos/Thread.h"
#include "rtos/ThisThread.h"
#include "rtos/EventFlags.h"

#include "BleSerialTestServer.h"

#include "greentea-client/test_env.h"
#include "unity/unity.h"

#include <chrono>

#define EVENT_FLAGS_BLE_READY 0x0001

using namespace std::chrono;
using namespace utest::v1;

static rtos::EventFlags flags;
static BleSerialTestServer test_srv;

static control_t echo_single_connection(const size_t call_count)
{

    // Send callback and parameter to the host runner
    greentea_send_kv("echo_single_connection_blocking",
            test_srv.get_mac_addr());

    // wait until we get a message back
    // if this takes too long, the timeout will trigger, so no need to handle this here
    char _key[20], _value[128];
    while (1) {
        greentea_parse_kv(_key, _value, sizeof(_key), sizeof(_value));

        // check if the key equals init, and if the return value is 'world'
        if (strcmp(_key, "init") == 0) {
            TEST_ASSERT_EQUAL(0, strcmp(_value, "world"));
            break;
        }
    }

    return CaseNext;
}

static void ble_thread_main(void) {
    // This should never return
    test_srv.start();
}

static void on_ble_ready(void) {
    flags.set(EVENT_FLAGS_BLE_READY);
}

utest::v1::status_t greentea_setup(const size_t number_of_cases)
{
    // Setup BleSerialTestServer
    test_srv.on_ready(on_ble_ready);

    // Spin off BLE thread
    rtos::Thread ble_thread(osPriorityNormal, MBED_CONF_RTOS_THREAD_STACK_SIZE,
            NULL, "ble-thread");

    ble_thread.start(ble_thread_main);

    GREENTEA_SETUP(60, "ble_serial_test");

    // Wait for BLE system to be ready
    //uint32_t result = flags.wait_all_for(EVENT_FLAGS_BLE_READY, 10s);

    // Make sure we didn't timeout waiting for the BLE system
    //TEST_ASSERT(!(result & osFlagsError));

    return greentea_test_setup_handler(number_of_cases);
}

Case cases[] = {
    Case("Echo single connection blocking", echo_single_connection),
//    Case("Echo multi connection blocking", echo_multi_connection),
//    Case("Echo single connection non-blocking", echo_non_blocking),
//    Case("Echo multi connection non-blocking", echo_non_blocking),
//    Case("Disconnect while reading", dc_while_reading),
//    Case("Disconnect while writing", dc_while_writing),
//    Case("Memory leak", memory_leak)
};

Specification specification(greentea_setup, cases);

int main()
{
    return !Harness::run(specification);
}


#endif
