/*
 * NCV7751.cpp
 *
 *  Created on: Apr 24, 2020
 *      Author: gdbeckstein
 */

#include "NCV7751.h"

#include "mbed_assert.h"

using namespace ep;

NCV7751::NCV7751(mbed::SPI& spi, PinName csb1, PinName csb2, PinName global_en) :
        _spi(spi), _csb1(csb1, 1), _csb2(csb2, 1), _global_en(nullptr), _channel_bits(
                0), _ol_bits(0), _cached_diag(0) {
    // Instantiate optional control outputs
    if (global_en != NC) {
        _global_en = new mbed::DigitalOut(global_en, 0);
    }
}

NCV7751::~NCV7751(void) {
    // Clean up any dynamically allocated members
    if (_global_en != nullptr) {
        delete _global_en;
    }
}

void NCV7751::enable(void) {
    if (_global_en != nullptr) {
        *_global_en = 1;
    }

    // Turn off all channels initially
    write_state(0);
}

void NCV7751::disable(void) {
    if (_global_en != nullptr) {
        *_global_en = 0;
    }
}

NCV7751::ChannelOut NCV7751::channel(int num) {
    return ChannelOut(*this, num);
}

uint32_t NCV7751::write_state(uint16_t channel_bits, uint16_t ol_bits) {

    _mutex.lock();

    _channel_bits = channel_bits;
    _ol_bits = ol_bits;

    // Format the channel/ol bits to NCV7751 format
    uint16_t si_port1 = 0, si_port2 = 0;
    for(int i = 0; i < 16; i++) {

        // Select the port this channel is controlled by
        uint16_t* port = (i <= 7? &si_port1 : &si_port2);

        /**
         * If the channel is set to on, ignore the ol bits
         * Open-load diagnostics are only enabled when channel is off
         */
        if(_channel_bits & (1 << i)) {
            // Set the channel to ON mode (0b10)
            port |= (1 << ((i << 1) + 1)); // 1 << ((i * 2) + 1)
            port &= ~(1 << (i << 1));
        } else if(_ol_bits & (1 << i)) {
            // Check if open-load diagnostics are enabled
            // If so, set channel to OFF mode (0b11)
            port |= (0b11 << (i << 1)); // i * 2
        } else {
            // Neither channel nor ol diag are enabled
            // Set channel to STANDBY mode (0b00)
            port &= ~(0b11 << (i << 1));
        }

    }

    /**
     * Access Channels 1 thru 8
     * CSB Mode = 0b10
     */
    _csb1 = 1;
    _csb2 = 0;
    _cached_diag = (_spi.write(si_port1) & 0xFFFF);
    /**
     * Access Channels 9 thru 12
     * CSB Mode = 0b01_T
     */
    _csb2 = 1;
    _csb1 = 0;
    // T = truncated (16-bit vs 24-bit), internal register setting
    _cached_diag |= (((_spi.write(si_port2)) & 0xFF00) << 12);

    /** Deassert */
    _csb1 = 1;

    _mutex.unlock();

    return _cached_diag;

}

uint32_t NCV7751::sync(void) {
    return write_state(_channel_bits, _ol_bits);
}

NCV7751::ChannelOut::ChannelOut(NCV7751& ncv7751, int channel_num) :
        _parent(ncv7751), _num(channel_num - 1) {
    // Only channels 1 through 12 are supported
    MBED_ASSERT((0 < channel_num) && (channel_num <= 12));
}

void NCV7751::ChannelOut::write(int value) {

    _parent._mutex.lock();

    // Get the cached channel states
    uint16_t new_channel_bits = _parent.get_channel_bits();

    // Set or clear the channel as indicated by value
    if (value) {
        // Set the channel on bit
        new_channel_bits |= (1 << _num);
    } else {
        // Clear the channel on bit
        new_channel_bits &= ~(1 << _num);
    }

    // Write out the value
    _parent.write_state(new_channel_bits, _parent.get_ol_bits());

    _parent._mutex.unlock();
}

int NCV7751::ChannelOut::read() {
    return (_parent.get_channel_bits() & (1 << _num));
}

NCV7751::fault_condition_t NCV7751::ChannelOut::get_fault(void) {

    // Lock the device mutex
    _parent._mutex.lock();

    // Sync diagnostic bits
    uint32_t diag_bits = _parent.sync();

    // Unlock the device mutex
    _parent._mutex.unlock();

    // First see if there's a fault reported on this channel
    return NO_FAULT;

}

void NCV7751::ChannelOut::enable_open_load_diag(void) {
    uint16_t new_ol_bits = _parent.get_ol_bits();
    new_ol_bits |= (1 << _num);

    _parent.write_state(_parent.get_channel_bits(), new_ol_bits);
}

void NCV7751::ChannelOut::disable_open_load_diag(void) {
    uint16_t new_ol_bits = _parent.get_ol_bits();
    new_ol_bits &= ~(1 << _num);

    _parent.write_state(_parent.get_channel_bits(), new_ol_bits);
}

bool NCV7751::ChannelOut::open_load_diag_enabled(void) {
    return (_parent.get_ol_bits() & (1 << _num));
}
