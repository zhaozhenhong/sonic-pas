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

/***************************************************************************
 * LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
 *
 * This source code is confidential, proprietary, and contains trade
 * secrets that are the sole property of Dell Inc.
 * Copy and/or distribution of this source code or disassembly or reverse
 * engineering of the resultant object code are strictly forbidden without
 * the written consent of Dell Inc.
 *
 ***************************************************************************/

/**************************************************************************
 * @file pas_media_channel_handler.c
 * 
 * @brief This file contains source code of physical media handling.
 ***************************************************************************/

#include "private/pas_media.h"
#include "private/pas_main.h"
#include "private/pald.h"
#include "private/pas_utils.h"
#include "private/pas_config.h"
#include "private/dn_pas.h"
#include "event_log.h"

t_std_error dn_pas_media_channel_get(
        cps_api_get_params_t * param,
        size_t key_ix)
{
    cps_api_qualifier_t     qualifier;
    uint_t                  slot, port, port_module, channel;
    bool                    slot_valid, port_module_valid, port_valid;
    bool                    channel_valid;
    cps_api_object_t        req_obj;
    t_std_error             ret = STD_ERR_OK;


    req_obj = cps_api_object_list_get(param->filters, key_ix);
    STD_ASSERT(req_obj != NULL);

    dn_pas_obj_key_media_channel_get(req_obj, &qualifier, &slot_valid,
            &slot, &port_module_valid, &port_module, &port_valid, &port,
            &channel_valid, &channel);

    if (slot_valid == false) {

        //TODO hadle chassis case when slot number is not part of 
        //CPS key, CP should initiate collecting from all line cards
        dn_pas_myslot_get(&slot);
    }

    if (port_valid == true) {
        if (channel_valid == true) {
            if (dn_pas_channel_get(qualifier, slot, port, channel, param,
                        req_obj) == false) {

                EV_LOG_INFO(ev_log_t_PAS, 3, "PAS", "dn_pas_channel_get\
                        failed, port(%d), channel(%d)\n", port, channel);
                return STD_ERR(PAS, FAIL, 0);
            }
        } else {
            if (dn_pas_port_channel_get(qualifier, slot, port, param, req_obj)
                    == false) {
                EV_LOG_INFO(ev_log_t_PAS, 3, "PAS",
                        "dn_pas_port_channel_get failed, port(%d).\n", port);
                return STD_ERR(PAS, FAIL, 0);
            }
        }
    } else {
        for (port = PAS_MEDIA_START_PORT; port <= dn_phy_media_count_get();
                port++) {
            if (dn_pas_port_channel_get(qualifier, slot, port, param, req_obj)
                    == false) {

                EV_LOG_INFO(ev_log_t_PAS, 3, "PAS",
                        "dn_pas_port_channel_get failed, port(%d).\n", port);

                ret = STD_ERR(PAS, FAIL, 0);
                continue;
            }
        }
    }

    return ret;
}

t_std_error dn_pas_media_channel_set(cps_api_transaction_params_t * param,
        cps_api_object_t obj)
{
    uint_t                  channel, port, port_module, slot;
    bool                    slot_valid, port_module_valid, port_valid;
    bool                    channel_valid;
    cps_api_qualifier_t     qualifier;
    t_std_error             ret = STD_ERR_OK;
    cps_api_object_it_t     it;
    cps_api_attr_id_t       attr_id;
    bool                    status;
    uint_t                  start, end, ch_start, ch_end;
    cps_api_object_t        cloned;
    phy_media_tbl_t         *mtbl = NULL;
    cps_api_operation_types_t operation;

    dn_pas_lock();
    
    if(dn_pald_diag_mode_get()) {
        
        dn_pas_unlock();
        return STD_ERR(PAS, FAIL, 0);
    }
    
    if ((param == NULL) || (obj == NULL)) {

        EV_LOG_INFO(ev_log_t_PAS, 3, "PAS", "Invalid param, \
                obj (%p)\n", obj);
        dn_pas_unlock();
        return STD_ERR(PAS, FAIL, 0);
    }

    operation = cps_api_object_type_operation(cps_api_object_key(obj));

    dn_pas_obj_key_media_channel_get(obj, &qualifier, &slot_valid, &slot,
            &port_module_valid, &port_module, &port_valid, &port, 
            &channel_valid, &channel);

    if (slot_valid == false) {
        dn_pas_myslot_get(&slot);
    }

    if (port_valid == false) {
        start = PAS_MEDIA_START_PORT;
        end = dn_phy_media_count_get();
    } else {
        start = end = port;
    }

    for ( ; (start <= end); start++) {

        if ((mtbl = dn_phy_media_entry_get(start)) == NULL) {
             EV_LOG_INFO(ev_log_t_PAS, 3, "PAS", "Invalid port(%d)\n", port);
             dn_pas_unlock();
             return STD_ERR(PAS, FAIL, 0);
        }

        if (channel_valid == false) {
            ch_start = PAS_MEDIA_CH_START;
            ch_end = mtbl->channel_cnt;
        } else {

            if (channel >= mtbl->channel_cnt) {
                dn_pas_unlock();
                return STD_ERR(PAS, FAIL, 0);
            }

            ch_start = channel;
            ch_end = ch_start + 1;
        }

        for ( ; ch_start < ch_end; ch_start++) {

            uint_t              count = 0;
            BASE_IF_SPEED_t     supported_speed[MAX_SUPPORTED_SPEEDS];

            memset(supported_speed, 0, sizeof(supported_speed));

            if ((cloned = cps_api_object_create()) == NULL) {
                EV_LOG_INFO(ev_log_t_PAS, 3, "PAS", "Object create failed\n");
                dn_pas_unlock();
                return STD_ERR(PAS, NOMEM, 0);
            }

            if (dn_pas_media_populate_current_data(BASE_PAS_MEDIA_CHANNEL_OBJ,
                        obj, cloned, start, ch_start) == false) {

                EV_LOG_INFO(ev_log_t_PAS, 3, "PAS", "Object clone failed.\n");
                cps_api_object_delete(cloned);
                dn_pas_unlock();
                return STD_ERR(PAS, FAIL, 0);
            }

            dn_pas_obj_key_media_channel_set(cloned, qualifier, true, slot, false,
                    PAS_MEDIA_INVALID_PORT_MODULE, true, start, true, ch_start);

            if (cps_api_object_list_append(param->prev, cloned) == false) {
                EV_LOG_INFO(ev_log_t_PAS, 3, "PAS",
                        "Object append to list failed.\n");
                cps_api_object_delete(cloned);
                dn_pas_unlock();
                return STD_ERR(PAS, FAIL, 0);
            }

            cps_api_object_it_begin(obj,&it);

            while (cps_api_object_it_valid (&it)) {

                attr_id = cps_api_object_attr_id (it.attr);

                switch (attr_id) {
                    case BASE_PAS_MEDIA_CHANNEL_STATE:
                        {
                            pas_media_phy_defaults  *phy_config_entry = NULL;

                            memcpy(&status,
                                    cps_api_object_attr_data_bin(it.attr),
                                    sizeof(status));

                            mtbl->channel_data[ch_start].tgt_state = status;

                            struct pas_config_media *cfg = dn_pas_config_media_get();

                            if ((cfg->lockdown == true)
                                    && (mtbl->res_data->dell_qualified == false)
                                    && dn_pas_is_capability_10G_plus(mtbl->res_data->capability)
                                    &&(status == true)) {
                                status = false;
                            }

                            if (dn_pas_media_channel_state_set(start, ch_start,
                                        status) == false) {

                                ret = STD_ERR(PAS, FAIL, 0);
                            }

                            if (status == true) {
                                phy_config_entry = dn_pas_media_phy_config_entry_get(mtbl->res_data->type);

                                if (phy_config_entry != NULL) {

                                    if (dn_pas_media_phy_interface_mode_set(start, ch_start) == false) {
                                        EV_LOG_INFO(ev_log_t_PAS, 3, "PAS", "phy interface mode set failed\n");
                                    }

                                    if (dn_pas_media_phy_autoneg_set(start, ch_start) == false) {
                                        EV_LOG_INFO(ev_log_t_PAS, 3, "PAS", "phy autoneg set failed\n");
                                    }

                                    if (dn_pas_media_phy_supported_speed_set(start, ch_start) == false) {
                                        EV_LOG_INFO(ev_log_t_PAS, 3, "PAS", "phy supported speed set failed\n");
                                    }
                                }
                            }
                            break;
                        }
                    case BASE_PAS_MEDIA_CHANNEL_SPEED:
                        {
                            BASE_IF_SPEED_t         speed;
                            struct pas_config_media *media_cfg = NULL;

                            if (((media_cfg = dn_pas_config_media_get()) == NULL)
                                    || (media_cfg->led_control == false)) {
                                EV_LOG_INFO(ev_log_t_PAS, 3, "PAS",
                                        "Led control not supported.\n");
                                ret = STD_ERR(PAS, FAIL, 0);
                            } else {

                                speed = cps_api_object_attr_data_uint(it.attr);

                                if (dn_pas_media_channel_led_set (start, ch_start,
                                            speed) == false) {
                                    ret = STD_ERR(PAS, FAIL, 0);
                                }
                                mtbl->channel_data[ch_start].speed = speed;
                            }
                            break;
                        }
                    case BASE_PAS_MEDIA_CHANNEL_AUTONEG:
                        {
                            bool autoneg;
                            memcpy(&autoneg, cps_api_object_attr_data_bin(it.attr),
                                    sizeof(autoneg));

                            if (dn_pas_media_phy_autoneg_config_set(start, ch_start, autoneg,
                                        operation) == false) {
                                ret = STD_ERR(PAS, FAIL, 0);
                            }
                            break;
                        }
                    case BASE_PAS_MEDIA_CHANNEL_SUPPORTED_SPEED:
                        {
                            supported_speed[count++] = cps_api_object_attr_data_uint(it.attr);
                            break;
                        }
                    default:
                        {
                            if (dn_pas_media_is_key_attr(
                                        BASE_PAS_MEDIA_CHANNEL_OBJ,
                                        attr_id) != true) {

                                EV_LOG_INFO(ev_log_t_PAS, 3, "PAS",
                                        "Invalid attr_id attr_id (%d)\n",
                                        attr_id);
                                ret = STD_ERR(PAS, FAIL, 0);
                            }
                        }
                }
                cps_api_object_it_next (&it);
            }
            if (count > 0) {
                if (dn_pas_media_phy_supported_speed_config_set(start, ch_start,
                            supported_speed, count, operation) == false) {
                    ret = STD_ERR(PAS, FAIL, 0);
                }
            }
        }
    }
    dn_pas_unlock();
    return ret;
}

