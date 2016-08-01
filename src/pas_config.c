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

#include "private/pas_config.h"
#include "private/pas_main.h"
#include "private/pas_utils.h"
#include "private/dn_pas.h"

#include "std_config_node.h"
#include "dell-base-platform-common.h"
#include "dell-base-pas.h"
#include "cps_api_key.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

#define ARRAY_SIZE(a)  (sizeof(a) / sizeof((a)[0]))
#define END(a)         (&(a)[ARRAY_SIZE(a)])
#define CALLOC_T(n, t)  ((t *) calloc((n), sizeof(t)))

typedef struct config_entry_s {
    char *name;
    void (*func)(std_config_node_t);
} config_entry_t;

static char pas_media_app_cfg_filename[] = "/etc/pas/media-config.xml";

static void dn_pas_media_read_app_config (std_config_node_t nd);
static void dn_pas_config_parse (const char *config_filename,
        cps_api_operation_handle_t _cps_hdl,
        config_entry_t *entry_tbl, uint_t size);

static cps_api_operation_handle_t cps_hdl;

static sdi_entity_info_t chassis_cfg[1];

static void dn_pas_config_chassis(std_config_node_t nd)
{
    char *a;

    if ((a = std_config_attr_get(nd, "vendor-name")) != 0) {
        STRLCPY(chassis_cfg->vendor_name, a);
    }
    if ((a = std_config_attr_get(nd, "product-name")) != 0) {
        STRLCPY(chassis_cfg->prod_name, a);
    }
    if ((a = std_config_attr_get(nd, "hw-version")) != 0) {
        STRLCPY(chassis_cfg->hw_revision, a);
    }
    if ((a = std_config_attr_get(nd, "platform-name")) != 0) {
        STRLCPY(chassis_cfg->platform_name, a);
    }
    if ((a = std_config_attr_get(nd, "ppid")) != 0) {
        STRLCPY(chassis_cfg->ppid, a);
    }
    if ((a = std_config_attr_get(nd, "service-tag")) != 0) {
        STRLCPY(chassis_cfg->service_tag, a);
    }
    if ((a = std_config_attr_get(nd, "base-mac-addresses")) != 0
        && sscanf(a, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                  &chassis_cfg->base_mac[0],
                  &chassis_cfg->base_mac[1],
                  &chassis_cfg->base_mac[2],
                  &chassis_cfg->base_mac[3],
                  &chassis_cfg->base_mac[4],
                  &chassis_cfg->base_mac[5]
                  )
        != 6
        ) {
        memset(chassis_cfg->base_mac, 0, sizeof(chassis_cfg->base_mac));
    }
    if ((a = std_config_attr_get(nd, "num-mac-addresses")) != 0
        && sscanf(a, "%u", &chassis_cfg->mac_size) != 1
        ) {
        chassis_cfg->mac_size = 0;
    }
}

sdi_entity_info_t *dn_pas_config_chassis_get(void)
{
    return (chassis_cfg);
}

static struct pas_config_entity entity_cfg_tbl[] = {
    { entity_type:     PLATFORM_ENTITY_TYPE_PSU,
      sdi_entity_type: SDI_ENTITY_PSU_TRAY,
      name:            "psu",
      poll_interval:   1000, 
      fan: {
        margin: 10,
        incr:   1,
        decr:   10,
        limit:  30
        }
    },
    { entity_type:     PLATFORM_ENTITY_TYPE_FAN_TRAY,
      sdi_entity_type: SDI_ENTITY_FAN_TRAY,
      name:            "fan-tray",
      poll_interval:   1000, 
      fan: {
        margin: 10,
        incr:   1,
        decr:   10,
        limit:  30
        }
    },
    { entity_type:     PLATFORM_ENTITY_TYPE_CARD,
      sdi_entity_type: SDI_ENTITY_SYSTEM_BOARD,
      name:            "card",
      poll_interval:   1000
    }
};

/* Find the entity-type attribute in the given node */

static struct pas_config_entity *dn_pas_config_entity_find_attr(std_config_node_t nd)
{
    char                     *a;
    struct pas_config_entity *e;

    a = std_config_attr_get(nd, "entity-type");
    if (a == 0)  return (0);

    for (e = entity_cfg_tbl; e < END(entity_cfg_tbl); ++e) {
        if (strcmp(e->name, a) == 0)  return (e);
    }

    return (0);
}

/* Read configuration for an entity */

static void dn_pas_config_entity(std_config_node_t nd)
{
    struct pas_config_entity *e;
    char                     *a;

    e = dn_pas_config_entity_find_attr(nd);
    if (e == 0)  return;

    e->num_slots = sdi_entity_count_get(e->sdi_entity_type);

    a = std_config_attr_get(nd, "poll-interval");
    if (a != 0)  sscanf(a, "%u", &e->poll_interval);
}

/* Lookup the configuration for the given entity */

struct pas_config_entity *dn_pas_config_entity_get_type(uint_t entity_type)
{
    struct pas_config_entity *e;

    for (e = entity_cfg_tbl; e < END(entity_cfg_tbl); ++e) {
        if (e->entity_type == entity_type)  return (e);
    }
    
    return (0);
}

/* Lookup the configuration for the index */

struct pas_config_entity *dn_pas_config_entity_get_idx(uint_t idx)
{
    return (idx >= ARRAY_SIZE(entity_cfg_tbl) ? 0 : &entity_cfg_tbl[idx]);
}



static struct pas_config_card card_cfg[1];

/* Read configuration for a card */

static void dn_pas_config_card(std_config_node_t nd)
{
    char *a, *p, *fmt;

    a = std_config_attr_get(nd, "type");
    if (a == 0)  return;
    
    if (strlen(a) >= 3 && a[0] == '0' && (a[1] | 0x20) == 'x') {
        p   = &a[2];
        fmt = "%x";
    } else {
        p   = a;
        fmt = "%u";
    }
    if (sscanf(p, fmt, &card_cfg->type) == 1)  card_cfg->valid = true;
}

/* Lookup the configuration for a card */

struct pas_config_card *dn_pas_config_card_get(void)
{
    return (card_cfg);
}


/* Read configuration for a fan */

static void dn_pas_config_fan(std_config_node_t nd)
{
    struct pas_config_entity *e;
    char                     *a;

    e = dn_pas_config_entity_find_attr(nd);
    if (e == 0) {}

    a = std_config_attr_get(nd, "margin");
    if (a != 0)  sscanf(a, "%u", &e->fan.margin);

    a = std_config_attr_get(nd, "incr");
    if (a != 0)  sscanf(a, "%u", &e->fan.incr);

    a = std_config_attr_get(nd, "decr");
    if (a != 0)  sscanf(a, "%u", &e->fan.decr);

    a = std_config_attr_get(nd, "limit");
    if (a != 0)  sscanf(a, "%u", &e->fan.limit);
}



struct led_pri_node {
    struct led_pri_node   *next;
    char                  *led_name;
    struct pas_config_led cfg[1];
};

struct led_group_node {
    uint_t                entity_type;
    struct led_group_node *next;
    struct led_pri_node   *pri_list;
};

static struct led_group_node *led_groups;

/* Create an LED group */

static struct led_group_node *led_group_create(uint_t entity_type)
{
    struct led_group_node *grp;

    grp = CALLOC_T(1, struct led_group_node);
    if (grp != 0) {
        grp->entity_type = entity_type;
        
        grp->next = led_groups;
        led_groups = grp;
    }

    return (grp);
}

/* Add an LED to the given LED group */

static void led_pri_add(struct led_group_node *grp, char *name, bool deflt)
{
    struct led_pri_node *pri;

    pri = CALLOC_T(1, struct led_pri_node);
    if (pri == 0)  return;

    pri->led_name = strdup(name);
    if (pri->led_name == 0) {
        free(pri);

        return;
    }
    pri->cfg->deflt = deflt;

    pri->next = grp->pri_list;
    grp->pri_list = pri;
}

/* Lookup LED config */

struct pas_config_led *dn_pas_config_led_get(
    uint_t entity_type,
    char   *name
                                             )
{
    struct led_group_node *p;
    struct led_pri_node   *q;

    for (p = led_groups; p != 0; p = p->next) {
        if (p->entity_type != entity_type)  continue;

        for (q = p->pri_list; q != 0; q = q->next) {
            if (strcmp(name, q->led_name) == 0) {
                return (q->cfg);
            }
        }
    }
    
    return (0);
}

/* Lookup the first LED in the same group as the given LED */

void *dn_pas_config_led_group_iter_first(uint_t entity_type, char *name)
{
    struct led_group_node *p;
    struct led_pri_node   *q;

    for (p = led_groups; p != 0; p = p->next) {
        if (p->entity_type != entity_type)  continue;

        for (q = p->pri_list; q != 0; q = q->next) {
            if (strcmp(name, q->led_name) == 0) {
                return (p->pri_list);
            }
        }
    }

    return (0);
}

/* Lookup the name of the LED for the given iterator */

char *dn_pas_config_led_group_iter_name(void *iter)
{
    return (((struct led_pri_node *) iter)->led_name);
}

/* Get the next LED in the group for the given iterator */

void *dn_pas_config_led_group_iter_next(void *iter)
{
    return (((struct led_pri_node *) iter)->next);
}

/* Read configuration for LED grouping */

static void dn_pas_config_led_groups(std_config_node_t nd)
{
    struct pas_config_entity *e;
    std_config_node_t        *grpnd, *lednd;
    struct led_group_node    *grp;

    /* Create the group */

    e = dn_pas_config_entity_find_attr(nd);
    if (e == 0)  return;

    for (grpnd = std_config_get_child(nd);
         grpnd != 0;
         grpnd = std_config_next_node(grpnd)
         ) {
        if (strcmp(std_config_name_get(grpnd), "led-group") != 0) {
            continue;
        }
    
        grp = led_group_create(e->entity_type);
        if (grp == 0)  continue;

        /* For each child node, add LED to the group */
        
        for (lednd = std_config_get_child(grpnd);
             lednd != 0;
             lednd = std_config_next_node(lednd)
             ) {
            if (strcmp(std_config_name_get(lednd), "led") != 0) {
                continue;
            }
            
            char *id = std_config_attr_get(lednd, "id");
            if (id == 0) {
                return;
            }
            char *deflt = std_config_attr_get(lednd, "default");
            
            led_pri_add(grp, id,
                        deflt != 0 && (strcmp(deflt, "1") == 0
                                       || strcasecmp(deflt, "on") == 0
                                       )
                        );
        }
    }
}


static struct pas_config_temperature cfg_temperature[1] = {
    { num_thresholds: 20 }
};

static void dn_pas_config_temp(std_config_node_t nd)
{
    char *a;

    a = std_config_attr_get(nd, "num_thresholds");
    if (a == 0)  return;

    sscanf(a, "%u", &cfg_temperature->num_thresholds);
}

struct pas_config_temperature *dn_pas_config_temperature_get(void)
{
    return (cfg_temperature);
}

static config_entry_t media_app_cfg_tbl [] = {{"media", dn_pas_media_read_app_config}};

static struct pas_config_media cfg_media[1] = {
    { poll_interval: 1000, rtd_interval: 5, lockdown: false, led_control: false }
};

/* Read configuration for optical media modules */

static void dn_pas_config_media(std_config_node_t nd)
{
    char *a;

    a = std_config_attr_get(nd, "poll-interval");
    if (a != 0) {
        sscanf(a, "%u", &cfg_media->poll_interval);
    }

    a = std_config_attr_get(nd, "rtd-interval");
    if (a != NULL) {
        sscanf(a, "%u", &cfg_media->rtd_interval);
    }

    a = std_config_attr_get(nd, "led-control");
    if (a != NULL) {
        if (strcmp(a, "software") == 0) {
            cfg_media->led_control = true;
        }
    }
    if (access(pas_media_app_cfg_filename, F_OK) == 0) {
        dn_pas_config_parse(pas_media_app_cfg_filename, NULL, media_app_cfg_tbl,
                ARRAY_SIZE(media_app_cfg_tbl));
    }
}

/* Read media configuration from apps platform config file */


static void dn_pas_media_read_app_config (std_config_node_t nd)
{
    char *a = NULL;


    a = std_config_attr_get(nd, "lockdown");
    if (a != NULL) {
        if (strcmp(a, "enable") == 0) {
            cfg_media->lockdown = true;
        }
    }
}

/* Lookup configuration for optical media modules */

struct pas_config_media *dn_pas_config_media_get(void)
{
    return (cfg_media);
}

static pas_config_media_phy cfg_media_phy[1] = {{ 0, NULL }};

/* Returns media PHY configuration defaults */

pas_config_media_phy *dn_pas_config_media_phy_get(void)
{
    return (cfg_media_phy);
}

/* Read media PHY configuration defaults */

static void dn_pas_media_read_phy_default_config (std_config_node_t phy_config)
{

    std_config_node_t  nd;
    uint_t             cnt = 0;
    const char         *nm;

    for (nd = std_config_get_child(phy_config); nd != 0; nd = std_config_next_node(nd)) {
        nm = std_config_name_get(nd);

        if (strcmp(nm, "phy-config-for-media-type") == 0) {
            cnt += 1;
        }
    }

    if (cnt > 0) {
        cfg_media_phy->count = cnt;

        cfg_media_phy->media_phy_defaults = (pas_media_phy_defaults *) calloc(
                cfg_media_phy->count, sizeof(pas_media_phy_defaults));

        if (cfg_media_phy->media_phy_defaults == NULL) {

            cfg_media_phy->count = 0;
            return;
        }

        for (nd = std_config_get_child(phy_config), cnt = 0; nd != 0;
                nd = std_config_next_node(nd), ++cnt) {
            nm = std_config_name_get(nd);

            if (strcmp(nm, "phy-config-for-media-type") == 0) {
                std_config_node_t  cnd;
                const char         *cnm;
                char               *val_str;

                val_str = std_config_attr_get(nd, "type");
                if (val_str != 0) {
                    STRLCPY(cfg_media_phy->media_phy_defaults[cnt].media_type_str,
                            val_str);

                } else continue;

                val_str = std_config_attr_get(nd, "type-enum-value");
                if (val_str != 0) {
                     sscanf(val_str, "%u",
                             &cfg_media_phy->media_phy_defaults[cnt].media_type);

                } else continue;

                for (cnd = std_config_get_child(nd); cnd != 0; cnd = std_config_next_node(cnd)) {

                    cnm = std_config_name_get(cnd);

                    if (strcmp(cnm, "internal-phy") == 0) {

                        val_str = std_config_attr_get(cnd, "mode");
                        if (val_str != 0) {
                            STRLCPY(cfg_media_phy->media_phy_defaults[cnt].interface_mode,
                                    val_str);
                        }

                        val_str = std_config_attr_get(cnd, "autoneg");
                        if (val_str != 0) {
                            cfg_media_phy->media_phy_defaults[cnt].intrl_phy_autoneg
                                = val_str[0] == '0' ? false : true;
                        }

                        val_str = std_config_attr_get(cnd, "supported-speeds");
                        if (val_str != 0) {
                            char     *token = NULL;
                            char     *saveptr  = NULL;
                            uint_t   indx;
                            uint_t   speed;
                            char     unit;

                            token = strtok_r(val_str, "/", &saveptr);
                            for (indx = 0; indx < MAX_SUPPORTED_SPEEDS && token != NULL;
                                    token = strtok_r(NULL, "/", &saveptr)) {
                                sscanf(token, "%u", &speed);

                                unit = token[strlen(token) - 1];
                                if (isalpha(unit) == false) continue;

                                switch (unit) {
                                    case 'M' :
                                        if (speed == 10) {
                                            cfg_media_phy->media_phy_defaults[cnt].intrl_phy_supported_speed[indx++]
                                                = BASE_IF_SPEED_10MBPS;
                                        } else if (speed == 100) {
                                            cfg_media_phy->media_phy_defaults[cnt].intrl_phy_supported_speed[indx++]
                                                = BASE_IF_SPEED_100MBPS;
                                        } else if (speed == 1000) {
                                            cfg_media_phy->media_phy_defaults[cnt].intrl_phy_supported_speed[indx++]
                                                = BASE_IF_SPEED_1GIGE;
                                        } else continue;
                                        break;
                                    case 'G':
                                        if (speed == 1) {
                                            cfg_media_phy->media_phy_defaults[cnt].intrl_phy_supported_speed[indx++]
                                                = BASE_IF_SPEED_1GIGE;
                                        } else if (speed == 10) {
                                            cfg_media_phy->media_phy_defaults[cnt].intrl_phy_supported_speed[indx++]
                                                = BASE_IF_SPEED_10GIGE;
                                        } else if (speed == 25) {
                                            cfg_media_phy->media_phy_defaults[cnt].intrl_phy_supported_speed[indx++]
                                                = BASE_IF_SPEED_25GIGE;
                                        } else if (speed == 40) {
                                            cfg_media_phy->media_phy_defaults[cnt].intrl_phy_supported_speed[indx++]
                                                = BASE_IF_SPEED_40GIGE;
                                        } else if (speed == 100) {
                                            cfg_media_phy->media_phy_defaults[cnt].intrl_phy_supported_speed[indx++]
                                                = BASE_IF_SPEED_100GIGE;
                                        } else continue;
                                        break;
                                    default:
                                        continue;
                                }
                            }
                        }
                    } else if(strcmp(cnm, "external-phy") == 0) {
                        // todo for external phy configuration parsing
                        continue;
                    }
                }
            }
        }
    }
}


static struct pas_config_subcat {
    uint_t subcat;
    char   *name;
    enum {
        SUBCAT_INST_SCHEME_NONE,
        SUBCAT_INST_SCHEME_SLOT,
        SUBCAT_INST_SCHEME_ENTITY_TYPE_SLOT
    } inst_scheme;
    bool   valid;
} subcat_tbl[] = {
    { subcat:        BASE_PAS_CHASSIS_OBJ,
      name:          "chassis",
      inst_scheme:   SUBCAT_INST_SCHEME_NONE
    },
    { subcat:        BASE_PAS_ENTITY_OBJ,
      name:          "entity",
      inst_scheme:   SUBCAT_INST_SCHEME_NONE
    },
    { subcat:        BASE_PAS_PSU_OBJ,
      name:          "psu",
      inst_scheme:   SUBCAT_INST_SCHEME_NONE
    },
    { subcat:        BASE_PAS_FAN_TRAY_OBJ,
      name:          "fan-tray",
      inst_scheme:   SUBCAT_INST_SCHEME_NONE
    },
    { subcat:        BASE_PAS_CARD_OBJ,
      name:          "card",
      inst_scheme:   SUBCAT_INST_SCHEME_SLOT
    },
    { subcat:        BASE_PAS_FAN_OBJ,
      name:          "fan",
      inst_scheme:   SUBCAT_INST_SCHEME_NONE
    },
    { subcat:        BASE_PAS_LED_OBJ,
      name:          "led",
      inst_scheme:   SUBCAT_INST_SCHEME_ENTITY_TYPE_SLOT
    },
    { subcat:        BASE_PAS_DISPLAY_OBJ,
      name:          "display",
      inst_scheme:   SUBCAT_INST_SCHEME_ENTITY_TYPE_SLOT
    },
    { subcat:        BASE_PAS_TEMPERATURE_OBJ,
      name:          "temperature",
      inst_scheme:   SUBCAT_INST_SCHEME_ENTITY_TYPE_SLOT
    },
    { subcat:        BASE_PAS_TEMP_THRESHOLD_OBJ,
      name:          "temp-threshold",
      inst_scheme:   SUBCAT_INST_SCHEME_ENTITY_TYPE_SLOT
    },
    { subcat:        BASE_PAS_MEDIA_OBJ,
      name:          "media",
      inst_scheme:   SUBCAT_INST_SCHEME_SLOT
    },
    { subcat:        BASE_PAS_MEDIA_CHANNEL_OBJ,
      name:          "media-channel",
      inst_scheme:   SUBCAT_INST_SCHEME_SLOT
    },
    { subcat:        BASE_PAS_MEDIA_CONFIG_OBJ,
      name:          "media-config",
      inst_scheme:   SUBCAT_INST_SCHEME_SLOT
    }
};

/* Read configuration for a CPS object subcategory */

static void dn_pas_config_subcat(std_config_node_t nd)
{
    static const uint_t cps_api_qualifiers[] = {
        cps_api_qualifier_OBSERVED,
        cps_api_qualifier_REALTIME,
        cps_api_qualifier_TARGET
    };
    
    char                             *a;
    struct pas_config_subcat         *subcat;

    /****************************************
     * \todo Remove this -- Workaround, see below
     *
     * struct pas_config_entity         *e;
     * uint_t                           n, slot;
     *
     ****************************************/

    uint_t                           i;
    cps_api_registration_functions_t reg[1];

    a = std_config_attr_get(nd, "id");
    if (a == 0)  return;
    
    for (subcat = subcat_tbl; subcat < END(subcat_tbl); ++subcat) {
        if (strcmp(a, subcat->name) == 0)  break;
    }

    if (subcat >= END(subcat_tbl))  return;

    cps_api_key_init(&reg->key,
                     cps_api_qualifier_OBSERVED, /* Dummy, overwritted below */
                     cps_api_obj_CAT_BASE_PAS,
                     subcat->subcat,
                     0
                     );

    /****************************************
     * \todo Remove this -- Workaround
     *
     * In order to support chassis and cards in a generic fashion, we
     * need the ability to register for objects more specific than
     * just subcategory, but given the current CPS API instancing
     * scheme, this is not possible.  When CPS API has that
     * capability, this code needs to be re-activated.
     *
     *
     * switch (subcat->inst_scheme) {
     * case SUBCAT_INST_SCHEME_ENTITY_TYPE_SLOT:
     *     e = dn_pas_config_entity_find_attr(nd);
     *     if (e == 0)  return;
     *
     *     n = cps_api_key_get_len(&reg->key);
     *     cps_api_key_set_len(&reg->key, n + 1);
     *     cps_api_key_set(&reg->key, n, e->entity_type);
     *
     *     if (e->entity_type != PLATFORM_ENTITY_TYPE_CARD)  break;
     *
     *     / * Intentional fall-through * /
     *
     * case SUBCAT_INST_SCHEME_SLOT:
     *     n = cps_api_key_get_len(&reg->key);
     *     cps_api_key_set_len(&reg->key, n + 1);
     *     dn_pas_myslot_get(&slot);
     *     cps_api_key_set(&reg->key, n, slot);
     *
     *     break;
     *
     * default:
     *     ;
     * }
     *
     ****************************************/

    for (i = 0; i < ARRAY_SIZE(cps_api_qualifiers); ++i) {
        reg->handle             = cps_hdl;
        cps_api_key_set(&reg->key, CPS_OBJ_KEY_INST_POS, cps_api_qualifiers[i]);
        reg->_read_function     = dn_pas_read_function;
        reg->_write_function    = dn_pas_write_function;
        reg->_rollback_function = dn_pas_rollback_function;
        
        if (cps_api_register(reg) != cps_api_ret_code_OK)  return;
    }

    subcat->valid = true;
}


static config_entry_t element_tbl[] = {
    { "chassis",     dn_pas_config_chassis },
    { "entity",      dn_pas_config_entity },
    { "card",        dn_pas_config_card },
    { "fan",         dn_pas_config_fan },
    { "led-groups",  dn_pas_config_led_groups },
    { "temperature", dn_pas_config_temp },
    { "media",       dn_pas_config_media },
    { "phy-config",  dn_pas_media_read_phy_default_config }
};


/*
 * PAS CPS handler registration
 */

static config_entry_t cps_handler_reg[] = {{"subcat", dn_pas_config_subcat}};

void dn_pas_cps_handler_reg (const char *config_filename,
        cps_api_operation_handle_t _cps_hdl)
{

    dn_pas_config_parse(config_filename, _cps_hdl, cps_handler_reg,
            ARRAY_SIZE(cps_handler_reg));

}

/* Process config file */

void dn_pas_config_init(
    const char *config_filename, cps_api_operation_handle_t _cps_hdl)
{
    dn_pas_config_parse(config_filename, _cps_hdl, element_tbl,
            ARRAY_SIZE(element_tbl));

    if (!card_cfg->valid) {
        /* Raise a warning that card type is not defined */
    }
}

static void dn_pas_config_parse (
    const char *config_filename,
    cps_api_operation_handle_t _cps_hdl,
    config_entry_t *entry_tbl,
    uint_t size

                        )
{
    std_config_hdl_t  cfg_hdl;
    const char        *nm;
    std_config_node_t nd;
    uint_t            i;

    cps_hdl = _cps_hdl;

    cfg_hdl = std_config_load(config_filename);
    if (cfg_hdl == 0) {
        return;
    }

    nd = std_config_get_root(cfg_hdl);

    /* For each child of root, call handler function */

    for (nd = std_config_get_child(nd); nd != 0; nd = std_config_next_node(nd)) {
        nm = std_config_name_get(nd);
        
        for (i = 0; i < size; ++i) {
            if (strcmp(nm, entry_tbl[i].name) == 0) {
                (*entry_tbl[i].func)(nd);
                
                break;
            }
        }
    }

    std_config_unload(cfg_hdl);
}
