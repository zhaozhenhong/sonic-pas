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

/*
 * filename: pas_display_handler.c
 */ 
     
#include "private/pald.h"
#include "private/pas_main.h"
#include "private/pas_res_structs.h"
#include "private/pas_data_store.h"
#include "private/pas_entity.h"
#include "private/pas_display.h"
#include "private/pas_utils.h"
#include "private/pas_config.h"
#include "private/dn_pas.h"

#include "std_error_codes.h"
#include "sdi_led.h"
#include "event_log.h"
#include "dell-base-common.h"
#include "dell-base-platform-common.h"
#include "dell-base-pas.h"

#include <stdlib.h>

#define ARRAY_SIZE(a)  (sizeof(a) / sizeof((a)[0]))


/* Append get response object to given get params for given fully-qualified key */

static t_std_error dn_pas_disp_get1(
    cps_api_get_params_t *param,
    cps_api_qualifier_t  qual,
    pas_display_t        *rec
                                   )
{
    cps_api_object_t resp_obj;
        
    /* Compose respose object */

    resp_obj = cps_api_object_create();
    if (resp_obj == CPS_API_OBJECT_NULL) {
        EV_LOG_ERR(ev_log_t_PAS, 3, "PAS", 
                   "%s, allocate response object failed\n", __FUNCTION__);

        return (STD_ERR(PAS, NOMEM, 0));
    }

    dn_pas_obj_key_display_set(resp_obj,
                               qual,
                               true, rec->parent->entity_type,
                               true, rec->parent->slot,
                               true, rec->name, rec->name_len
                               );

    cps_api_object_attr_add_u8(resp_obj,
                               BASE_PAS_DISPLAY_OPER_STATUS,
                               rec->oper_fault_state->oper_status
                               );
    
    cps_api_object_attr_add_u8(resp_obj,
                               BASE_PAS_DISPLAY_FAULT_TYPE,
                               rec->oper_fault_state->fault_type
                               );
    
    cps_api_object_attr_add(resp_obj,
                            BASE_PAS_DISPLAY_MESSAGE,
                            rec->mesg,
                            rec->mesg != 0 ? strlen(rec->mesg) : 0
                            );
    
    /* Add response object to get response */

    if (!cps_api_object_list_append(param->list, resp_obj)) {
        cps_api_object_delete(resp_obj);

        return (STD_ERR(PAS, FAIL, 0));
    }

    return (STD_ERR_OK);
}

/* Append get response objects to given get params for given LED instances */

t_std_error dn_pas_display_get(cps_api_get_params_t * param, size_t key_idx)
{
    cps_api_object_t         req_obj = cps_api_object_list_get(param->filters, key_idx);
    cps_api_qualifier_t      qual;
    bool                     entity_type_valid, slot_valid, disp_name_valid;
    uint_t                   entity_type, slot, disp_name_len, i, disp_idx;
    uint_t                   slot_start, slot_limit;
    char                     *disp_name;
    struct pas_config_entity *e;
    pas_entity_t             *entity_rec;
    pas_display_t            *disp_rec;

    dn_pas_obj_key_display_get(req_obj,
                               &qual,
                               &entity_type_valid, &entity_type,
                               &slot_valid, &slot,
                               &disp_name_valid,
                               &disp_name, &disp_name_len
                           );
    
    for (i = 0; (e = dn_pas_config_entity_get_idx(i)) != 0; ++i) {
        if (entity_type_valid && e->entity_type != entity_type)  continue;
        
        if (slot_valid) {
            slot_start = slot_limit = slot;
        } else {
            slot_start = 1;
            slot_limit = e->num_slots;
        }

        for (slot = slot_start; slot <= slot_limit; ++slot) {
            if (disp_name_valid) {
                disp_rec = dn_pas_disp_rec_get_name(e->entity_type, slot, disp_name, disp_name_len);
                if (disp_rec == 0)  continue;

                dn_pas_disp_get1(param, qual, disp_rec);
                
                continue;
            }

            entity_rec = dn_pas_entity_rec_get(e->entity_type, slot);
            if (entity_rec == 0)  continue;

            for (disp_idx = 1; disp_idx <= entity_rec->num_displays; ++disp_idx) {
                disp_rec = dn_pas_disp_rec_get_idx(e->entity_type, slot, disp_idx);
                if (disp_rec == 0)  continue;

                dn_pas_disp_get1(param, qual, disp_rec);
            }
        }
    }

    return (STD_ERR_OK);
}

/* Set one LED */

static t_std_error dn_pas_disp_set1(
    cps_api_transaction_params_t *param,
    cps_api_qualifier_t          qual,
    pas_display_t                *rec,
    bool                         mesg_valid,
    char                         *mesg,
    unsigned                     mesg_len
                                   )
{
    cps_api_object_t old_obj;

    /* Add old values, for rollback */

    old_obj = cps_api_object_create();
    if (old_obj == CPS_API_OBJECT_NULL)  return (STD_ERR(PAS, NOMEM, 0));

    dn_pas_obj_key_display_set(old_obj,
                               qual,
                               true, rec->parent->entity_type,
                               true, rec->parent->slot,
                               true, rec->name, rec->name_len
                               );

    cps_api_object_attr_add(old_obj,
                            BASE_PAS_DISPLAY_MESSAGE,
                            rec->mesg,
                            rec->mesg != 0 ? strlen(rec->mesg) : 0
                            );

    if (!cps_api_object_list_append(param->prev, old_obj)) {
        cps_api_object_delete(old_obj);

        return (STD_ERR(PAS, FAIL, 0));
    }

    old_obj = CPS_API_OBJECT_NULL; /* No longer owned */

    if (!mesg_valid)  return (STD_ERR_OK);

    if (rec->mesg != 0) {
        free(rec->mesg);
        rec->mesg = 0;
    }
 
    if (mesg_len == 0) {

        dn_pas_lock();
        if(!dn_pald_diag_mode_get()) {
          
            sdi_digital_display_led_off(rec->sdi_resource_hdl);
        }
        dn_pas_unlock();
       
        return (STD_ERR_OK);
    }

    rec->mesg = malloc(mesg_len + 1);
    if (rec->mesg == 0)  return (STD_ERR(PAS, NOMEM, 0));

    memcpy(rec->mesg, mesg, mesg_len);
    rec->mesg[mesg_len] = 0;

    dn_pas_lock();
    if(!dn_pald_diag_mode_get()) {
        
        sdi_digital_display_led_set(rec->sdi_resource_hdl, rec->mesg); 
        sdi_digital_display_led_on(rec->sdi_resource_hdl);
    }
    dn_pas_unlock();
    
    return (STD_ERR_OK);
}

t_std_error dn_pas_display_set(cps_api_transaction_params_t *param, cps_api_object_t obj)
{
    cps_api_qualifier_t      qual;
    bool                     entity_type_valid, slot_valid, disp_name_valid;
    uint_t                   entity_type, slot, disp_name_len, disp_idx, i;
    uint_t                   slot_start, slot_limit;
    char                     *disp_name;
    cps_api_object_attr_t    a;
    bool                     mesg_valid;
    char                     *mesg;
    uint_t                   mesg_len;
    struct pas_config_entity *e;
    pas_entity_t             *entity_rec;
    pas_display_t            *disp_rec;

    if (cps_api_object_type_operation(cps_api_object_key(obj)) != cps_api_oper_SET) {
        return cps_api_ret_code_ERR;
    }

    a = cps_api_object_attr_get(obj, BASE_PAS_DISPLAY_MESSAGE);
    if ((mesg_valid = (a != CPS_API_ATTR_NULL))) {
        mesg_len = cps_api_object_attr_len(a);
        mesg     = (char *) cps_api_object_attr_data_bin(a);
    }

    dn_pas_obj_key_display_get(obj,
                               &qual,
                               &entity_type_valid, &entity_type,
                               &slot_valid, &slot,
                               &disp_name_valid, &disp_name, &disp_name_len
                               );

    for (i = 0; (e = dn_pas_config_entity_get_idx(i)) != 0; ++i) {
        if (entity_type_valid && e->entity_type != entity_type)  continue;

        if (slot_valid) {
            slot_start = slot_limit = slot;
        } else {
            slot_start = 1;
            slot_limit = e->num_slots;
        }

        for (slot = slot_start; slot <= slot_limit; ++slot) {
            if (disp_name_valid) {
                disp_rec = dn_pas_disp_rec_get_name(e->entity_type,
                                                    slot,
                                                    disp_name,
                                                    disp_name_len
                                                    );
                if (disp_rec == 0)  continue;

                dn_pas_disp_set1(param, qual, disp_rec, mesg_valid, mesg, mesg_len);
                
                continue;
            }

            entity_rec = dn_pas_entity_rec_get(e->entity_type, slot);
            if (entity_rec == 0)  continue;

            for (disp_idx = 1; disp_idx <= entity_rec->num_displays; ++disp_idx) {
                disp_rec = dn_pas_disp_rec_get_idx(e->entity_type, slot, disp_idx);
                if (disp_rec == 0)  continue;

                dn_pas_disp_set1(param, qual, disp_rec, mesg_valid, mesg, mesg_len);
            }
        }
    }

    return (STD_ERR_OK);
}
