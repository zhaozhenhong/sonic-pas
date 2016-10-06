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

/**
 * @file pas_data_store.cpp
 *
 * @brief This file contains the API's for accessing the Data store
 **************************************************************************/
#include <map>
#include <string>

#include "cps_api_key.h"
#include "event_log.h"

#include "private/pas_data_store.h"


typedef std::map<std::string, void *> pas_res_map_t;
typedef std::pair<std::string, void *> pas_res_pair_t;
typedef std::pair<pas_res_map_t::iterator,bool> pas_res_return_t;


static pas_res_map_t res_map;

enum {
    /* Offset to ignore qualifier (single digit plus period)
       in the printable-string of an OID.
    */
    IGNORE_QUALIFIER_OFS = 2
};

/*
 * dn_pas_res_insertc is to insert the data in data store.
 *
 * INPUT: 1. key of the resource instance
 *        2. Pointer to an resource data structure.
 *
 * Return value: true on success and false on failure.
 */

bool dn_pas_res_insertc (char *key, void *p_res_obj)
{
    if (!p_res_obj) {
        EV_LOG_ERR(ev_log_t_PAS, 3, "PAS", "%s - %d: Invalid input, p_res_obj: (%p)\n",
                __FUNCTION__, __LINE__, p_res_obj);
        return false;
    }

    pas_res_map_t::iterator it;

    it = res_map.find(key);

    if (it != res_map.end()) {
        EV_LOG_ERR(ev_log_t_PAS, 3, "PAS", "%s - %d: Key already exists, Key: %s\n",
                __FUNCTION__, __LINE__, key);
        return false;
    }

    pas_res_return_t ret;

    ret = res_map.insert (pas_res_pair_t(key, p_res_obj));

    if (ret.second == false) {
        EV_LOG_ERR(ev_log_t_PAS, 3, "PAS", "%s - %d: Insert failed, Key: %s\n",
                __FUNCTION__, __LINE__, key);
        return false;
    }

    return true;
}

/*
 * dn_pas_res_insert is to insert the data in data store. User should
 * pass the CPS key and the pointer to the resource data structure.
 *
 * INPUT: 1. CPS key of the resource instance
 *        2. Pointer to an resource data structure.
 *
 * Return value: true on success and false on failure.
 */

bool dn_pas_res_insert (cps_api_key_t *key, void *p_res_obj)
{
    char key_str [PAS_CPS_KEY_STR_LEN] = "";

    cps_api_key_print(key, key_str, sizeof(key_str));

    return (dn_pas_res_insertc(key_str + IGNORE_QUALIFIER_OFS, p_res_obj));
}

/*
 * dn_pas_res_removec function is to remove resource data and the key
 * from data store
 *
 * INPUT: 1. key of the resource instance
 *
 * Return value: Returns pointer to resource data on success 
 *               otherwise NULL.
 */

void *dn_pas_res_removec (char *key)
{
    pas_res_map_t::iterator it;
    void                    *p_res_obj;

    it = res_map.find(key);

    if (it == res_map.end()) {
        EV_LOG_ERR(ev_log_t_PAS, 3, "PAS", "%s - %d: Key not found, Key: %s\n",
                __FUNCTION__, __LINE__, key);
        return NULL;
    }

    p_res_obj = it->second;

    res_map.erase(it);

    return p_res_obj;
}

/*
 * dn_pas_res_remove function is to remove resource data and the key
 * from data store by specifying CPS key and it returns pointer to
 * data which is removed from data store.
 *
 * INPUT: 1. CPS key of the resource instance
 *
 * Return value: Returns pointer to resource data on success 
 *               otherwise NULL.
 */


void *dn_pas_res_remove (cps_api_key_t *key)
{
    char key_str [PAS_CPS_KEY_STR_LEN] = "";

    cps_api_key_print(key, key_str, sizeof(key_str));

    return (dn_pas_res_removec(key_str + IGNORE_QUALIFIER_OFS));
}

/*
 * dn_pas_res_getc function is to get resource data from data store
 *
 * INPUT: key of the resource instance
 *
 * Return value: Pointer to resource data record, or 0 if not found
 *
 */

void *dn_pas_res_getc (char *key)
{
    pas_res_map_t::iterator it;
    
    it = res_map.find(key);
    
    if (it == res_map.end()) {
        EV_LOG_ERR(ev_log_t_PAS, 3, "PAS", "%s - %d: Key not found, Key: %s\n",
                   __FUNCTION__, __LINE__, key);
        return (0);
    }
    
    return (it->second);
}

/*
 * dn_pas_res_get function is to get resource data from data store by
 * specifying the CPS key and it returns the pointer to the data.
 *
 * INPUT: CPS key of the resource instance
 *
 * Return value: Pointer to resource data record, or 0 if not found
 *
 */

void *dn_pas_res_get (cps_api_key_t *key)
{
    char key_str [PAS_CPS_KEY_STR_LEN] = "";
    
    cps_api_key_print(key, key_str, sizeof(key_str));
    
    return (dn_pas_res_getc(key_str + IGNORE_QUALIFIER_OFS));
}
