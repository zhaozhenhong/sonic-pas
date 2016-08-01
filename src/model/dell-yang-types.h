

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

#ifndef DELL_YANG_TYPES_H
#define DELL_YANG_TYPES_H

#include "cps_api_operation.h"
#include <stdint.h>
#include <stdbool.h>


#define cps_api_obj_CAT_COMMON (40) 

#define DELL_YANG_TYPES_MODEL_STR "dell-yang-types"

/*
Comments:vrf id value.
*/
typedef uint32_t COMMON_VRF_ID_t;
/*
Comments:The decimal or float value type
*/
typedef double COMMON_XFLOAT_t;
/*
Comments:The as-number type in dot format represents autonomous system numbers
which identify an Autonomous System.
*/
typedef const char* COMMON_AS_NUMBER_DOT_t;
/*
Comments:Name of the interface as in ifName
*/
typedef const char* COMMON_INTERFACE_NAME_t;
/* 
Name: common:interface-types
Type:union
Comments:
*/

/*
Comments:One vlan (1..4094) or a range separated by comma:1,2,3
*/
typedef const char* COMMON_VLAN_RANGE_SEPARATED_BY_COMMA_t;
/*
Comments:The name of VLAN.
*/
typedef const char* COMMON_VLAN_NAME_t;
/*
Comments:The name of the vrf(virtual routing and forwarding).
*/
typedef const char* COMMON_VRF_NAME_t;
/* 
Name: common:ethertype
Type:union
Comments:<600-FFFF>;;Ethertype hex value.
*/

/* 
Name: common:ip-mask-or-prefix
Type:union
Comments:
*/

/* 
Name: common:ipv6-mask-or-prefix
Type:union
Comments:
*/

/*
Comments:The number or id of VLAN.
*/
typedef uint16_t COMMON_VLAN_ID_t;
/*
Comments:Name of the access group or list
*/
typedef const char* COMMON_POLICY_NAME_t;
/*
Comments:percentage type
*/
typedef uint8_t COMMON_PERCENT_t;
/*
Comments:One vlan (1..4094) or a range separated by dash: 1-1 to 4094-4094
*/
typedef const char* COMMON_VLAN_RANGE_t;
/* 
Name: common:ipv4-mask-or-prefix
Type:union
Comments:
*/


/* Enumeration common:admin-status */

typedef enum { 
  COMMON_ADMIN_STATUS_ADMIN_UP = 1, 
  COMMON_ADMIN_STATUS_ADMIN_DOWN = 2, 
  COMMON_ADMIN_STATUS_MIN=1,
  COMMON_ADMIN_STATUS_MAX=2,
} COMMON_ADMIN_STATUS_t;
/* Enumeration common:protocol-name */

typedef enum { 
  COMMON_PROTOCOL_NAME_BGP = 1, 
  COMMON_PROTOCOL_NAME_DHCP = 2, 
  COMMON_PROTOCOL_NAME_DHCP_RELAY = 3, 
  COMMON_PROTOCOL_NAME_FTP = 4, 
  COMMON_PROTOCOL_NAME_ICMP = 5, 
  COMMON_PROTOCOL_NAME_IGMP = 6, 
  COMMON_PROTOCOL_NAME_MSDP = 7, 
  COMMON_PROTOCOL_NAME_NTP = 8, 
  COMMON_PROTOCOL_NAME_OSPF = 9, 
  COMMON_PROTOCOL_NAME_PIM = 10, 
  COMMON_PROTOCOL_NAME_RIP = 11, 
  COMMON_PROTOCOL_NAME_SSH = 12, 
  COMMON_PROTOCOL_NAME_TELNET = 13, 
  COMMON_PROTOCOL_NAME_VRRP = 14, 
  COMMON_PROTOCOL_NAME_IP = 15, 
  COMMON_PROTOCOL_NAME_TCP = 16, 
  COMMON_PROTOCOL_NAME_UDP = 17, 
  COMMON_PROTOCOL_NAME_IPV6 = 18, 
  COMMON_PROTOCOL_NAME_ARP = 19, 
  COMMON_PROTOCOL_NAME_BRIDGING = 20, 
  COMMON_PROTOCOL_NAME_CDP = 21, 
  COMMON_PROTOCOL_NAME_IS_IS = 22, 
  COMMON_PROTOCOL_NAME_SCP = 23, 
  COMMON_PROTOCOL_NAME_HTTP = 24, 
  COMMON_PROTOCOL_NAME_HTTPS = 25, 
  COMMON_PROTOCOL_NAME_FRRP = 26, 
  COMMON_PROTOCOL_NAME_GVRP = 27, 
  COMMON_PROTOCOL_NAME_LACP = 28, 
  COMMON_PROTOCOL_NAME_LLDP = 29, 
  COMMON_PROTOCOL_NAME_STP = 30, 
  COMMON_PROTOCOL_NAME_MIN=1,
  COMMON_PROTOCOL_NAME_MAX=30,
} COMMON_PROTOCOL_NAME_t;
/* Enumeration common:direction-enumeration */

typedef enum { 
  COMMON_DIRECTION_ENUMERATION_IN = 1, 
  COMMON_DIRECTION_ENUMERATION_OUT = 2, 
  COMMON_DIRECTION_ENUMERATION_BOTH = 3, 
  COMMON_DIRECTION_ENUMERATION_MIN=1,
  COMMON_DIRECTION_ENUMERATION_MAX=3,
} COMMON_DIRECTION_ENUMERATION_t;
/* Enumeration common:oper-status */

typedef enum { 
  COMMON_OPER_STATUS_UNKNOWN = 1, 
  COMMON_OPER_STATUS_UP = 2, 
  COMMON_OPER_STATUS_DOWN = 3, 
  COMMON_OPER_STATUS_TESTING = 4, 
  COMMON_OPER_STATUS_NOT_PRESENT = 5, 
  COMMON_OPER_STATUS_DISABLED = 6, 
  COMMON_OPER_STATUS_DIAG = 7, 
  COMMON_OPER_STATUS_LINKUP = 8, 
  COMMON_OPER_STATUS_MIN=1,
  COMMON_OPER_STATUS_MAX=8,
} COMMON_OPER_STATUS_t;
/* Enumeration common:phy-intf-types */

typedef enum { 
  COMMON_PHY_INTF_TYPES_TENGIGABITETHERNET = 1, 
  COMMON_PHY_INTF_TYPES_FORTYGIGABITETNERNET = 2, 
  COMMON_PHY_INTF_TYPES_GIGABITETHERNET = 3, 
  COMMON_PHY_INTF_TYPES_MIN=1,
  COMMON_PHY_INTF_TYPES_MAX=3,
} COMMON_PHY_INTF_TYPES_t;
/* Enumeration common:log-intf-types */

typedef enum { 
  COMMON_LOG_INTF_TYPES_LOGICAL = 1, 
  COMMON_LOG_INTF_TYPES_CPU = 2, 
  COMMON_LOG_INTF_TYPES_LAG = 3, 
  COMMON_LOG_INTF_TYPES_NULL = 4, 
  COMMON_LOG_INTF_TYPES_LOOPBACK = 5, 
  COMMON_LOG_INTF_TYPES_PORT_CHANNEL = 6, 
  COMMON_LOG_INTF_TYPES_VLAN = 7, 
  COMMON_LOG_INTF_TYPES_L2_TUNNEL = 8, 
  COMMON_LOG_INTF_TYPES_MIN=1,
  COMMON_LOG_INTF_TYPES_MAX=8,
} COMMON_LOG_INTF_TYPES_t;

/* No objects defined.. */

#endif
