
from typing import List, Optional, Any, Mapping

from mbed_host_tests import BaseHostTest
from mbed_host_tests.host_tests_logger import HtrunLogger

from mbed_ble_test_suite.common import fixtures
from mbed_ble_test_suite.common.ble_device import BleDevice

import json

import time

# UART Service UUID
uart_svc_uuid = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
# UART TX Characteristic UUID
uart_tx_uuid = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
# UART RX Characteristic UUID
uart_rx_uuid = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

# Cached handles (GattServer under test should not change after first run)
cached_rx_handle = None
cached_tx_handle = None
cached_rx_cccd_handle = None

address_type_map = {0: "PUBLIC", 1: "RANDOM", 2: "PUBLIC_IDENTITY", 3: "RANDOM_STATIC_IDENTITY", 0xFF: "ANONYMOUS"}


class UARTSerialAllocator(fixtures.BoardAllocator):

    def __init__(self, ht : BaseHostTest, platforms_supported: List[str], binaries: Mapping[str, str],
                 serial_inter_byte_delay: float, baudrate: int, command_delay: float):

        # Call the BoardAllocator init
        super(UARTSerialAllocator, self).__init__(platforms_supported, binaries, serial_inter_byte_delay, baudrate,
                                                  command_delay)

        self.port = ht.get_config_item("port")

        # Remove the serial port allocated to the host test
        self.allocation = [x for x in self.allocation if self.port not in x.description['serial_port']]


class BleSerialClient:

    def __init__(self, ble_dev):
        self.ble_dev = ble_dev  # type: BleDevice
        self.connection_info = None
        self.default_mtu = 20
        self.mtu = self.default_mtu

        self.logger = HtrunLogger('TEST')

    def discover_uart_handles(self):

        global cached_rx_handle, cached_tx_handle, cached_rx_cccd_handle

        r = self.ble_dev.gattClient.discoverAllServicesAndCharacteristics(
            self.connection_info["connection_handle"])

        if not r.success():
            self.logger.prn_err("failed to discover services and characteristics")
            return False

        for service in r.result:
            self.logger.prn_inf("service uuid: {}".format(service["UUID"]))
            # Found UART service
            if uart_svc_uuid in str(service["UUID"]):  # some of the uuids (short ones) are ints instead of strings
                for c in service['characteristics']:
                    if uart_rx_uuid in str(c['UUID']):
                        cached_rx_handle = c
                    elif uart_tx_uuid in str(c['UUID']):
                        cached_tx_handle = c

        if not cached_rx_handle:
            self.logger.prn_err("failed to discover uart rx characteristic")
            return False
        if not cached_tx_handle:
            self.logger.prn_err("failed to discover uart tx characteristic")
            return False

        # Discover RX char descriptors
        r = self.ble_dev.gattClient.discoverAllCharacteristicsDescriptors(
            self.connection_info['connection_handle'],
            cached_rx_handle['start_handle'],
            cached_rx_handle['end_handle'])

        if not r.success():
            self.logger.prn_err("failed to discover uart rx descriptors")
            return False

        for desc in r.result:
            try:
                if desc['UUID'] == 0x2902:  # CCCD UUID
                    cached_rx_cccd_handle = desc
                    break

            except TypeError:  # ignore if the UUID is a string we're looking for a short UUID
                continue
                
        if not cached_rx_cccd_handle:
            self.logger.prn_err("failed to discover uart rx CCCD")
            return False

        # Okay we're connected and have discovered what we need to
        return True

    def connect_to(self, mac, addr_type):

        global cached_rx_handle, cached_tx_handle, cached_rx_cccd_handle

        # Initialize BLE
        if not self.ble_dev.ble.init().success():
            return False

        self.logger.prn_inf("ble initialized")

        # Connect to given device MAC
        r = self.ble_dev.gap.connect(addr_type, mac)
        if not r.success():
            return False

        self.logger.prn_inf("connection to {} succeeded".format(mac))

        self.connection_info = r.result

        # Get the UART RX/TX handles and TX CCCD handle if not already cached
        if not cached_rx_handle:
            r = self.discover_uart_handles()
            if not r:
                return False

    def subscribe(self):
        # Write the CCCD to 01 to subscribe to notifications
        r = self.ble_dev.gattClient.writeCharacteristicDescriptor(
            self.connection_info["connection_handle"],
            cached_rx_cccd_handle["handle"], "0100"
        )

        if not r.success():
            self.logger.prn_err("could not write rx char CCCD")
            return False

        # Now listen for HVX
        r = self.ble_dev.gattClient.enableUnsolicitedHVX(1)
        if not r.success():
            self.logger.prn_err("Could not enable unsolicited HVX")
            return False

        return True

    def unsubscribe(self):

        # Disable HVX
        r = self.ble_dev.gattClient.enableUnsolicitedHVX(0)
        if not r.success():
            self.logger.prn_err("Could not disable unsolicited HVX")
            return False

        # Write the CCCD back to 00
        r = self.ble_dev.gattClient.writeCharacteristicDescriptor(
            self.connection_info["connection_handle"],
            cached_rx_cccd_handle["handle"], "0000"
        )
        if not r.success():
            self.logger.prn_err("could not write rx char CCCD")
            return False

        return True

    def update_mtu(self, mtu):
        if self.mtu != self.default_mtu:
            self.logger.prn_err("cannot update MTU twice in one connection")
            return False

        self.mtu = mtu

        # TODO - set the MTU of the connection

    def echo_test(self, data: bytearray):

        data_str = "".join("{:02x}".format(x) for x in data)

        # Write the byte array to device
        # Format to hex string ([0x01, 0x02, ...] = "0102...")
        r = self.ble_dev.gattClient.writeWithoutResponse(
            self.connection_info["connection_handle"],
            cached_tx_handle["value_handle"],
            data_str
        )

        if not r.success():
            self.logger.prn_err("could not write to tx char")
            return False

        # Wait for an HVX
        self.ble_dev.wait_for_output("<<<", timeout=10, assert_timeout=False)
        event = self.ble_dev.events.get(block=True, timeout=10)

        d = json.loads(event)
        if 'BLE_HVX_NOTIFICATION' in d['type']:
            if self.connection_info['connection_handle'] == d['connHandle']:
                if cached_rx_handle['value_handle'] == d['handle']:
                    # We received data, make sure it matches what we sent
                    self.logger.prn_inf("received data: {}".format(d['data']))
                    if data_str == d['data']:
                        return True

        return False

    def reset(self):
        pass


class BleSerialTest(BaseHostTest):

    def _callback_echo_single_connection_blocking(self, key, value, timestamp):

        # Get MAC address and address type from value
        print(value.split(","))
        mac = value.split(",")[0]
        addr_type = address_type_map[int(value.split(",")[1])]

        self.logger.prn_inf('Received MAC Addr: %s' % value)

        # Connect with a single client and perform the echo test
        client = BleSerialClient(self.allocator.allocate())

        if not client.connect_to(mac, addr_type):
            self.notify_complete(False)

        # Subscribe to RX characteristic
        client.subscribe()

        # Run the echo test
        result = client.echo_test(bytearray([1, 2, 3, 4, 5]))

        # Unsubscribe from client and reset
        client.unsubscribe()
        client.reset()

        # Release client
        self.allocator.release(client.ble_dev)

        self.notify_complete(result)

    def setup(self):

        # Set up board allocator
        self.allocator = UARTSerialAllocator(self, ["NRF52_DK"],
                                             {"NRF52_DK": None},
                                             0, 115200, 0)

        # Register callbacks
        self.register_callback('echo_single_connection_blocking',
                               self._callback_echo_single_connection_blocking)

    def result(self):
        pass

    def teardown(self):
        pass

    def __init__(self):
        super(BleSerialTest, self).__init__()

        self.logger = HtrunLogger('TEST')
        self.allocator = None


if __name__ == '__main__':
    test = BleSerialTest()
    test._callback_echo_single_connection_blocking(None, "eb:59:b8:48:1d:4d", 0)