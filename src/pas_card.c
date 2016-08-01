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

#include "private/pald.h"
#include "private/pas_entity.h"
#include "private/pas_res_structs.h"
#include "private/pas_data_store.h"
#include "private/pas_event.h"
#include "private/pas_config.h"
#include "private/pas_utils.h"

#include "std_type_defs.h"
#include "std_error_codes.h"
#include "sdi_entity.h"
#include "cps_api_service.h"

#include <stdlib.h>

/* Initialize the cache entries for cards */

#define CALLOC_T(_type, _n)  ((_type *) calloc((_n), sizeof(_type)))

bool dn_cache_init_card(void)
{
    uint_t        slot, n;
    cps_api_key_t key[1];
    pas_entity_t  *parent;
    pas_card_t    *data;

    for (slot = 1, n = sdi_entity_count_get(SDI_ENTITY_SYSTEM_BOARD); n > 0; --n, ++slot) {
        cps_api_key_init(key,
                         cps_api_qualifier_OBSERVED,
                         cps_api_obj_CAT_BASE_PAS, 
                         BASE_PAS_ENTITY_OBJ,
                         2,
                         PLATFORM_ENTITY_TYPE_CARD,
                         slot
                         );

        parent = (pas_entity_t *) dn_pas_res_get(key);
        if (parent == 0) {
            return (false);
        }

        data = CALLOC_T(pas_card_t, 1);
        if (data == 0) {
            return (false);
        }
        
        data->parent = parent;

        data->card_type = dn_pas_config_card_get()->type;

        cps_api_key_init(key,
                         cps_api_qualifier_OBSERVED,
                         cps_api_obj_CAT_BASE_PAS, 
                         BASE_PAS_CARD_OBJ,
                         1,
                         slot
                         );

        if (!dn_pas_res_insert(key, data)) {
            free(data);

            return (false);
        }
    }

    return (true);
}
