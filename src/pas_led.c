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

#include "private/pas_led.h"
#include "private/pas_entity.h"
#include "private/pas_res_structs.h"
#include "private/pas_data_store.h"
#include "private/pas_utils.h"
#include "private/pas_data_store.h"
#include "private/pas_config.h"
#include "private/dn_pas.h"

#include "std_type_defs.h"
#include "std_error_codes.h"
#include "sdi_entity.h"
#include "sdi_led.h"
#include "cps_api_service.h"
#include "dell-base-platform-common.h"
#include "dell-base-pas.h"

#include <stdlib.h>

#define ARRAY_SIZE(a)        (sizeof(a) / sizeof((a)[0]))
#define CALLOC_T(_type, _n)  ((_type *) calloc((_n), sizeof(_type)))


/* Compose a cache key for an LED */

void dn_led_res_key_name(
    char     *buf,
    unsigned bufsize,
    uint_t   entity_type,
    uint_t   slot,
    char     *led_name,
    unsigned led_name_len
                            )
{
    cps_api_key_t key[1];
    uint_t        n;

    cps_api_key_init(key,
                     cps_api_qualifier_OBSERVED,
                     cps_api_obj_CAT_BASE_PAS, 
                     BASE_PAS_LED_OBJ,
                     2,
                     entity_type,
                     slot
                     );

    cps_api_key_print(key, buf, bufsize);

    n = strlen(buf);
    memcpy(buf + n, led_name, led_name_len);
    buf[n + led_name_len] = 0;
}

/* Create an LED cache record */

void dn_cache_init_led(
    sdi_resource_hdl_t sdi_resource_hdl,
    void               *data
                       )
{
    pas_entity_t  *parent = (pas_entity_t *) data;
    const char    *led_name;
    char          ckey[PAS_CPS_KEY_STR_LEN];
    cps_api_key_t cps_key[1];
    pas_led_t     *rec;

    ++parent->num_leds;

    rec = CALLOC_T(pas_led_t, 1);
    if (rec == 0)  return;

    led_name = sdi_resource_alias_get(sdi_resource_hdl);

    rec->parent           = parent;
    rec->led_idx          = parent->num_leds;
    STRLCPY(rec->name, led_name);
    rec->name_len         = strlen(rec->name);
    rec->sdi_resource_hdl = sdi_resource_hdl;

    dn_pas_oper_fault_state_init(rec->oper_fault_state);

    dn_led_res_key_name(ckey, sizeof(ckey), 
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
                     BASE_PAS_LED_OBJ,
                     3,
                     parent->entity_type,
                     parent->slot,
                     rec->led_idx
                     );

    if (!dn_pas_res_insert(cps_key, rec)) {
        dn_pas_res_removec(ckey);
        free(rec);

        return;
    }

    /* Set LED to default */

    struct pas_config_led *cfg = dn_pas_config_led_get(parent->entity_type, rec->name);
    (*(cfg != 0 && cfg->deflt ? sdi_led_on : sdi_led_off))(sdi_resource_hdl);
}

pas_led_t *dn_pas_led_rec_get_name(
    uint_t entity_type,
    uint_t slot,
    char   *led_name,
    uint_t led_name_len
)
{
    char ckey[PAS_CPS_KEY_STR_LEN];

    dn_led_res_key_name(ckey, sizeof(ckey), 
                        entity_type,
                        slot,
                        led_name,
                        led_name_len
                        );

    return ((pas_led_t *) dn_pas_res_getc(ckey));
}

pas_led_t *dn_pas_led_rec_get_idx(
    uint_t entity_type,
    uint_t slot,
    uint_t led_idx
                                                         )
{
    cps_api_key_t rec_key[1];

    cps_api_key_init(rec_key,
                     cps_api_qualifier_OBSERVED,
                     cps_api_obj_CAT_BASE_PAS, 
                     BASE_PAS_LED_OBJ,
                     3,
                     entity_type,
                     slot,
                     led_idx
                     );

    return ((pas_led_t *) dn_pas_res_get(rec_key));
}

/* Delete a temperature sensor cache record */

void dn_cache_del_led(pas_entity_t *parent)
{
    char          ckey[PAS_CPS_KEY_STR_LEN];
    cps_api_key_t cps_key[1];
    uint_t        led_idx;
    pas_led_t     *rec;

    for (led_idx = 1; led_idx <= parent->num_leds; ++led_idx) {
        cps_api_key_init(cps_key,
                         cps_api_qualifier_OBSERVED,
                         cps_api_obj_CAT_BASE_PAS, 
                         BASE_PAS_LED_OBJ,
                         3,
                         parent->entity_type,
                         parent->slot,
                         led_idx
                         );

        rec = dn_pas_res_remove(cps_key);

        dn_led_res_key_name(ckey, sizeof(ckey), 
                            parent->entity_type,
                            parent->slot,
                            rec->name,
                            rec->name_len
                            );
        
        dn_pas_res_removec(ckey);

        free(rec);
    }
}

