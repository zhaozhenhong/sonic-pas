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
 * file : pas_fuse_display_led.c
 * brief: pas daemon interface layer to display_led SDI API
 * date : 04/2015
 *
 */


#include "private/pas_fuse_handlers.h"


/*
 * PAS Daemon display_led read interface 
 */
int dn_pas_fuse_display_led_read(
        dev_node_t *node, 
        char       *buf,
        size_t     size, 
        off_t      offset
        )
{
 
    int    res                               = -ENOTSUP;
    size_t len                               = 0;
    char   trans_buf[FUSE_FILE_DEFAULT_SIZE] = { 0 };
    memset(buf, 0, size);

    /** check for node & buffer validity */
    if ((NULL == node) || (NULL == buf)) {
        
        return res;
    }

    /** switch on filetype */
    switch (node->fuse_filetype) {

        /** alias */
        case FUSE_DISPLAY_LED_FILETYPE_ALIAS:
            {

                const char *alias_name;
                if (NULL != (alias_name = sdi_resource_alias_get(node->fuse_resource_hdl))) {

                    dn_pas_fuse_print(trans_buf, FUSE_FILE_DEFAULT_SIZE, &len, &res, 
                            "%-25s : %s", "Alias", (char *) alias_name);
                }
                break;
            }

        /** diagnostic mode */
        case FUSE_DISPLAY_LED_FILETYPE_DIAG_MODE:
            {

                dn_pas_fuse_print(trans_buf, FUSE_FILE_DEFAULT_SIZE, &len, &res, 
                        "%-25s : %s", "Diagnostic Mode", (dn_pald_diag_mode_get() ? "up" : "down"));
                break;
            }

        default:
            {    
                if(node->fuse_filetype >= FUSE_DISPLAY_LED_FILETYPE_MIN && 
                   node->fuse_filetype < FUSE_DISPLAY_LED_FILETYPE_MAX) {
                    
                    res = -EPERM;
                } else {
                    
                    res = -ENOENT;
                }
                break;
            }
    }

    if (offset < len) {

        size = dn_pas_fuse_calc_size(len, size, offset);
        memcpy(buf, &trans_buf[offset], size);
        res = size;
    }

    return res;
}



/*
 * PAS Daemon display_led write interface 
 */
int dn_pas_fuse_display_led_write(
        dev_node_t *node, 
        const char *buf,
        size_t     size, 
        off_t      offset
        )
{
    int    res   = -ENOTSUP;
    uint_t state = 0;

    /** check node validity */
    if ((NULL == node) || (NULL == buf)) {
        
        return res;
    }

    switch (node->fuse_filetype) {
        
        /** get the display string */
        case FUSE_DISPLAY_LED_FILETYPE_STRING:
            {
                if (STD_ERR_OK ==
                        sdi_digital_display_led_set(node->fuse_resource_hdl, buf)) {
                    
                    res = size;
                }
                break;
            }


        /** set the display led ON/OFF */
        case FUSE_DISPLAY_LED_FILETYPE_STATE:
            {
                if (true == dn_pas_fuse_atoui(buf, &state)) {

                    if(state != 1 && state != 0) {

                        return -EINVAL;
                    }

                    if (state == 1 
                            && STD_ERR_OK == sdi_digital_display_led_on(node->fuse_resource_hdl)) {

                        res = size;

                    } else if (state == 0 
                            && STD_ERR_OK == sdi_digital_display_led_off(node->fuse_resource_hdl)) {

                        res = size;
                    }
                }
                break;
            }


        /** diagnostic mode */
        case FUSE_DISPLAY_LED_FILETYPE_DIAG_MODE:
            {
                if (dn_pas_fuse_atoui(buf, &state)) {

                    switch(state) {
                        case 1  : /** TODO: define variable and set to true */
                                  res = size;
                                  break;
                        
                        case 0  : /** TODO: define variable and set to false */
                                  res = size;
                                  break;
                        
                        default : res = -EINVAL;
                                  break;
                    }
                
                } else res = -EINVAL;
                
                break;
            }

        default:
            {    
                if(node->fuse_filetype >= FUSE_DISPLAY_LED_FILETYPE_MIN && 
                   node->fuse_filetype < FUSE_DISPLAY_LED_FILETYPE_MAX) {
                    
                    res = -EPERM;
                } else {
                    
                    res = -ENOENT;
                }
                break;
            }
    }
    
    return res;
}
