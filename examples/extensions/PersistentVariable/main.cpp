/**
 * ep-oc-mcu
 * Embedded Planet Open Core for Microcontrollers
 *
 * Built with ARM Mbed-OS
 *
 * Copyright (c) 2019 Embedded Planet, Inc.
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

#include "extensions/PersistentVariable.h"

class TestClass {

public:

	typedef struct multiple_settings_t {
		uint32_t id;
		bool flag;
		float last_val;
	} multiple_settings_t;

public:

	TestClass() : int_setting(10, "/TestClass/int_setting"),
				  flag_setting(true, "/TestClass/flag_setting"),
				  multi_settings({1234, false, 3.24f}, "/TestClass/multi_setting") { }

	ep::PersistentVariable<unsigned int> int_setting;
	ep::PersistentVariable<bool>	flag_setting;
	ep::PersistentVariable<multiple_settings_t> multi_settings;

};


int main(void) {

	ep::PersistentVariable<bool> main_flag(false, "/main/main_flag");

	TestClass my_test;

	printf("IntSetting: %u\r\n", (unsigned int)(my_test.int_setting));
	printf("FlagSetting: %s\r\n", (my_test.flag_setting? "true" : "false"));
	printf("MultiSetting: ");
	TestClass::multiple_settings_t settings = my_test.multi_settings;
	printf("\tid: %lu\r\n", settings.id);
	printf("\tflag: %s\r\n", (settings.flag? "true" : "false"));
	printf("\tlast_val: %f\r\n", settings.last_val);

	printf("main_flag: %s\r\n", (main_flag? "true" : "false"));

	/** Update all the values */
	my_test.int_setting = (my_test.int_setting+1);
	my_test.flag_setting = !(my_test.flag_setting);
	settings.id++;
	settings.flag = !settings.flag;
	settings.last_val += 0.5f;
	my_test.multi_settings = settings;

	main_flag = !(main_flag);

	while(true) {

	}

	return 0;

}
