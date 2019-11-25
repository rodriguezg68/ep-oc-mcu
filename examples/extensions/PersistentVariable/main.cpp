

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
