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

#include "private/pas_display.h"
#include "private/pas_entity.h"
#include "private/pas_res_structs.h"
#include "private/pas_data_store.h"
#include "private/pas_utils.h"
#include "private/pas_data_store.h"
#include "private/dn_pas.h"

#include "std_type_defs.h"
#include "std_error_codes.h"
#include "sdi_entity.h"
#include "cps_api_service.h"
#include "dell-base-platform-common.h"
#include "dell-base-pas.h"

#include <stdlib.h>

#define ARRAY_SIZE(a)        (sizeof(a) / sizeof((a)[0]))
#define CALLOC_T(_type, _n)  ((_type *) calloc((_n), sizeof(_type)))


/* Compose a cache key for a display */

void dn_disp_res_key_name(
    char     *buf,
    unsigned bufsize,
    uint_t   entity_type,
    uint_t   slot,
    char     *disp_name,
    unsigned disp_name_len
                            )
{
    cps_api_key_t key[1];
    uint_t        n;

    cps_api_key_init(key,
                     cps_api_qualifier_OBSERVED,
                     cps_api_obj_CAT_BASE_PAS, 
                     BASE_PAS_DISPLAY_OBJ,
                     2,
                     entity_type,
                     slot
                     );

    cps_api_key_print(key, buf, bufsize);

    n = strlen(buf);
    memcpy(buf + n, disp_name, disp_name_len);
    buf[n + disp_name_len] = 0;
}

/* Create a display cache record */

void dn_cache_init_disp(
    sdi_resource_hdl_t sdi_resource_hdl,
    void               *data
                       )
{
    pas_entity_t  *parent = (pas_entity_t *) data;
    const char    *disp_name;
    char          ckey[PAS_CPS_KEY_STR_LEN];
    cps_api_key_t cps_key[1];
    pas_display_t *rec;

    ++parent->num_displays;

    rec = CALLOC_T(pas_display_t, 1);
    if (rec == 0)  return;

    disp_name = sdi_resource_alias_get(sdi_resource_hdl);

    rec->parent           = parent;
    rec->disp_idx         = parent->num_displays;
    STRLCPY(rec->name, disp_name);
    rec->name_len         = strlen(rec->name);
    rec->sdi_resource_hdl = sdi_resource_hdl;

    dn_pas_oper_fault_state_init(rec->oper_fault_state);

    dn_disp_res_key_name(ckey, sizeof(ckey), 
                         parent->entity_type,
                         parent->slot,
                         rec->name,
                         rec->name_len
                         );

    if (!dn_pas_res_insertc(ckey, rec)) {
        free(rec);

        return;
    }

    cps_api_key_init(cps_key,
                     cps_api_qualifier_OBSERVED,
                     cps_api_obj_CAT_BASE_PAS, 
                     BASE_PAS_DISPLAY_OBJ,
                     3,
                     parent->entity_type,
                     parent->slot,
                     rec->disp_idx
                     );

    if (!dn_pas_res_insert(cps_key, rec)) {
        dn_pas_res_removec(ckey);
        free(rec);
    }
}

pas_display_t *dn_pas_disp_rec_get_name(
    uint_t entity_type,
    uint_t slot,
    char   *disp_name,
    uint_t disp_name_len
)
{
    char ckey[PAS_CPS_KEY_STR_LEN];

    dn_disp_res_key_name(ckey, sizeof(ckey), 
                         entity_type,
                         slot,
                         disp_name,
                         disp_name_len
                         );

    return ((pas_display_t *) dn_pas_res_getc(ckey));
}

pas_display_t *dn_pas_disp_rec_get_idx(
    uint_t entity_type,
    uint_t slot,
    uint_t disp_idx
                                                         )
{
    cps_api_key_t rec_key[1];

    cps_api_key_init(rec_key,
                     cps_api_qualifier_OBSERVED,
                     cps_api_obj_CAT_BASE_PAS, 
                     BASE_PAS_DISPLAY_OBJ,
                     3,
                     entity_type,
                     slot,
                     disp_idx
                     );

    return ((pas_display_t *) dn_pas_res_get(rec_key));
}

/* Delete a temperature sensor cache record */

void dn_cache_del_disp(pas_entity_t *parent)
{
    char          ckey[PAS_CPS_KEY_STR_LEN];
    cps_api_key_t cps_key[1];
    uint_t        disp_idx;
    pas_display_t *rec;

    for (disp_idx = 1; disp_idx <= parent->num_displays; ++disp_idx) {
        cps_api_key_init(cps_key,
                         cps_api_qualifier_OBSERVED,
                         cps_api_obj_CAT_BASE_PAS, 
                         BASE_PAS_DISPLAY_OBJ,
                         3,
                         parent->entity_type,
                         parent->slot,
                         disp_idx
                         );

        rec = dn_pas_res_remove(cps_key);

        dn_disp_res_key_name(ckey, sizeof(ckey), 
                             parent->entity_type,
                             parent->slot,
                             rec->name,
                             rec->name_len
                             );
        
        dn_pas_res_removec(ckey);

        free(rec);
    }
}

