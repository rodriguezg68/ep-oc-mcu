/*
 * NCV7608.cpp
 *
 *  Created on: Apr 24, 2020
 *      Author: gdbeckstein
 */

#include "NCV7608.h"

#include "mbed_assert.h"

#define NCV7608_TW_BIT (1 << 15)
#define NCV7608_VS_DIAG_BIT (1 << 0)

using namespace ep;

NCV7608::NCV7608(mbed::SPI& spi, PinName csb, PinName global_en) :
        _spi(spi), _cs(nullptr), _global_en(nullptr), _cached_state(0), _cached_diag(
                0) {

    // Instantiate optional control outputs
    if (csb != NC) {
        _cs = new mbed::DigitalOut(csb, 1);
    }

    if (global_en != NC) {
        // Disabled by default
        _global_en = new mbed::DigitalOut(global_en, 0);
    }
}

NCV7608::~NCV7608(void) {
    // Clean up any dynamically allocated members
    if (_cs != nullptr) {
        delete _cs;
    }

    if (_global_en != nullptr) {
        delete _global_en;
    }
}

void NCV7608::enable(void) {
    if (_global_en != nullptr) {
        *_global_en = 1;
    }

    // Turn off all channels initially
    batch_write(0);
}

void NCV7608::disable(void) {
    if (_global_en != nullptr) {
        *_global_en = 0;
    }
}

void ep::NCV7608::assert_cs(void) {
    if (_cs != nullptr) {
        *_cs = 0;
    }
}

void ep::NCV7608::deassert_cs(void) {
    if (_cs != nullptr) {
        *_cs = 1;
    }
}

NCV7608::ChannelOut NCV7608::channel(int num) {
    return ChannelOut(*this, num);
}

uint16_t NCV7608::batch_write(uint8_t channel_bits, uint8_t ol_bits) {

    assert_cs();

    _cached_state = ((channel_bits << 8) | ol_bits);

    uint16_t diag_flipped = _spi.write(_cached_state);
    _cached_diag = ((diag_flipped & 0xFF00) >> 8);
    _cached_diag |= ((diag_flipped & 0xFF) << 8);

    deassert_cs();
    return _cached_diag;
}

uint16_t NCV7608::sync(void) {
    return batch_write((uint8_t) ((_cached_state & 0xFF00) >> 8),
            (uint8_t) (_cached_state & 0x00FF));
}

NCV7608::ChannelOut::ChannelOut(NCV7608& ncv7608, int channel_num) :
        _parent(ncv7608), _num(channel_num - 1) {
    // Only channels 1 through 8 are supported
    MBED_ASSERT((0 < channel_num) && (channel_num <= 8));
}

void NCV7608::ChannelOut::write(int value) {

    // Lock the device mutex
    _parent.mutex.lock();

    // Get the cached channel states
    uint16_t new_state = _parent.get_cached_state();

    // Set or clear the channel as indicated by value
    if (value) {
        // Set the channel enable bit
        new_state |= (0x8000 >> _num);
    } else {
        // Clear the channel enable bit
        new_state &= ~(0x8000 >> _num);
    }

    // Write out the value
    _parent.batch_write((uint8_t) ((new_state & 0xFF00) >> 8),
            (uint8_t) (new_state & 0x00FF));

    // Unlock the device mutex
    _parent.mutex.unlock();

}

int NCV7608::ChannelOut::read() {
    return (_parent.get_cached_state() & (0x8000 >> _num));
}

NCV7608::fault_condition_t NCV7608::ChannelOut::get_fault(void) {

    // Lock the device mutex
    _parent.mutex.lock();

    // Sync diagnostic bits
    uint16_t diag_bits = _parent.sync();

    // Unlock the device mutex
    _parent.mutex.unlock();

    // First see if there's a fault reported on this channel
    if (!(diag_bits & (0x80 >> (_num + 1)))) {
        // No fault, return here
        return NO_FAULT;
    }

    // Okay, fault bit is set. Determine what caused it

    /**
     * Open load fault can only be detected when the channel is off
     * and the open load detection enable bit is set
     */
    if (this->is_off() && this->open_load_diag_enabled()) {
        return OPEN_LOAD;
    }

    /** Thermal fault is indicated globally, so check if that bit is set
     */
    if (diag_bits & NCV7608_TW_BIT) {
        return THERMAL_FAULT;
    }

    /** Otherwise, it must be an over-current fault
     */
    return OVER_CURRENT;
}

void NCV7608::ChannelOut::enable_open_load_diag(void) {

    // Lock the device mutex
    _parent.mutex.lock();

    // Get the cached channel states
    uint16_t new_state = _parent.get_cached_state();

    // Set the channel ol enable bit
    new_state |= (0x80 >> _num);

    // Write out the value
    _parent.batch_write((uint8_t) ((new_state & 0xFF00) >> 8),
            (uint8_t) (new_state & 0x00FF));

    // Unlock the device mutex
    _parent.mutex.unlock();
}

void NCV7608::ChannelOut::disable_open_load_diag(void) {

    // Lock the device mutex
    _parent.mutex.lock();

    // Get the cached channel states
    uint16_t new_state = _parent.get_cached_state();

    // Clear the channel ol enable bit
    new_state &= ~(0x80 >> _num);

    // Write out the value
    _parent.batch_write((uint8_t) ((new_state & 0xFF00) >> 8),
            (uint8_t) (new_state & 0x00FF));

    // Unlock the device mutex
    _parent.mutex.unlock();

}

bool NCV7608::ChannelOut::open_load_diag_enabled(void) {
    return (_parent.get_cached_state() & (0x80 >> _num));
}

