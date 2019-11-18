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

#include <algorithm>
#include <string>

/*
 *
 */

#define MAX_NUM_BITFLAGS 32

class TestCallChain : public testing::Test {
    
	virtual void SetUp()
	
    {
		//memset()
    }

    virtual void TearDown()
    {
    }

public:


    /**
     * Ensures the flags set by each callback match the expected bitstring
     * This function asserts failure if not.
     *
     * @param[in] bitstring C-style string of '1'/'0' characters representing
     * the expected bit flags to be set by the callbacks in a test.
     *
     * IE: callback_1 added, callback_2 added, callback 1 removed ->
     * expected test result is "10"
     *
     *
     * @note this is not thread safe
     */
    void assert_matching_flags(const std::string bitstring) {

    	/** Compare each character in the string to its corresponding flag boolean */

    	// Reset the position in the bit flags
    	current_bit = bit_flags;
    	std::for_each(bitstring.begin(), bitstring.end(),[this](const char& c) {
    		bool expected = ((c == '1')? true : false);
    		EXPECT_EQ(*current_bit, expected);
    	});
    }

    bool* current_bit; /** Current bit being checked */
    bool bit_flags[MAX_NUM_BITFLAGS];

private:

};

TEST_F(TestCallChain, constructor)
{

}
