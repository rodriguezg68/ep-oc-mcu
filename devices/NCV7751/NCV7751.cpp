/*
 * NCV7751.cpp
 *
 *  Created on: Apr 24, 2020
 *      Author: gdbeckstein
 */

#include "NCV7751.h"

#include "mbed_assert.h"

using namespace ep;

NCV7751::NCV7751(mbed::SPI& spi, PinName csb1, PinName csb2,
        PinName global_en) : _spi(spi), _csb1(csb1, 1),
                _csb2(csb2, 1), _global_en(nullptr),
                _cached_state(0), _cached_diag(0) {
    // Instantiate optional control outputs
    if(global_en != NC) {
        _global_en = new mbed::DigitalOut(global_en, 0);
    }
}

NCV7751::~NCV7751(void) {
    // Clean up any dynamically allocated members
    if(_global_en != nullptr) {
        delete _global_en;
    }
}

void NCV7751::enable(void) {
    if(_global_en != nullptr) {
        *_global_en = 1;
    }

    // Turn off all channels initially
    batch_write(0);
}

void NCV7751::disable(void) {
    if(_global_en != nullptr) {
        *_global_en = 0;
    }
}

NCV7751::ChannelOut NCV7751::channel(int num) {
    return ChannelOut(*this, num);
}

uint32_t NCV7751::batch_write(uint16_t channel_bits, uint16_t ol_bits) {
}

uint32_t NCV7751::write_state(uint32_t state) {

    /**
     * Access Channels 1 thru 8
     * CSB Mode = 0b10
     */
    _csb1 = 1;
    _csb2 = 0;
    uint32_t serial_out = _spi.write((uint16_t)(state & 0xFFFF));
    /**
     * Access Channels 9 thru 12
     * CSB Mode = 0b01_T
     */
    _csb2 = 1;
    _csb1 = 0;
    // T = truncated (16-bit vs 24-bit), internal register setting
    serial_out |= (((_spi.write((uint16_t)((state & 0xFFFF0000) >> 16)))
            & 0xFF00) << 12);

    /** Deassert */
    _csb1 = 1;

    return serial_out;

}

NCV7751::ChannelOut::ChannelOut(NCV7751& ncv7751, int channel_num) :
    _parent(ncv7751), _num(channel_num) {
    // Only channels 1 through 12 are supported
    MBED_ASSERT((0 < channel_num) && (channel_num <= 12));
}

void NCV7751::ChannelOut::write(int value) {

    _parent._mutex.lock();

    // Get the cached channel states
    uint16_t new_state = _parent.get_cached_state();

    // Set or clear the channel as indicated by value
    if(value) {

        // Set the channel mode to 10 (ON mode)
        new_state |= (1  << ((_num -1) << 1)+1);    // ((_num - 1) * 2) + 1
        new_state &= ~(1 << ((_num-1) << 1));       // ((_num -1) * 2)
    }

    // Write out the value
    _parent.write_state(new_state);

    _parent._mutex.unlock();
}

int NCV7751::ChannelOut::read() {

    // TODO - get bitmask for a given channel number
    uint32_t state = _parent.get_cached_state();
}

NCV7751::fault_condition_t NCV7751::ChannelOut::get_fault(void) {
}

void NCV7751::ChannelOut::enable_open_load_diag(void) {
}

void NCV7751::ChannelOut::disable_open_load_diag(void) {
}

bool NCV7751::ChannelOut::open_load_diag_enabled(void) {
}
