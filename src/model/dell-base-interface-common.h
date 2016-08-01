

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

#ifndef DELL_BASE_INTERFACE_COMMON_H
#define DELL_BASE_INTERFACE_COMMON_H

#include "cps_api_operation.h"
#include "dell-base-common.h"
#include <stdint.h>
#include <stdbool.h>


#define cps_api_obj_CAT_BASE_IF (20) 

#define DELL_BASE_INTERFACE_COMMON_MODEL_STR "dell-base-interface-common"

/*Null interface type*/
#define IF_INTERFACE_TYPE_IANAIFT_IANA_INTERFACE_TYPE_BASE_IF_NULL  "base-if:null"
/* 
Name: base-if:mll-violation-actions
Type:enumeration
Comments:Configurable mac learn limit (mll) actions when mac learn limit is violated
*/

/*Management interface type*/
#define IF_INTERFACE_TYPE_IANAIFT_IANA_INTERFACE_TYPE_BASE_IF_MANAGEMENT  "base-if:management"
/* 
Name: base-if:msm-violation
Type:enumeration
Comments:MAC station move (msm) violation actions.
*/

/*CPU interface type*/
#define IF_INTERFACE_TYPE_IANAIFT_IANA_INTERFACE_TYPE_BASE_IF_CPU  "base-if:cpu"

/* Enumeration base-if:mode */

typedef enum { 
  BASE_IF_MODE_MODE_NONE = 1, 
  BASE_IF_MODE_MODE_L2 = 2, /*The port only supports L2.*/
  BASE_IF_MODE_MODE_L2HYBRID = 3, /*The port supports L2 hybrid.*/
  BASE_IF_MODE_MODE_L3 = 4, /*The port only supports L3.*/
  BASE_IF_MODE_MIN=1,
  BASE_IF_MODE_MAX=4,
} BASE_IF_MODE_t;
/* Enumeration base-if:lacp-mode-type */

typedef enum { 
  BASE_IF_LACP_MODE_TYPE_ACTIVE = 1, 
  BASE_IF_LACP_MODE_TYPE_PASSIVE = 2, 
  BASE_IF_LACP_MODE_TYPE_MIN=1,
  BASE_IF_LACP_MODE_TYPE_MAX=2,
} BASE_IF_LACP_MODE_TYPE_t;
/* Enumeration base-if:lag-mode-type */

typedef enum { 
  BASE_IF_LAG_MODE_TYPE_STATIC = 1, 
  BASE_IF_LAG_MODE_TYPE_DYNAMIC = 2, 
  BASE_IF_LAG_MODE_TYPE_MIN=1,
  BASE_IF_LAG_MODE_TYPE_MAX=2,
} BASE_IF_LAG_MODE_TYPE_t;
/* Enumeration base-if:mac-learn-limit/msm-violation */

typedef enum { 
  BASE_IF_MAC_LEARN_LIMIT_MSM_VIOLATION_LOG = 1, /*Send the Mac-station-move-violation notification to syslog*/
  BASE_IF_MAC_LEARN_LIMIT_MSM_VIOLATION_OFFENDING_SHUTDOWN = 2, /*Shutdown the offending interface*/
  BASE_IF_MAC_LEARN_LIMIT_MSM_VIOLATION_ORIGINAL_SHUTDOWN = 3, /*Shutdown the original interface*/
  BASE_IF_MAC_LEARN_LIMIT_MSM_VIOLATION_BOTH_SHUTDOWN = 4, /*Disable both the interfaces*/
  BASE_IF_MAC_LEARN_LIMIT_MSM_VIOLATION_MIN=1,
  BASE_IF_MAC_LEARN_LIMIT_MSM_VIOLATION_MAX=4,
} BASE_IF_MAC_LEARN_LIMIT_MSM_VIOLATION_t;
/* Enumeration base-if:mac-learn-limit/mll-violation-actions */

typedef enum { 
  BASE_IF_MAC_LEARN_LIMIT_MLL_VIOLATION_ACTIONS_LOG = 1, /*Send the Mac-learn-limit violation notification to syslog*/
  BASE_IF_MAC_LEARN_LIMIT_MLL_VIOLATION_ACTIONS_SHUTDOWN = 2, /*Shutdown the Interface*/
  BASE_IF_MAC_LEARN_LIMIT_MLL_VIOLATION_ACTIONS_MIN=1,
  BASE_IF_MAC_LEARN_LIMIT_MLL_VIOLATION_ACTIONS_MAX=2,
} BASE_IF_MAC_LEARN_LIMIT_MLL_VIOLATION_ACTIONS_t;
/* Enumeration base-if:speed */

typedef enum { 
  BASE_IF_SPEED_0MBPS = 0, /*0Mbps data rate.*/
  BASE_IF_SPEED_10MBPS = 1, /*10Mbps data rate.*/
  BASE_IF_SPEED_100MBPS = 2, /*100Mbps data rate.*/
  BASE_IF_SPEED_1GIGE = 3, /*1G data rate.*/
  BASE_IF_SPEED_10GIGE = 4, /*10G data rate.*/
  BASE_IF_SPEED_25GIGE = 5, /*25G data rate.*/
  BASE_IF_SPEED_40GIGE = 6, /*40G data rate.*/
  BASE_IF_SPEED_100GIGE = 7, /*100G data rate.*/
  BASE_IF_SPEED_AUTO = 8, /*The speed is dependent on the media type.*/
  BASE_IF_SPEED_MIN=0,
  BASE_IF_SPEED_MAX=8,
} BASE_IF_SPEED_t;

/* No objects defined.. */

#endif
