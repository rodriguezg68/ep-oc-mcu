/*
 * GattPresentationFormatDescriptor.h
 *
 *  Created on: May 21, 2019
 *      Author: becksteing
 */

#ifndef EP_CORE_FEATURES_FEATURE_BLE_GATTPRESENTATIONFORMATDESCRIPTOR_H_
#define EP_CORE_FEATURES_FEATURE_BLE_GATTPRESENTATIONFORMATDESCRIPTOR_H_

#include "GattCharacteristic.h"
#include "GattAttribute.h"

/** Length of a presentation format descriptor struct */
#define PRESENTATION_DESC_LEN 7

/**
 * Class encapsulating a GATT Presentation Format Descriptor
 */
class GattPresentationFormatDescriptor : public GattAttribute
{
	public:

		GattPresentationFormatDescriptor(uint8_t format_type, uint16_t unit = GattCharacteristic::BLE_GATT_UNIT_NONE,
				int8_t exponent = 1, uint8_t namespace_id = 0x01, uint16_t namespace_description = 0x0000) :
					GattAttribute((const UUID&) UUID(BLE_UUID_DESCRIPTOR_CHAR_PRESENTATION_FORMAT),
					(uint8_t*) format, PRESENTATION_DESC_LEN, PRESENTATION_DESC_LEN, false)
		{

			/** Populate the format struct */
//			format.gatt_format = format_type;
//			format.exponent = exponent;
//			format.gatt_unit = unit;
//			format.gatt_namespace = namespace_id;
//			format.gatt_nsdesc = namespace_description;

			format[0] = format_type;
			format[1] = exponent;
			memcpy(&format[2], &unit, sizeof(unit));
			format[4] = namespace_id;
			memcpy(&format[5], &namespace_description, sizeof(namespace_description));

		}

	private:

		// Wouldn't it be nice if packing structs was more consistently supported by compilers?
		//struct GattCharacteristic::PresentationFormat_t format;

		// In lieu of using the struct above, packing issues makes us have to use a raw buffer
		uint8_t format[7];
};

#endif /* EP_CORE_FEATURES_FEATURE_BLE_GATTPRESENTATIONFORMATDESCRIPTOR_H_ */
