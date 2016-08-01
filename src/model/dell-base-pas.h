

/*
 * Copyright (c) 2016 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 *  LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */

#ifndef DELL_BASE_PAS_H
#define DELL_BASE_PAS_H

#include "cps_api_operation.h"
#include "dell-base-platform-common.h"
#include <stdint.h>
#include <stdbool.h>


#define cps_api_obj_CAT_BASE_PAS (28) 

#define DELL_BASE_PAS_MODEL_STR "dell-base-pas"


/* Object base-pas/temperature */

typedef enum { 
/*Entity type*/
/*type=enumeration*/ 
  BASE_PAS_TEMPERATURE_ENTITY_TYPE = 1835009,
/*Entity slot*/
/*type=uint8*/ 
  BASE_PAS_TEMPERATURE_SLOT = 1835010,
/*Printable name for sensor*/
/*type=string*/ 
  BASE_PAS_TEMPERATURE_NAME = 1835011,
/*Operational status*/
/*type=enumeration*/ 
  BASE_PAS_TEMPERATURE_OPER_STATUS = 1835012,
/*Cause of fault*/
/*type=enumeration*/ 
  BASE_PAS_TEMPERATURE_FAULT_TYPE = 1835013,
/*Current temperature*/
/*type=int16*/ 
  BASE_PAS_TEMPERATURE_TEMPERATURE = 1835014,
/*Threshold for hardware-based shutdown*/
/*type=int16*/ 
  BASE_PAS_TEMPERATURE_SHUTDOWN_THRESHOLD = 1835015,
/*Enable threshold detection*/
/*type=boolean*/ 
  BASE_PAS_TEMPERATURE_THRESH_ENABLE = 1835016,
/*Last threshold temperature crossed*/
/*type=int16*/ 
  BASE_PAS_TEMPERATURE_LAST_THRESH = 1835017,
} BASE_PAS_TEMPERATURE_t;
/* Object base-pas/fan-tray */

typedef enum { 
/*Fan tray slot*/
/*type=uint8*/ 
  BASE_PAS_FAN_TRAY_SLOT = 1835018,
/*Direction of airflow for fans*/
/*type=enumeration*/ 
  BASE_PAS_FAN_TRAY_FAN_AIRFLOW_TYPE = 1835019,
} BASE_PAS_FAN_TRAY_t;
/* Object base-pas/media-channel */

typedef enum { 
/*Card slot*/
/*type=uint8*/ 
  BASE_PAS_MEDIA_CHANNEL_SLOT = 1835020,
/*Slot of port module on card*/
/*type=uint8*/ 
  BASE_PAS_MEDIA_CHANNEL_PORT_MODULE_SLOT = 1835021,
/*Port on port module*/
/*type=uint8*/ 
  BASE_PAS_MEDIA_CHANNEL_PORT = 1835022,
/*Channel of given port*/
/*type=uint8*/ 
  BASE_PAS_MEDIA_CHANNEL_CHANNEL = 1835023,
/*Channel state (Enable/Disable)*/
/*type=boolean*/ 
  BASE_PAS_MEDIA_CHANNEL_STATE = 1835024,
/*RX power*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_CHANNEL_RX_POWER = 1835025,
/*TX power*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_CHANNEL_TX_POWER = 1835026,
/*TX Bias Current*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_CHANNEL_TX_BIAS_CURRENT = 1835027,
/*Operational status*/
/*type=enumeration*/ 
  BASE_PAS_MEDIA_CHANNEL_OPER_STATUS = 1835028,
/*Channel TX loss status*/
/*type=boolean*/ 
  BASE_PAS_MEDIA_CHANNEL_RX_LOSS = 1835029,
/*Channel TX loss status*/
/*type=boolean*/ 
  BASE_PAS_MEDIA_CHANNEL_TX_LOSS = 1835030,
/*Channel TX fault status*/
/*type=boolean*/ 
  BASE_PAS_MEDIA_CHANNEL_TX_FAULT = 1835031,
/*Channel TX disable status*/
/*type=boolean*/ 
  BASE_PAS_MEDIA_CHANNEL_TX_DISABLE = 1835032,
/*RX power High/Low alarm/warning state*/
/*type=enumeration*/ 
  BASE_PAS_MEDIA_CHANNEL_RX_POWER_STATE = 1835033,
/*TX power High/Low alarm/warning state*/
/*type=enumeration*/ 
  BASE_PAS_MEDIA_CHANNEL_TX_POWER_STATE = 1835034,
/*TX-BIAS High/Low alarm/warning state*/
/*type=enumeration*/ 
  BASE_PAS_MEDIA_CHANNEL_TX_BIAS_STATE = 1835035,
/*Speed to derive and set LED state*/
/*type=enumeration*/ 
  BASE_PAS_MEDIA_CHANNEL_SPEED = 1835036,
/*Auto Negotiation*/
/*type=boolean*/ 
  BASE_PAS_MEDIA_CHANNEL_AUTONEG = 1835224,
/*Interface supported speed list*/
/*type=enumeration*/ 
  BASE_PAS_MEDIA_CHANNEL_SUPPORTED_SPEED = 1835225,
} BASE_PAS_MEDIA_CHANNEL_t;
/* Object base-pas/media-config */

typedef enum { 
/*Node Id (Chassis id / stackId)*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_CONFIG_NODE_ID = 1835037,
/*Card slot number*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_CONFIG_SLOT = 1835038,
/*Lock down feature status.*/
/*type=boolean*/ 
  BASE_PAS_MEDIA_CONFIG_LOCK_DOWN_STATUS = 1835039,
/*LED control flag*/
/*type=boolean*/ 
  BASE_PAS_MEDIA_CONFIG_LED_CONTROL = 1835040,
} BASE_PAS_MEDIA_CONFIG_t;
/* Object base-pas/entity/entity-type-choice/card */

typedef enum { 
/*Power control for entity*/
/*type=boolean*/ 
  BASE_PAS_ENTITY_ENTITY_TYPE_CHOICE_CARD_POWER_ON = 1835041,
/*reboot type*/
/*type=enumeration*/ 
  BASE_PAS_ENTITY_ENTITY_TYPE_CHOICE_CARD_REBOOT = 1835042,
} BASE_PAS_ENTITY_ENTITY_TYPE_CHOICE_CARD_t;
/* Object base-pas/media/media-category */

typedef enum { 

/*type=binary*/ 
  BASE_PAS_MEDIA_MEDIA_CATEGORY_QSFP_PLUS = 1835043,

/*type=binary*/ 
  BASE_PAS_MEDIA_MEDIA_CATEGORY_SFP_PLUS = 1835044,
} BASE_PAS_MEDIA_MEDIA_CATEGORY_t;
/* Object base-pas/entity */

typedef enum { 
/*Entity type*/
/*type=enumeration*/ 
  BASE_PAS_ENTITY_ENTITY_TYPE = 1835045,
/*Entity slot*/
/*type=uint8*/ 
  BASE_PAS_ENTITY_SLOT = 1835046,
/*Entity vendor name*/
/*type=string*/ 
  BASE_PAS_ENTITY_VENDOR_NAME = 1835047,
/*Entity product name*/
/*type=string*/ 
  BASE_PAS_ENTITY_PRODUCT_NAME = 1835048,
/*Entity hardware version*/
/*type=string*/ 
  BASE_PAS_ENTITY_HW_VERSION = 1835049,
/*Entity platform name*/
/*type=string*/ 
  BASE_PAS_ENTITY_PLATFORM_NAME = 1835050,
/*Entity PPID*/
/*type=string*/ 
  BASE_PAS_ENTITY_PPID = 1835051,
/*Entity Part Number*/
/*type=string*/ 
  BASE_PAS_ENTITY_PART_NUMBER = 1835052,
/*Entity service tag*/
/*type=string*/ 
  BASE_PAS_ENTITY_SERVICE_TAG = 1835053,
/*Printable name of entity*/
/*type=string*/ 
  BASE_PAS_ENTITY_NAME = 1835054,
/*Entity present flag*/
/*type=boolean*/ 
  BASE_PAS_ENTITY_PRESENT = 1835055,
/*Number of times entity inserted*/
/*type=uint64*/ 
  BASE_PAS_ENTITY_INSERTION_CNT = 1835056,
/*Timestamp of last entity insertion*/
/*type=uint32*/ 
  BASE_PAS_ENTITY_INSERTION_TIMESTAMP = 1835057,
/*Administrative status*/
/*type=enumeration*/ 
  BASE_PAS_ENTITY_ADMIN_STATUS = 1835058,
/*Operational status*/
/*type=enumeration*/ 
  BASE_PAS_ENTITY_OPER_STATUS = 1835059,
/*Cause of fault*/
/*type=enumeration*/ 
  BASE_PAS_ENTITY_FAULT_TYPE = 1835060,

/*type=binary*/ 
  BASE_PAS_ENTITY_ENTITY_TYPE_CHOICE = 1835061,
} BASE_PAS_ENTITY_t;
/* Object base-pas/led */

typedef enum { 
/*Entity type*/
/*type=enumeration*/ 
  BASE_PAS_LED_ENTITY_TYPE = 1835062,
/*Entity slot*/
/*type=uint8*/ 
  BASE_PAS_LED_SLOT = 1835063,
/*Printable name for LED*/
/*type=string*/ 
  BASE_PAS_LED_NAME = 1835064,
/*Operational status*/
/*type=enumeration*/ 
  BASE_PAS_LED_OPER_STATUS = 1835065,
/*Cause of fault*/
/*type=enumeration*/ 
  BASE_PAS_LED_FAULT_TYPE = 1835066,
/*Illumination state of LED requested by application*/
/*type=boolean*/ 
  BASE_PAS_LED_REQ_ON = 1835067,
/*Illumination state of physical LED*/
/*type=boolean*/ 
  BASE_PAS_LED_ON = 1835068,
} BASE_PAS_LED_t;
/* Object base-pas/fan */

typedef enum { 
/*Entity type*/
/*type=enumeration*/ 
  BASE_PAS_FAN_ENTITY_TYPE = 1835069,
/*Entity slot*/
/*type=uint8*/ 
  BASE_PAS_FAN_SLOT = 1835070,
/*Index of fan on entity*/
/*type=uint8*/ 
  BASE_PAS_FAN_FAN_INDEX = 1835071,
/*Operational status*/
/*type=enumeration*/ 
  BASE_PAS_FAN_OPER_STATUS = 1835072,
/*Cause of fault*/
/*type=enumeration*/ 
  BASE_PAS_FAN_FAULT_TYPE = 1835073,
/*Fan speed, in RPM*/
/*type=uint16*/ 
  BASE_PAS_FAN_SPEED = 1835074,
/*Fan speed, as % of maximum*/
/*type=uint8*/ 
  BASE_PAS_FAN_SPEED_PCT = 1835075,
/*Maximum speed of fan, in RPM*/
/*type=uint16*/ 
  BASE_PAS_FAN_MAX_SPEED = 1835076,
} BASE_PAS_FAN_t;
/* Object base-pas/media */

typedef enum { 
/*Card slot*/
/*type=uint8*/ 
  BASE_PAS_MEDIA_SLOT = 1835077,
/*Slot of port module on card*/
/*type=uint8*/ 
  BASE_PAS_MEDIA_PORT_MODULE_SLOT = 1835078,
/*Port on port module*/
/*type=uint8*/ 
  BASE_PAS_MEDIA_PORT = 1835079,
/*Adapter present flag*/
/*type=boolean*/ 
  BASE_PAS_MEDIA_PRESENT = 1835080,
/*Number of times adapter inserted*/
/*type=uint64*/ 
  BASE_PAS_MEDIA_INSERTION_CNT = 1835081,
/*Timestamp of last adapter insertion*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_INSERTION_TIMESTAMP = 1835082,
/*Administrative status*/
/*type=enumeration*/ 
  BASE_PAS_MEDIA_ADMIN_STATUS = 1835083,
/*Operational status*/
/*type=enumeration*/ 
  BASE_PAS_MEDIA_OPER_STATUS = 1835084,
/*Media category*/
/*type=enumeration*/ 
  BASE_PAS_MEDIA_CATEGORY = 1835085,
/*Media type*/
/*type=enumeration*/ 
  BASE_PAS_MEDIA_TYPE = 1835086,
/*Media speed capability*/
/*type=enumeration*/ 
  BASE_PAS_MEDIA_CAPABILITY = 1835087,
/*Vendor id of the physical media*/
/*type=string*/ 
  BASE_PAS_MEDIA_VENDOR_ID = 1835088,
/*Serial number of the physical media*/
/*type=string*/ 
  BASE_PAS_MEDIA_SERIAL_NUMBER = 1835089,
/*Is dell qualified or not*/
/*type=boolean*/ 
  BASE_PAS_MEDIA_DELL_QUALIFIED = 1835090,
/*Enable high power mode*/
/*type=boolean*/ 
  BASE_PAS_MEDIA_HIGH_POWER_MODE = 1835091,
/*Type of serial transceiver*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_IDENTIFIER = 1835092,
/*Extended identifier of type of serial transceiver*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_EXT_IDENTIFIER = 1835093,
/*Code for connector type*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_CONNECTOR = 1835094,
/*Code for electronic or optical compatibility*/
/*type=binary*/ 
  BASE_PAS_MEDIA_TRANSCEIVER = 1835095,
/*Code for serial encoding algorithm*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_ENCODING = 1835096,
/*Nominal signaling rate, units of 100Mbits/sec*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_BR_NOMINAL = 1835097,
/*Type of rate select functionality*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_RATE_IDENTIFIER = 1835098,
/*Link length supported for SMF fiber in km*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_LENGTH_SFM_KM = 1835099,
/*Link length supported for single mode fiber, units of 100m*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_LENGTH_SFM = 1835100,
/*Link length supported for 50um OM2 fiber, units of 10m*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_LENGTH_OM2 = 1835101,
/*Link length supported for 62.5um OM1 fiber, units of 10m*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_LENGTH_OM1 = 1835102,
/*Link length supported for copper or direct attach cable, units of m*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_LENGTH_CABLE = 1835103,
/*Link length supported for 50um OM3 fiber, units of 10m*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_LENGTH_OM3 = 1835104,
/*Vendor name (ASCII)*/
/*type=string*/ 
  BASE_PAS_MEDIA_VENDOR_NAME = 1835105,
/*Extended Transceiver Codes*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_EXT_TRANSCEIVER = 1835106,
/*Part number provided by SFP+ transceiver vendor (ASCII)*/
/*type=string*/ 
  BASE_PAS_MEDIA_VENDOR_PN = 1835107,
/*Revision level for part number provided by vendor (ASCII)*/
/*type=binary*/ 
  BASE_PAS_MEDIA_VENDOR_REV = 1835108,
/*Laser wavelength (Passive/Active Cable Specification Compliance)*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_WAVELENGTH = 1835109,

/*type=binary*/ 
  BASE_PAS_MEDIA_MEDIA_CATEGORY = 1835110,
/*Check code for Base ID Fields (addresses 0 to 62)*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_CC_BASE = 1835111,
/*Indicates which optional SFP+ signals are implemented*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_OPTIONS = 1835112,
/*Vendor's manufacturing date code*/
/*type=binary*/ 
  BASE_PAS_MEDIA_DATE_CODE = 1835113,
/*Indicates which type of diagnostic monitoring is implemented (if any)*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_DIAG_MON_TYPE = 1835114,
/*Indicates which optional enhanced features are implemented (if any)*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_ENHANCED_OPTIONS = 1835115,
/*Check code for the Extended ID Fields (addr. 64 to 94)*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_CC_EXT = 1835116,
/*Vendor Specific EEPROM*/
/*type=binary*/ 
  BASE_PAS_MEDIA_VENDOR_SPECIFIC = 1835117,
/*Rate select state*/
/*type=boolean*/ 
  BASE_PAS_MEDIA_RATE_SELECT_STATE = 1835118,
/*Received power measurement type*/
/*type=enumeration*/ 
  BASE_PAS_MEDIA_RX_POWER_MEASUREMENT_TYPE = 1835119,
/*Temperature High alarm*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_TEMP_HIGH_ALARM_THRESHOLD = 1835120,
/*Temperature Low alarm*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_TEMP_LOW_ALARM_THRESHOLD = 1835121,
/*Temperature High warning*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_TEMP_HIGH_WARNING_THRESHOLD = 1835122,
/*Temperature Low warning*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_TEMP_LOW_WARNING_THRESHOLD = 1835123,
/*Voltage High alarm*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_VOLTAGE_HIGH_ALARM_THRESHOLD = 1835124,
/*Voltage Low alarm*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_VOLTAGE_LOW_ALARM_THRESHOLD = 1835125,
/*Voltage High warning*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_VOLTAGE_HIGH_WARNING_THRESHOLD = 1835126,
/*Voltage Low warning*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_VOLTAGE_LOW_WARNING_THRESHOLD = 1835127,
/*RX Power High alarm*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_RX_POWER_HIGH_ALARM_THRESHOLD = 1835128,
/*RX Power Low alarm*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_RX_POWER_LOW_ALARM_THRESHOLD = 1835129,
/*RX Power High warning*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_RX_POWER_HIGH_WARNING_THRESHOLD = 1835130,
/*RX Power Low warning*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_RX_POWER_LOW_WARNING_THRESHOLD = 1835131,
/*Bias High alarm*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_BIAS_HIGH_ALARM_THRESHOLD = 1835132,
/*Bias Low alarm*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_BIAS_LOW_ALARM_THRESHOLD = 1835133,
/*Bias High warning*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_BIAS_HIGH_WARNING_THRESHOLD = 1835134,
/*Bias Low warning*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_BIAS_LOW_WARNING_THRESHOLD = 1835135,
/*Tx power High alarm*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_TX_POWER_HIGH_ALARM_THRESHOLD = 1835136,
/*Tx power Low alarm*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_TX_POWER_LOW_ALARM_THRESHOLD = 1835137,
/*Tx power High warning*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_TX_POWER_HIGH_WARNING_THRESHOLD = 1835138,
/*Tx power Low warning*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_TX_POWER_LOW_WARNING_THRESHOLD = 1835139,
/*Current temperature*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_CURRENT_TEMPERATURE = 1835140,
/*Current voltage*/
/*type=decimal64*/ 
  BASE_PAS_MEDIA_CURRENT_VOLTAGE = 1835141,
/*Temperature High/Low alarm/warning state*/
/*type=enumeration*/ 
  BASE_PAS_MEDIA_TEMP_STATE = 1835142,
/*Voltage High/Low alarm/warning state*/
/*type=enumeration*/ 
  BASE_PAS_MEDIA_VOLTAGE_STATE = 1835143,
/*Auto Negotiation*/
/*type=boolean*/ 
  BASE_PAS_MEDIA_AUTONEG = 1835144,
/*Interface supported speed list*/
/*type=enumeration*/ 
  BASE_PAS_MEDIA_SUPPORTED_SPEED = 1835145,
} BASE_PAS_MEDIA_t;
/* Object base-pas/card */

typedef enum { 
/*Card slot*/
/*type=uint8*/ 
  BASE_PAS_CARD_SLOT = 1835146,
/*Type of card*/
/*type=uint32*/ 
  BASE_PAS_CARD_CARD_TYPE = 1835147,
} BASE_PAS_CARD_t;
/* Object base-pas/phy */

typedef enum { 
/*Card slot*/
/*type=uint8*/ 
  BASE_PAS_PHY_SLOT = 1835148,
/*Slot of port module on card*/
/*type=uint8*/ 
  BASE_PAS_PHY_PORT_MODULE_SLOT = 1835149,
/*Port on port module*/
/*type=uint8*/ 
  BASE_PAS_PHY_PORT = 1835150,
/*Administrative status*/
/*type=enumeration*/ 
  BASE_PAS_PHY_ADMIN_STATUS = 1835151,
/*Operational status*/
/*type=enumeration*/ 
  BASE_PAS_PHY_OPER_STATUS = 1835152,
/*Cause of fault*/
/*type=enumeration*/ 
  BASE_PAS_PHY_FAULT_TYPE = 1835153,
} BASE_PAS_PHY_t;
/* Object base-pas/pld */

typedef enum { 
/*Entity type*/
/*type=enumeration*/ 
  BASE_PAS_PLD_ENTITY_TYPE = 1835154,
/*Entity slot*/
/*type=uint8*/ 
  BASE_PAS_PLD_SLOT = 1835155,
/*Printable name for PLD*/
/*type=string*/ 
  BASE_PAS_PLD_NAME = 1835156,
/*Operational status*/
/*type=enumeration*/ 
  BASE_PAS_PLD_OPER_STATUS = 1835157,
/*Cause of fault*/
/*type=enumeration*/ 
  BASE_PAS_PLD_FAULT_TYPE = 1835158,
/*PLD firmware version*/
/*type=string*/ 
  BASE_PAS_PLD_VERSION = 1835159,
/*File containing new PLD firmware*/
/*type=string*/ 
  BASE_PAS_PLD_FW_FILENAME = 1835160,
} BASE_PAS_PLD_t;
/* Object base-pas/psu */

typedef enum { 
/*PSU slot*/
/*type=uint8*/ 
  BASE_PAS_PSU_SLOT = 1835161,
/*Input current type*/
/*type=enumeration*/ 
  BASE_PAS_PSU_INPUT_TYPE = 1835162,
/*Direction of airflow for PSU fan(s)*/
/*type=enumeration*/ 
  BASE_PAS_PSU_FAN_AIRFLOW_TYPE = 1835163,
} BASE_PAS_PSU_t;
/* Object base-pas/temp_threshold */

typedef enum { 
/*Entity type*/
/*type=enumeration*/ 
  BASE_PAS_TEMP_THRESHOLD_ENTITY_TYPE = 1835164,
/*Entity slot*/
/*type=uint8*/ 
  BASE_PAS_TEMP_THRESHOLD_SLOT = 1835165,
/*Printable name for sensor*/
/*type=string*/ 
  BASE_PAS_TEMP_THRESHOLD_NAME = 1835166,
/*Index of threshold for sensor*/
/*type=uint8*/ 
  BASE_PAS_TEMP_THRESHOLD_THRESHOLD_INDEX = 1835167,
/*Threshold for increasing temperature*/
/*type=int16*/ 
  BASE_PAS_TEMP_THRESHOLD_HI = 1835168,
/*Threshold for decreasing temperature*/
/*type=int16*/ 
  BASE_PAS_TEMP_THRESHOLD_LO = 1835169,
} BASE_PAS_TEMP_THRESHOLD_t;
/* Object base-pas/ready */

typedef enum { 
/*Slot number*/
/*type=uint8*/ 
  BASE_PAS_READY_SLOT = 1835170,
/*PAS status*/
/*type=boolean*/ 
  BASE_PAS_READY_STATUS = 1835171,
} BASE_PAS_READY_t;
/* Object base-pas/media/media-category/sfp-plus */

typedef enum { 
/*Upper bit rate margin, units of %*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_MEDIA_CATEGORY_SFP_PLUS_BR_MAX = 1835172,
/*Lower bit rate margin, units of %*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_MEDIA_CATEGORY_SFP_PLUS_BR_MIN = 1835173,
/*Indicates which revision of SFF-8472 the transceiver complies with*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_MEDIA_CATEGORY_SFP_PLUS_SFF_8472_COMPLIANCE = 1835174,
} BASE_PAS_MEDIA_MEDIA_CATEGORY_SFP_PLUS_t;
/* Object base-pas/port-module */

typedef enum { 
/*Card slot*/
/*type=uint8*/ 
  BASE_PAS_PORT_MODULE_SLOT = 1835175,
/*Slot of port module on card*/
/*type=uint8*/ 
  BASE_PAS_PORT_MODULE_PORT_MODULE_SLOT = 1835176,
/*Module present flag*/
/*type=boolean*/ 
  BASE_PAS_PORT_MODULE_PRESENT = 1835177,
/*Number of times module inserted*/
/*type=uint64*/ 
  BASE_PAS_PORT_MODULE_INSERTION_CNT = 1835178,
/*Timestamp of last module insertion*/
/*type=uint32*/ 
  BASE_PAS_PORT_MODULE_INSERTION_TIMESTAMP = 1835179,
/*Administrative status*/
/*type=enumeration*/ 
  BASE_PAS_PORT_MODULE_ADMIN_STATUS = 1835180,
/*Operational status*/
/*type=enumeration*/ 
  BASE_PAS_PORT_MODULE_OPER_STATUS = 1835181,
/*Cause of fault*/
/*type=enumeration*/ 
  BASE_PAS_PORT_MODULE_FAULT_TYPE = 1835182,
} BASE_PAS_PORT_MODULE_t;
/* Object base-pas/chassis */

typedef enum { 
/*Entity vendor name*/
/*type=string*/ 
  BASE_PAS_CHASSIS_VENDOR_NAME = 1835183,
/*Entity product name*/
/*type=string*/ 
  BASE_PAS_CHASSIS_PRODUCT_NAME = 1835184,
/*Entity hardware version*/
/*type=string*/ 
  BASE_PAS_CHASSIS_HW_VERSION = 1835185,
/*Entity platform name*/
/*type=string*/ 
  BASE_PAS_CHASSIS_PLATFORM_NAME = 1835186,
/*Entity PPID*/
/*type=string*/ 
  BASE_PAS_CHASSIS_PPID = 1835187,
/*Entity Part Number*/
/*type=string*/ 
  BASE_PAS_CHASSIS_PART_NUMBER = 1835188,
/*Entity service tag*/
/*type=string*/ 
  BASE_PAS_CHASSIS_SERVICE_TAG = 1835189,
/*Operational status*/
/*type=enumeration*/ 
  BASE_PAS_CHASSIS_OPER_STATUS = 1835190,
/*Cause of fault*/
/*type=enumeration*/ 
  BASE_PAS_CHASSIS_FAULT_TYPE = 1835191,
/*Number of MAC addresses assigned to chassis*/
/*type=uint32*/ 
  BASE_PAS_CHASSIS_NUM_MAC_ADDRESSES = 1835192,
/*Base MAC addresses assigned to chassis*/
/*type=binary*/ 
  BASE_PAS_CHASSIS_BASE_MAC_ADDRESSES = 1835193,
/*Total power consumed by chassis*/
/*type=uint32*/ 
  BASE_PAS_CHASSIS_POWER_USAGE = 1835194,
/*Kill power to chassis*/
/*type=boolean*/ 
  BASE_PAS_CHASSIS_POWER_OFF = 1835195,
/*Slot of active RPM*/
/*type=uint8*/ 
  BASE_PAS_CHASSIS_ACTIVE_RPM_SLOT = 1835196,
/*reboot type*/
/*type=enumeration*/ 
  BASE_PAS_CHASSIS_REBOOT = 1835197,
} BASE_PAS_CHASSIS_t;
/* Object base-pas/display */

typedef enum { 
/*Entity type*/
/*type=enumeration*/ 
  BASE_PAS_DISPLAY_ENTITY_TYPE = 1835198,
/*Entity slot*/
/*type=uint8*/ 
  BASE_PAS_DISPLAY_SLOT = 1835199,
/*Printable name for display*/
/*type=string*/ 
  BASE_PAS_DISPLAY_NAME = 1835200,
/*Operational status*/
/*type=enumeration*/ 
  BASE_PAS_DISPLAY_OPER_STATUS = 1835201,
/*Cause of fault*/
/*type=enumeration*/ 
  BASE_PAS_DISPLAY_FAULT_TYPE = 1835202,
/*Message to display in text display*/
/*type=string*/ 
  BASE_PAS_DISPLAY_MESSAGE = 1835203,
} BASE_PAS_DISPLAY_t;
/* Object base-pas/media/media-category/qsfp-plus */

typedef enum { 
/*Guaranteed range of laser wavelength*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_MEDIA_CATEGORY_QSFP_PLUS_WAVELENGTH_TOLERANCE = 1835204,
/*Maximum Case Temperature in Degrees C*/
/*type=uint32*/ 
  BASE_PAS_MEDIA_MEDIA_CATEGORY_QSFP_PLUS_MAX_CASE_TEMP = 1835205,
} BASE_PAS_MEDIA_MEDIA_CATEGORY_QSFP_PLUS_t;
/* Object base-pas/entity/entity-type-choice */

typedef enum { 

/*type=binary*/ 
  BASE_PAS_ENTITY_ENTITY_TYPE_CHOICE_CARD = 1835206,
} BASE_PAS_ENTITY_ENTITY_TYPE_CHOICE_t;

/* Object's continued */

typedef enum{
/*PAS status notification*/
  BASE_PAS_READY = 1835207,
  BASE_PAS_READY_OBJ = 1835207,

/*Attributes of a chassis*/
  BASE_PAS_CHASSIS = 1835208,
  BASE_PAS_CHASSIS_OBJ = 1835208,

/*Attributes of an entity (FRU)*/
  BASE_PAS_ENTITY = 1835209,
  BASE_PAS_ENTITY_OBJ = 1835209,

/*Attributes of a power supply*/
  BASE_PAS_PSU = 1835210,
  BASE_PAS_PSU_OBJ = 1835210,

/*Attributes of a fan tray*/
  BASE_PAS_FAN_TRAY = 1835211,
  BASE_PAS_FAN_TRAY_OBJ = 1835211,

/*Attributes of a card*/
  BASE_PAS_CARD = 1835212,
  BASE_PAS_CARD_OBJ = 1835212,

/*Attributes of a (PSU or fan tray) fan*/
  BASE_PAS_FAN = 1835213,
  BASE_PAS_FAN_OBJ = 1835213,

/*Attributes of an LED*/
  BASE_PAS_LED = 1835214,
  BASE_PAS_LED_OBJ = 1835214,

/*Attributes of a text message display*/
  BASE_PAS_DISPLAY = 1835215,
  BASE_PAS_DISPLAY_OBJ = 1835215,

/*Attributes of a temperature sensor*/
  BASE_PAS_TEMPERATURE = 1835216,
  BASE_PAS_TEMPERATURE_OBJ = 1835216,

/*Attributes of a temperature sensor threshold*/
  BASE_PAS_TEMP_THRESHOLD = 1835217,
  BASE_PAS_TEMP_THRESHOLD_OBJ = 1835217,

/*Attributes of a programmable logic device (PLD)*/
  BASE_PAS_PLD = 1835218,
  BASE_PAS_PLD_OBJ = 1835218,

/*Attributes of a linecard port module*/
  BASE_PAS_PORT_MODULE = 1835219,
  BASE_PAS_PORT_MODULE_OBJ = 1835219,

/*PAS media configuration.*/
  BASE_PAS_MEDIA_CONFIG = 1835220,
  BASE_PAS_MEDIA_CONFIG_OBJ = 1835220,

/*Attributes of an optical media adapter*/
  BASE_PAS_MEDIA = 1835221,
  BASE_PAS_MEDIA_OBJ = 1835221,

/*Attributes of a channel of an optical media adapter*/
  BASE_PAS_MEDIA_CHANNEL = 1835222,
  BASE_PAS_MEDIA_CHANNEL_OBJ = 1835222,

/*Attributes of a port PHY*/
  BASE_PAS_PHY = 1835223,
  BASE_PAS_PHY_OBJ = 1835223,

} BASE_PAS_OBJECTS_t;


#endif
