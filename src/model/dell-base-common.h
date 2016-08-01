

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

#ifndef DELL_BASE_COMMON_H
#define DELL_BASE_COMMON_H

#include "cps_api_operation.h"
#include <stdint.h>
#include <stdbool.h>


#define cps_api_obj_CAT_BASE_CMN (11) 

#define DELL_BASE_COMMON_MODEL_STR "dell-base-common"

/*
Comments:Uniquely identifies any interface system-wide including
physical ports, LAG interfaces and Vlan interfaces
*/
typedef uint32_t BASE_CMN_LOGICAL_IFINDEX_t;
/*
Comments:Traffic class, i.e. internal QoS level
*/
typedef uint8_t BASE_CMN_TRAFFIC_CLASS_t;
/*
Comments:LAG opaque data blob that Application obtains from
LAG module and passes to other Base modules.
*/
typedef uint8_t* BASE_CMN_LAG_OPAQUE_DATA_t;
/*
Comments:This type denotes a IPv4 address as a 4 octet binary number
in network-byte order.
*/
typedef uint8_t* BASE_CMN_IPV4_ADDRESS_t;
/*
Comments:Uniquely identifies a Network Processing Unit hardware
in the system
*/
typedef uint32_t BASE_CMN_NPU_ID_t;
/*
Comments:Meter info opaque data blob that Application obtains from
QoS module and passes to other Base modules.
*/
typedef uint8_t* BASE_CMN_QOS_METER_OPAQUE_DATA_t;
/*
Comments:3-bit of priority field in 802.1Q header
*/
typedef uint8_t BASE_CMN_DOT1P_t;
/*
Comments:Common type for object IDs returned to Applications
*/
typedef uint64_t BASE_CMN_BASE_OBJ_ID_TYPE_t;
/*
Comments:Identifies a port within a Network Processing Unit
*/
typedef uint32_t BASE_CMN_NPU_PORT_ID_t;
/*
Comments:Mirroring session opaque data blob that Application obtains from
Mirroring module and passes to other Base modules.
*/
typedef uint8_t* BASE_CMN_MIRROR_OPAQUE_DATA_t;
/*
Comments:Queue info opaque data blob that Application obtains from
QoS module and passes to other Base modules.
*/
typedef uint8_t* BASE_CMN_QOS_QUEUE_OPAQUE_DATA_t;
/*
Comments:Identifier of the NAS mirror-session
*/
typedef uint32_t BASE_CMN_MIRROR_ID_t;
/*
Comments:This type denotes a MAC address as a 6 octet binary number
in network-byte order.
*/
typedef uint8_t* BASE_CMN_MAC_ADDRESS_t;
/*
Comments:Logical Switch Identifier. Logical switch is a grouping
of one or more NPU(s) in the system. Each NPU can belong
to only one logical switch
*/
typedef uint32_t BASE_CMN_LOGICAL_SWITCH_ID_t;
/* 
Name: base-cmn:ip-address
Type:union
Comments:This type denotes a IP address (V4 or V6) 4/16 binary number
in network-byte order.
*/

/*
Comments:This type denotes a IPv6 address as a 16 octet binary number
in network-byte order.
*/
typedef uint8_t* BASE_CMN_IPV6_ADDRESS_t;
/*
Comments:IP Nexthop group info opaque data blob that Application obtains
from Routing module and passes to other Base modules.
*/
typedef uint8_t* BASE_CMN_IP_NEXTHOP_GROUP_OPAQUE_DATA_t;
/*
Comments:This type denotes a IEEE 802.1Q VLAN Identifier.
*/
typedef uint16_t BASE_CMN_VLAN_ID_t;

/* Enumeration base-cmn:loopback-type */

typedef enum { 
  BASE_CMN_LOOPBACK_TYPE_NONE = 1, 
  BASE_CMN_LOOPBACK_TYPE_PHY = 2, 
  BASE_CMN_LOOPBACK_TYPE_MAC = 3, 
  BASE_CMN_LOOPBACK_TYPE_MIN=1,
  BASE_CMN_LOOPBACK_TYPE_MAX=3,
} BASE_CMN_LOOPBACK_TYPE_t;
/* Enumeration base-cmn:oper-status-type */

typedef enum { 
  BASE_CMN_OPER_STATUS_TYPE_UP = 1, /*Ready to pass packets.*/
  BASE_CMN_OPER_STATUS_TYPE_DOWN = 2, /*Entity is down.*/
  BASE_CMN_OPER_STATUS_TYPE_TESTING = 3, /*In some test mode.  No operational packets can
be passed.*/
  BASE_CMN_OPER_STATUS_TYPE_UNKNOWN = 4, /*Status cannot be determined for some reason.*/
  BASE_CMN_OPER_STATUS_TYPE_DORMANT = 5, /*Waiting for some external event.*/
  BASE_CMN_OPER_STATUS_TYPE_NOT_PRESENT = 6, /*Some component (typically hardware) is missing.*/
  BASE_CMN_OPER_STATUS_TYPE_LOWER_LAYER_DOWN = 7, /*Down due to state of lower-layer interface(s).*/
  BASE_CMN_OPER_STATUS_TYPE_FAIL = 8, /*The interface has an internal failure.*/
  BASE_CMN_OPER_STATUS_TYPE_MIN=1,
  BASE_CMN_OPER_STATUS_TYPE_MAX=8,
} BASE_CMN_OPER_STATUS_TYPE_t;
/* Enumeration base-cmn:duplex-type */

typedef enum { 
  BASE_CMN_DUPLEX_TYPE_FULL = 1, 
  BASE_CMN_DUPLEX_TYPE_HALF = 2, 
  BASE_CMN_DUPLEX_TYPE_AUTO = 3, 
  BASE_CMN_DUPLEX_TYPE_MIN=1,
  BASE_CMN_DUPLEX_TYPE_MAX=3,
} BASE_CMN_DUPLEX_TYPE_t;
/* Enumeration base-cmn:stat-type */

typedef enum { 
  BASE_CMN_STAT_TYPE_PORT_STAT = 1, 
  BASE_CMN_STAT_TYPE_FLOW_STAT = 2, 
  BASE_CMN_STAT_TYPE_PREFIX_STAT = 3, 
  BASE_CMN_STAT_TYPE_QUEUE_STAT = 4, 
  BASE_CMN_STAT_TYPE_BUFFER_STAT = 5, 
  BASE_CMN_STAT_TYPE_INTERFACE_STAT = 6, 
  BASE_CMN_STAT_TYPE_MIN=1,
  BASE_CMN_STAT_TYPE_MAX=6,
} BASE_CMN_STAT_TYPE_t;
/* Enumeration base-cmn:admin-status-type */

typedef enum { 
  BASE_CMN_ADMIN_STATUS_TYPE_UP = 1, 
  BASE_CMN_ADMIN_STATUS_TYPE_DOWN = 2, 
  BASE_CMN_ADMIN_STATUS_TYPE_TESTING = 3, 
  BASE_CMN_ADMIN_STATUS_TYPE_MIN=1,
  BASE_CMN_ADMIN_STATUS_TYPE_MAX=3,
} BASE_CMN_ADMIN_STATUS_TYPE_t;
/* Enumeration base-cmn:af-type */

typedef enum { 
  BASE_CMN_AF_TYPE_INET = 0, 
  BASE_CMN_AF_TYPE_INET6 = 1, 
  BASE_CMN_AF_TYPE_MIN=0,
  BASE_CMN_AF_TYPE_MAX=1,
} BASE_CMN_AF_TYPE_t;
/* Enumeration base-cmn:traffic-path */

typedef enum { 
  BASE_CMN_TRAFFIC_PATH_INGRESS = 1, /*Enable sampling on Ingress packets*/
  BASE_CMN_TRAFFIC_PATH_EGRESS = 2, /*Enable sampling of Egress packets*/
  BASE_CMN_TRAFFIC_PATH_INGRESS_EGRESS = 3, /*Enable sampling of Ingress and Egress packets*/
  BASE_CMN_TRAFFIC_PATH_MIN=1,
  BASE_CMN_TRAFFIC_PATH_MAX=3,
} BASE_CMN_TRAFFIC_PATH_t;
/* Enumeration base-cmn:breakout-type */

typedef enum { 
  BASE_CMN_BREAKOUT_TYPE_DISABLED = 1, 
  BASE_CMN_BREAKOUT_TYPE_BREAKOUT_4X1 = 2, 
  BASE_CMN_BREAKOUT_TYPE_BREAKOUT_2X1 = 3, 
  BASE_CMN_BREAKOUT_TYPE_BREAKOUT_1X1 = 4, 
  BASE_CMN_BREAKOUT_TYPE_MIN=1,
  BASE_CMN_BREAKOUT_TYPE_MAX=4,
} BASE_CMN_BREAKOUT_TYPE_t;
/* Enumeration base-cmn:tun-type */

typedef enum { 
  BASE_CMN_TUN_TYPE_IPV4_TUNNEL = 1, 
  BASE_CMN_TUN_TYPE_IPV6_TUNNEL = 2, 
  BASE_CMN_TUN_TYPE_GRE_TUNNEL = 3, 
  BASE_CMN_TUN_TYPE_VXLAN_TUNNEL = 4, 
  BASE_CMN_TUN_TYPE_NVGRE_TUNNEL = 5, 
  BASE_CMN_TUN_TYPE_ERSPAN_TUNNEL = 6, 
  BASE_CMN_TUN_TYPE_MPLS_TUNNEL = 7, 
  BASE_CMN_TUN_TYPE_MIN=1,
  BASE_CMN_TUN_TYPE_MAX=7,
} BASE_CMN_TUN_TYPE_t;
/* Enumeration base-cmn:lag-hash-type */

typedef enum { 
  BASE_CMN_LAG_HASH_TYPE_SRC_IP = 1, 
  BASE_CMN_LAG_HASH_TYPE_DEST_IP = 2, 
  BASE_CMN_LAG_HASH_TYPE_SRC_DEST_IP = 3, 
  BASE_CMN_LAG_HASH_TYPE_SRC_PORT = 4, 
  BASE_CMN_LAG_HASH_TYPE_MIN=1,
  BASE_CMN_LAG_HASH_TYPE_MAX=4,
} BASE_CMN_LAG_HASH_TYPE_t;
/* Enumeration base-cmn:encap-type */

typedef enum { 
  BASE_CMN_ENCAP_TYPE_TAGGED = 1, 
  BASE_CMN_ENCAP_TYPE_UNTAGGED = 2, 
  BASE_CMN_ENCAP_TYPE_PRIO_TAGGED = 3, 
  BASE_CMN_ENCAP_TYPE_MIN=1,
  BASE_CMN_ENCAP_TYPE_MAX=3,
} BASE_CMN_ENCAP_TYPE_t;
/* Enumeration base-cmn:interface-type */

typedef enum { 
  BASE_CMN_INTERFACE_TYPE_L3_PORT = 1, 
  BASE_CMN_INTERFACE_TYPE_L2_PORT = 2, 
  BASE_CMN_INTERFACE_TYPE_LOOPBACK = 3, 
  BASE_CMN_INTERFACE_TYPE_NULL = 4, 
  BASE_CMN_INTERFACE_TYPE_TUNNEL = 5, 
  BASE_CMN_INTERFACE_TYPE_SVI = 6, 
  BASE_CMN_INTERFACE_TYPE_CPU = 7, 
  BASE_CMN_INTERFACE_TYPE_MANAGEMENT = 8, 
  BASE_CMN_INTERFACE_TYPE_ETHERNET = 9, 
  BASE_CMN_INTERFACE_TYPE_VLAN = 10, 
  BASE_CMN_INTERFACE_TYPE_LAG = 11, 
  BASE_CMN_INTERFACE_TYPE_MIN=1,
  BASE_CMN_INTERFACE_TYPE_MAX=11,
} BASE_CMN_INTERFACE_TYPE_t;

/* No objects defined.. */

#endif
