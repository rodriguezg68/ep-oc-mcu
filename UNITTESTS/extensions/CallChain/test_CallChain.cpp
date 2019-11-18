/* Copyright (c) 2019 ARM Limited
#include <platform_extensions/CallChain.h>
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
 */

#include "gtest/gtest.h"

#include "extensions/CallChain.h"

#include "platform/Span.h"

#include <string>

/*
 *
 */

#define MAX_NUM_BITFLAGS 32

#define MIN(a, b) (((a) <= (b)) ? (a) : (b))

class TestCallChain : public testing::Test {
    
	virtual void SetUp()
	
    {
    }

    virtual void TearDown()
    {
    }

public:

    class BitFlag {
    public :
    	BitFlag(void) : value(0) { }

    	void set_bit(void) {
    		value = 1;
    	}

    	void clear_bit(void) {
    		value = 0;
    	}

    	operator bool() const {
    		return value;
    	}

    protected:
    	bool value; /** Value of this bit flag */
    };

public:

    /**
     * Ensures the flags set by each callback match the expected bitstring
     * This function asserts failure if not.
     *
     * @param[in] bitstring C-style string of '1'/'0' characters representing
     * @param[in]
     * the expected bit flags to be set by the callbacks in a test.
     *
     * IE: callback_0 added, callback_1 added, callback 0 removed ->
     * expected test result is "10" (bit 1 set, bit 0 clear)
     *
     *
     */
    void assert_matching_flags(const std::string bitstring,
    		const mbed::Span<BitFlag> flags) {

    	// Bitstream and length should be equal
    	EXPECT_EQ(bitstring.length(), flags.size());

    	/** Compare each character in the string to its corresponding flag boolean */
    	for(int i = 0; i < bitstring.length(); i++) {
    		bool expected = (bitstring[(bitstring.length()-1)-i] == '1');
    		EXPECT_EQ((bool)flags[i], expected);
    	}
    }

    void reset_flags(const mbed::Span<BitFlag> flags) {
    	for(int i = 0; i < flags.size(); i++) {
    		flags[i].clear_bit();
    	}
    }
};


/**
 * Tests the CallChain with a single callback in it
 */
TEST_F(TestCallChain, single_callback)
{
	BitFlag flags[3];
	mbed::Span<BitFlag, 3> flag_list(flags);
	ep::CallChain<> callchain;
	callchain.attach(mbed::callback(&flags[0], &BitFlag::set_bit));
	callchain.call();

	assert_matching_flags("001", flag_list);
}

/** Tests the CallChain with several callbacks in it */
TEST_F(TestCallChain, multi_callback)
{
	BitFlag flags[3];
	mbed::Span<BitFlag, 3> flag_list(flags);
	ep::CallChain<> callchain;
	callchain.attach(mbed::callback(&flags[0], &BitFlag::set_bit));
	callchain.attach(mbed::callback(&flags[1], &BitFlag::set_bit));
	callchain.attach(mbed::callback(&flags[2], &BitFlag::set_bit));
	callchain.call();

	assert_matching_flags("111", flag_list);
}

/** Tests to make sure callbacks are removed from the CallChain properly.
 * Includes tests for beginning and end removal cases
 */
TEST_F(TestCallChain, detach_middle)
{
	BitFlag flags[3];
	mbed::Span<BitFlag, 3> flag_list(flags);
	ep::CallChain<> callchain;
	callchain.attach(mbed::callback(&flags[0], &BitFlag::set_bit));
	callchain.attach(mbed::callback(&flags[1], &BitFlag::set_bit));
	callchain.attach(mbed::callback(&flags[2], &BitFlag::set_bit));

	// Detach middle one
	callchain.detach(mbed::callback(&flags[1], &BitFlag::set_bit));
	callchain.call();

	assert_matching_flags("101", flag_list);
}

/** Tests to make sure callbacks are removed from the CallChain properly.
 * Includes tests for beginning and end removal cases
 */
TEST_F(TestCallChain, detach_end)
{
	BitFlag flags[3];
	mbed::Span<BitFlag, 3> flag_list(flags);
	ep::CallChain<> callchain;
	callchain.attach(mbed::callback(&flags[0], &BitFlag::set_bit));
	callchain.attach(mbed::callback(&flags[1], &BitFlag::set_bit));
	callchain.attach(mbed::callback(&flags[2], &BitFlag::set_bit));

	// Detach middle one
	callchain.detach(mbed::callback(&flags[2], &BitFlag::set_bit));
	callchain.call();

	assert_matching_flags("011", flag_list);
}

/** Tests to make sure callbacks are removed from the CallChain properly.
 * Includes tests for beginning and end removal cases
 */
TEST_F(TestCallChain, detach_beginning)
{
	BitFlag flags[3];
	mbed::Span<BitFlag, 3> flag_list(flags);
	ep::CallChain<> callchain;
	callchain.attach(mbed::callback(&flags[0], &BitFlag::set_bit));
	callchain.attach(mbed::callback(&flags[1], &BitFlag::set_bit));
	callchain.attach(mbed::callback(&flags[2], &BitFlag::set_bit));

	// Detach middle one
	callchain.detach(mbed::callback(&flags[0], &BitFlag::set_bit));
	callchain.call();

	assert_matching_flags("110", flag_list);
}

/** Tests to make sure callbacks are removed from the CallChain properly.
 * Includes tests for beginning and end removal cases
 */
TEST_F(TestCallChain, detach_all)
{
	BitFlag flags[3];
	mbed::Span<BitFlag, 3> flag_list(flags);
	ep::CallChain<> callchain;
	callchain.attach(mbed::callback(&flags[0], &BitFlag::set_bit));
	callchain.attach(mbed::callback(&flags[1], &BitFlag::set_bit));
	callchain.attach(mbed::callback(&flags[2], &BitFlag::set_bit));

	// Detach middle one
	callchain.detach_all();
	callchain.call();

	assert_matching_flags("000", flag_list);
}

