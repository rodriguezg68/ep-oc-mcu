/*
 * NCV7751.h
 *
 *  Created on: Apr 24, 2020
 *      Author: gdbeckstein
 */

#ifndef EP_OC_MCU_DEVICES_NCV7751_NCV7751_H_
#define EP_OC_MCU_DEVICES_NCV7751_NCV7751_H_

#include "drivers/SPI.h"
#include "drivers/DigitalOut.h"

#include "platform/PlatformMutex.h"

namespace ep {

/**
 * The NCV7751 is an automotive-grade 12-channel low-side output driver
 * that can be controlled over SPI.
 *
 * The NCV7751 has built-in protection, including flyback diodes, ESD,
 * over-current, over temperature, and open load detection. The cause of
 * failure can be diagnosed through the SPI bus interface.
 *
 * The built-in protection features and large number of outputs
 * make the NCV7751 ideal for driving resistive as well as inductive loads
 * in an automotive setting while minimizing BOM complexity. It is especially
 * useful in I/O-constrained applications.
 *
 */
class NCV7751
{


public:

    /**
     *
     */
    typedef enum {
        NO_FAULT,           /** No fault condition */
        OPEN_LOAD,          /** Open load condition exists on channel */
        OVER_LOAD,          /** Over-load condition exists on channel */
        OVER_TEMPERATURE,   /** Over temperature fault */
    } fault_condition_t;

    /**
     * Convenience class similar to DigitalOut
     */
    class ChannelOut {

    public:

        /**
         * Construct a ChannelOut
         * @param[in] ncv7751 Parent ncv7751 instance
         * @param[in] num Channel number (1 through 12)
         *
         * @note the preferred method create a ChannelOut is
         * to use ::channel on the given NCV7751 instance
         */
        ChannelOut(NCV7751& ncv7751, int num);

        /** Set the output off or on, specified as 0 or 1 (int), respectively
         *
         *  @param value An integer specifying the pin output value,
         *      0 for logical 0, 1 (or any other non-zero value) for logical 1
         */
        void write(int value);

        /** Return the output setting, represented as 0 or 1 (int)
         *
         *  @returns
         *    an integer representing the output setting of the pin,
         *    0 for logical 0, 1 for logical 1
         */
        int read();

        /**
         * Gets the fault condition of the channel
         *
         * @note this does not check the PWM input status bits
         *
         * @note If a fault is reported on a channel, you
         * must typically disable the channel and then re-enable
         * the channel to reset the fault condition. This will only
         * work if the cause of the fault is also removed.
         */
        fault_condition_t get_fault(void);

        /**
         * Enables the open load diagnostics on this channel
         */
        void enable_open_load_diag(void);

        /**
         * Disables the open load diagnostics on this channel
         */
        void disable_open_load_diag(void);

        /**
         * Checks if open load diagnostics are enabled on this channel
         */
        bool open_load_diag_enabled(void);

        /**
         * Set the output to on
         */
        inline void on(void) {
            write(1);
        }

        /**
         * Set the output to off
         */
        inline void off(void) {
            write(0);
        }

        /**
         * Returns true if current state of channel is on
         */
        inline bool is_on(void) {
            return (read() == 1);
        }

        /**
         * Returns true if current state of channel is off
         */
        inline bool is_off(void) {
            return (read() == 0);
        }

        /** A shorthand for write()
         * \sa ChannelOut::write()
         * @code
         *      ChannelOut led(LED1);
         *      led = 1;   // Equivalent to led.write(1)
         * @endcode
         */
        ChannelOut &operator=(int value) {
            // Underlying write is thread safe
            write(value);
            return *this;
        }

        /** A shorthand for write() using the assignment operator which copies the
         * state from the DigitalOut argument.
         * \sa DigitalOut::write()
         */
        ChannelOut &operator=(ChannelOut &rhs) {
            write(rhs.read());
            return *this;
        }

        /** A shorthand for read()
         * \sa DigitalOut::read()
         */
        operator int() {
            // Underlying call is thread safe
            return read();
        }

    protected:
        NCV7751& _parent; /** Parent NCV7751 of this channel object */
        int _num; /** Channel number of this object (1-12) */

    };

public:

    /** Allow ChannelOut to access internal members */
    friend class ChannelOut;

    /**
     * Instantiate an NCV7751 driver
     * @param[in] spi SPI bus instance to use for communication (16-bit format!)
     * @param[in] csb1 Chip select "bar" 1
     * @param[in] csb2 Chip select "bar" 2
     * @param[in] global_en Global enable pin, defaults to NC (unused)
     *
     * @note The SPI bus instance used must be configured for 16-bit format
     * to work properly!
     */
    NCV7751(mbed::SPI& spi, PinName csb1, PinName csb2, PinName global_en = NC);

    /**
     * Destructor
     */
    ~NCV7751(void);

    /**
     * Globally enable (if global_en != NC)
     */
    void enable(void);

    /**
     * Globally disable (if global_en != NC)
     */
    void disable(void);

    /**
     * Convenience function to create a ChannelOut object for a given
     * channel
     *
     * @param[in] num Desired channel number. Allowed values: 1 thru 12
     * @retval channel Reference to channel
     */
    ChannelOut channel(int num);

    /**
     * Batch writes channel settings to the NCV7751
     *
     * If your application requires closely-timed output transitions,
     * this function ensures the output states are updated in the same
     * SPI transaction.
     *
     * This is useful if, for example, you are using two outputs in
     * parallel to achieve a higher current capacity
     *
     * @param[in] channel_bits Each desired channel state is represented by
     * a bit in this number. The bit corresponds to channel (bit_pos) + 1.
     * (eg: bit 0 represents the desired state of channel 1, 1 = on, 0 = off)
     *
     * @param[in] open_load_en Similar to channel bits, each bit in this
     * number represents whether open-load diagnostics are desired on
     * the given channel. 0 = not enabled, 1 = enabled. Defaults to disabled.
     *
     * @retval diag_bits 32-bit output from NCV7608 representing the diagnostics
     * state of each channel. If you are using the ChannelOut API there are
     * convenience functions to interpret this information for you.
     */
    uint32_t batch_write(uint16_t channel_bits, uint16_t ol_bits = 0x0);

protected:

    /**
     * Writes a new 32-bit state to the NCV7751
     * @param[in] state New state to write
     * @retval diag_bits See ::batch_write
     */
    uint32_t write_state(uint32_t state);

    /**
     * Returns the cached channel state
     */
    uint32_t get_cached_state(void) {
        return _cached_state;
    }

    /**
     * Returns the cached diagnostics bits
     */
    uint32_t get_cached_diag(void) {
        return _cached_diag;
    }

protected:

    mbed::SPI& _spi;
    mbed::DigitalOut _csb1;
    mbed::DigitalOut _csb2;
    mbed::DigitalOut* _global_en;

    uint32_t _cached_state; /** Cached channel on/off state bits */
    uint32_t _cached_diag;  /** Cached diagnostics bits */

    PlatformMutex _mutex;

};

}



#endif /* EP_OC_MCU_DEVICES_NCV7751_NCV7751_H_ */
