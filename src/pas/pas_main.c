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
 * filename: pas_main.c
 */ 
     
#include <unistd.h>

#include "event_log.h"
#include "dell-base-common.h"
#include "dell-base-pas.h"

#include "private/pas_main.h"
#include "private/pald.h"


/************************************************************************
 *
 * Name: dn_pas_read_function
 *
 *      This function is to handle cps get request
 *
 * Input: context pointer
 *        cps_api_get_params_t pointer
 *        cps key index
 *
 * Return Values: cps_api_ret_code_OK or cps_api_ret_code_ERR
 * -------------
 *
 ************************************************************************/
cps_api_return_code_t dn_pas_read_function (void *context, 
                                            cps_api_get_params_t *param, 
                                            size_t key_ix) 
{
    cps_api_object_t obj      = cps_api_object_list_get(param->filters, key_ix);
    cps_api_key_t    *the_key = cps_api_object_key(obj);
    uint_t           cat      = cps_api_key_get_cat(the_key);
    uint_t           sub_cat  = cps_api_key_get_subcat(the_key);

    if (cat == cps_api_obj_CAT_BASE_PAS) {
      switch (sub_cat) {
        case BASE_PAS_CHASSIS_OBJ:
              dn_pas_chassis_get(param, key_ix);
              break;
    
        case BASE_PAS_ENTITY_OBJ:
              dn_pas_entity_get(param, key_ix);
              break;

        case BASE_PAS_PSU_OBJ:
              dn_pas_psu_get(param, key_ix);
              break;

        case BASE_PAS_FAN_TRAY_OBJ:
              dn_pas_fan_tray_get(param, key_ix);
              break;

        case BASE_PAS_CARD_OBJ:
              dn_pas_card_get(param, key_ix);
              break;
    
        case BASE_PAS_FAN_OBJ:
              dn_pas_fan_get(param, key_ix);
              break;

        case BASE_PAS_LED_OBJ:
              dn_pas_led_get(param, key_ix);
              break;

        case BASE_PAS_DISPLAY_OBJ:
              dn_pas_display_get(param, key_ix);
              break;

        case BASE_PAS_TEMPERATURE_OBJ:
              dn_pas_temperature_get(param, key_ix);
              break;

        case BASE_PAS_TEMP_THRESHOLD_OBJ:
              dn_pas_temp_threshold_get(param, key_ix);
              break;

        case BASE_PAS_PLD_OBJ:
              dn_pas_pld_get(param, key_ix);
              break;

        case BASE_PAS_PORT_MODULE_OBJ:
              dn_pas_port_module_get(param, key_ix);
              break;

        case BASE_PAS_MEDIA_OBJ:
              dn_pas_media_get(param, key_ix);
              break;

        case BASE_PAS_MEDIA_CHANNEL_OBJ:
              dn_pas_media_channel_get(param, key_ix);
              break;

        case BASE_PAS_MEDIA_CONFIG_OBJ:
              dn_pas_media_config_get(param, key_ix);
              break;

        case BASE_PAS_PHY_OBJ:
              dn_pas_phy_get(param, key_ix);
              break;
 
        case BASE_PAS_READY_OBJ:
              dn_pas_status_get(param, key_ix);
              break;
     
        default:
              EV_LOG_ERR(ev_log_t_PAS, 3, "PAS", 
                         "%s undefined PAS subcategory %d !\n", 
                         __FUNCTION__, sub_cat); 
              return cps_api_ret_code_ERR;
      }
    }
    else {
           EV_LOG_ERR(ev_log_t_PAS, 3, "PAS", 
                     "%s, the category %d not for PAS\n",
                     __FUNCTION__, cat);
    }
    return cps_api_ret_code_OK;
}

/************************************************************************
 *
 * Name: dn_pas_write_function
 *
 *      This function is to handle cps set request
 *
 * Input: context pointer
 *        cps_api_transaction_params_t pointer
 *        change list index
 *
 * Return Values: cps_api_ret_code_OK or cps_api_ret_code_ERR
 * -------------
 *
 ************************************************************************/
cps_api_return_code_t dn_pas_write_function(void *context, 
                                            cps_api_transaction_params_t *param, 
                                            size_t index_of_element_being_updated) 
{
    cps_api_key_t    *the_key = NULL;
    cps_api_object_t obj     = CPS_API_OBJECT_NULL;
    uint_t           cat;
    uint_t           sub_cat;

    obj = cps_api_object_list_get(param->change_list,index_of_element_being_updated);
    STD_ASSERT(obj != CPS_API_OBJECT_NULL);
    the_key  = cps_api_object_key(obj);

    if (cps_api_key_get_qual(the_key) != cps_api_qualifier_TARGET) {
        /* Attempt to set object not in target space */

        return (cps_api_ret_code_ERR);
    }

    cat      = cps_api_key_get_cat(the_key);
    sub_cat  = cps_api_key_get_subcat(the_key);

    if (cat == cps_api_obj_CAT_BASE_PAS) {
        switch (sub_cat) {
            case BASE_PAS_CHASSIS_OBJ:
                  dn_pas_chassis_set(param, obj);
                  break;
        
            case BASE_PAS_ENTITY_OBJ:
                  dn_pas_entity_set(param, obj);
                  break;

            case BASE_PAS_PSU_OBJ:
                  dn_pas_psu_set(the_key, obj);
                  break;

            case BASE_PAS_FAN_TRAY_OBJ:
                  dn_pas_fan_tray_set(the_key, obj);
                  break;

            case BASE_PAS_CARD_OBJ:
                  dn_pas_card_set(the_key, obj);
                  break;
        
            case BASE_PAS_FAN_OBJ:
                  dn_pas_fan_set(param, obj);
                  break;

            case BASE_PAS_LED_OBJ:
                  dn_pas_led_set(param, obj);
                  break;

            case BASE_PAS_DISPLAY_OBJ:
                  dn_pas_display_set(param, obj);
                  break;

            case BASE_PAS_TEMPERATURE_OBJ:
                  dn_pas_temperature_set(param, obj);
                  break;

            case BASE_PAS_TEMP_THRESHOLD_OBJ:
                  dn_pas_temp_threshold_set(param, obj);
                  break;

            case BASE_PAS_PLD_OBJ:
                  dn_pas_pld_set(the_key, obj);
                  break;

            case BASE_PAS_PORT_MODULE_OBJ:
                  dn_pas_port_module_set(the_key, obj);
                  break;

            case BASE_PAS_MEDIA_OBJ:
                  dn_pas_media_set(param, obj);
                  break;

            case BASE_PAS_MEDIA_CHANNEL_OBJ:
                  dn_pas_media_channel_set(param, obj);
                  break;

            case BASE_PAS_MEDIA_CONFIG_OBJ:
                  dn_pas_media_config_set(param, obj);
                  break;

            case BASE_PAS_PHY_OBJ:
                  dn_pas_phy_set(the_key, obj);
                  break;
         
            default:
                  EV_LOG_ERR(ev_log_t_PAS, 3, "PAS", 
                             "%s, undefined PAS subcategory %d !\n", 
                             __FUNCTION__, sub_cat); 
                  return cps_api_ret_code_ERR;
        }
    }
    else {
           EV_LOG_ERR(ev_log_t_PAS, 3, "PAS", "%s, the category %d not for PAS\n", 
                     __FUNCTION__, cat);
    }
    return cps_api_ret_code_OK;
}

/************************************************************************
 *
 * Name: dn_pas_rollback_function
 *
 *      This function is to handle cps rollback
 *
 * Input: context pointer
 *        cps_api_transaction_params_t pointer
 *        change list index
 *
 * Return Values: cps_api_ret_code_OK or cps_api_ret_code_ERR
 * -------------
 *
 ************************************************************************/
cps_api_return_code_t dn_pas_rollback_function(void *context, 
                                               cps_api_transaction_params_t *param, 
                                               size_t index_of_element_being_updated) 
{
    return cps_api_ret_code_OK;
}


/*******************************************************************************
 *
 * Name: dn_pas_main_thread 
 *
 *      pas main thread
 *      It will handle cps get and set request
 *
 * Input: None
 *
 * Return Values: cps_api_ret_code_OK or cps_api_ret_code_ERR
 * -------------
 *
 *******************************************************************************/
t_std_error dn_pas_main_thread()
{
    while (1) {
        pause();
    }
    return STD_ERR_OK; 
}
