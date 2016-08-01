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

/*********************************************************************
 * LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
 *
 * This source code is confidential, proprietary, and contains trade
 * secrets that are the sole property of Dell Inc.
 * Copy and/or distribution of this source code or disassembly or reverse
 * engineering of the resultant object code are strictly forbidden without
 * the written consent of Dell Inc.
 *
*********************************************************************/

/*********************************************************************
 * @file pas_data_store.h
 * @brief This file contains function prototypes to access 
 *        pas data store.
 *
 ********************************************************************/

#ifndef __PAS_DATA_STORE_H
#define __PAS_DATA_STORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cps_api_key.h"

enum {
    PAS_CPS_KEY_STR_LEN = 1024
};


/*
 * dn_pas_res_insertc is to insert the data in data store.
 */

bool dn_pas_res_insertc (char *key, void *p_res_obj);

/*
 * dn_pas_res_insert is to insert the data in data store. User should
 * pass the CPS key and the pointer to the resource data structure.
 */

bool dn_pas_res_insert (cps_api_key_t *key, void *p_res_obj);

/*
 * dn_pas_res_removec function is to remove resource data and the key 
 * from data store
 */

void *dn_pas_res_removec (char *key);

/*
 * dn_pas_res_remove function is to remove resource data and the key 
 * from data store by specifying CPS key and it returns pointer to
 * data which is removed from data store.
 */

void *dn_pas_res_remove (cps_api_key_t * key);

/*
 * dn_pas_res_getc function is to get resource data from data store
 */

void *dn_pas_res_getc (char *key);

/*
 * dn_pas_res_get function is to get resource data from data store by
 * specifying the CPS key and it returns the pointer to the data.
 */

void *dn_pas_res_get (cps_api_key_t * key);

#ifdef __cplusplus
}
#endif


#endif  //__PAS_DATA_STORE_H
