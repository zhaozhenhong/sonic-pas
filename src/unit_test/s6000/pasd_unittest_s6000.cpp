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
 * filename: pasd_unittest_s6000.cpp
 */ 

#include "std_type_defs.h"     
#include "std_event_service.h"
#include "cps_api_operation.h"
#include "cps_api_events.h"
#include "cps_api_event_init.h"
#include "cps_api_service.h"
#include "dn_pas.h"

#include "dell-base-common.h"   /** \todo Simplify */
#include "dell-base-platform-common.h"
#include "dell-base-pas.h"

#ifdef VM
#include "sdi_db.h"
#endif

#include <stdio.h>
#include <pthread.h>
#include <list>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "gtest/gtest.h"
#include <stdlib.h>
#include <time.h>
#include <string>

static const char *psu_entity_names[] = {
    "PSU Tray-1", "PSU Tray-2"
};

static const char *fan_tray_entity_names[] = {
    "Fan Tray-1", "Fan Tray-2", "Fan Tray-3"
};

static const char *card_entity_names[] = {
    "System Board"
};

static const char * const *entity_names[] = {
    psu_entity_names,
    fan_tray_entity_names,
    card_entity_names
};

static const uint8_t reboot_type[] = {
           PLATFORM_REBOOT_TYPE_WARM,  // reset all except NPU
           PLATFORM_REBOOT_TYPE_COLD   // reset all and CPU and NPU
};

static inline bool _try_false(uint_t line, const char *cond, bool f)
{
    f = !f;

    if (f) {
        printf("***FAILED*** Line %u: %s -> false\n", line, cond);
    }
  
    return (f);
}
#define TRY_FALSE(cond)  (_try_false(__LINE__, # cond, (cond)))

static inline bool _try_false_accum(
    uint_t     line,
    const char *cond,
    bool       f,
    bool       &accum
                                    )
{
    bool result;

    if ((result = _try_false(line, cond, f)))  accum = false;

    return (result);
}
#define TRY_FALSE_ACCUM(cond, accum)                    \
    (_try_false_accum(__LINE__, # cond, (cond), accum))

#define ARRAY_SIZE(a)  (sizeof(a) / sizeof((a)[0]))

#ifdef VM
static db_sql_handle_t db_hdl;
#endif

enum {
#ifdef VM
    POLL_DELAY = 5             // Time to wait for polling to catch up
#else
    POLL_DELAY = 2             // Time to wait for polling to catch up
#endif
};

static char *fuse_mount;

/***************************************************************************
 *
 * CPS API utilities
 */

static inline bool cps_api_object_attr_add_u8(
    cps_api_object_t  obj,
    cps_api_attr_id_t attr,
    uint_t            val
                                              )
{
    uint8_t temp = val;

    return (cps_api_object_attr_add(obj, attr, &temp, sizeof(temp)));
}

// Safe CPS API object delete

void cps_api_object_delete_safe(cps_api_object_t &obj)
{
    cps_api_object_delete(obj);
    obj = CPS_API_OBJECT_NULL;
}


bool obj_subcat_chk(cps_api_object_t obj, uint_t subcat)
{
    return (cps_api_key_get_subcat(cps_api_object_key(obj)) == subcat);
}

// Check that an attribute is present and has at least a certain length

bool attr_chk_len(cps_api_object_t obj, uint_t attr, uint_t min_len)
{
    cps_api_object_attr_t a;

    a = cps_api_object_attr_get(obj, attr);
    return (a != CPS_API_ATTR_NULL
            && cps_api_object_attr_len(a) >= min_len
            );
}

// Check the value of a bool attribute

bool attr_bool_chk(cps_api_object_t obj, uint_t attr, bool val)
{
    cps_api_object_attr_t a;

    a = cps_api_object_attr_get(obj, attr);
    return (a != CPS_API_ATTR_NULL
            && cps_api_object_attr_len(a) == 1
            && (* (uint8_t *) cps_api_object_attr_data_bin(a) != 0) == val
            );
}

// Check the value of a uint attribute

bool attr_uint_chk(cps_api_object_t obj, uint_t attr, uint_t val)
{
    cps_api_object_attr_t a;

    a = cps_api_object_attr_get(obj, attr);
    return (a != CPS_API_ATTR_NULL && cps_api_object_attr_data_uint(a) == val
            );
}

// Check the value of an int attribute

bool attr_int_chk(cps_api_object_t obj, uint_t attr, int val)
{
    cps_api_object_attr_t a;
    uint_t                v;

    a = cps_api_object_attr_get(obj, attr);
    if (a == CPS_API_ATTR_NULL)  return (false);

    v = cps_api_object_attr_data_uint(a);

    switch (cps_api_object_attr_len(a)) {
    case 1:
        if (v & (1 << 7))  v |= (~((uint_t) 0) & ~((1 << 8) - 1));
        break;
    case 2:
        if (v & (1 << 15))  v |= (~((uint_t) 0) & ~((1 << 16) - 1));
        break;
    case 4:
        ;
    default:
        return (false);
    }

    return ((int) v == val);
}

// Check the value of a uint64 attribute

bool attr_u64_chk(cps_api_object_t obj, uint_t attr, uint64_t val)
{
    cps_api_object_attr_t a;

    a = cps_api_object_attr_get(obj, attr);
    return (a != CPS_API_ATTR_NULL && cps_api_object_attr_data_u64(a) == val
            );
}

// Check the value of a string attribute

bool attr_str_chk(
    cps_api_object_t   obj,
    uint_t             attr,
    const char * const val
                  )
{
    cps_api_object_attr_t a;
    uint_t                alen;

    a = cps_api_object_attr_get(obj, attr);
    if (a == CPS_API_ATTR_NULL)  return (false);

    alen = cps_api_object_attr_len(a);
    return (alen == strlen(val)
            && memcmp(cps_api_object_attr_data_bin(a), val, alen) == 0
            );
}

// Check the value of a binary attribute

bool attr_bin_chk(
    cps_api_object_t      obj,
    uint_t                attr,
    uint_t                len,
    const uint8_t * const val
                  )
{
    cps_api_object_attr_t a;
    uint_t                alen;

    a = cps_api_object_attr_get(obj, attr);
    if (a == CPS_API_ATTR_NULL)  return (false);

    alen = cps_api_object_attr_len(a);
    return (alen == len
            && memcmp(cps_api_object_attr_data_bin(a), val, alen) == 0
            );
}

// Send a CPS API get request, expecting 1 object in response

bool cps_api_get1(cps_api_object_t &req_obj, cps_api_object_t &resp_obj)
{
    bool                 result = false;
    cps_api_get_params_t get_req[1];

    if (cps_api_get_request_init(get_req) == cps_api_ret_code_OK) {
        do {
            if (!cps_api_object_list_append(get_req->filters, req_obj))  break;

            req_obj = CPS_API_OBJECT_NULL; // No longer owned

            if (cps_api_get(get_req) != cps_api_ret_code_OK)  break;

            if (cps_api_object_list_size(get_req->list) != 1)  break;

            resp_obj = cps_api_object_list_get(get_req->list, 0);
            if (resp_obj == CPS_API_OBJECT_NULL)  break;

            cps_api_object_list_remove(get_req->list, 0);

            result = true;
        } while (0);
        
        cps_api_get_request_close(get_req);    
    }

    if (req_obj != CPS_API_OBJECT_NULL)  cps_api_object_delete(req_obj);

    return (result);
}

// Send a CPS API get request, count number of response objects

bool cps_api_get_cnt(cps_api_object_t &req_obj, uint_t &resp_cnt)
{
    bool                 result = false;
    cps_api_get_params_t get_req[1];

    if (cps_api_get_request_init(get_req) == cps_api_ret_code_OK) {
        do {
            if (!cps_api_object_list_append(get_req->filters, req_obj))  break;

            req_obj = CPS_API_OBJECT_NULL; // No longer owned

            if (cps_api_get(get_req) != cps_api_ret_code_OK)  break;

            resp_cnt = cps_api_object_list_size(get_req->list);

            result = true;
        } while (0);
        
        cps_api_get_request_close(get_req);    
    }

    if (req_obj != CPS_API_OBJECT_NULL)  cps_api_object_delete(req_obj);

    return (result);
}

// Send a CPS API set request

bool cps_api_set1(cps_api_object_t &obj)
{
    bool                         result = false;
    cps_api_transaction_params_t xact[1];
   
    if (cps_api_transaction_init(xact) == cps_api_ret_code_OK) {
        do {
            if (cps_api_set(xact, obj) != cps_api_ret_code_OK)  break;
            
            obj = CPS_API_OBJECT_NULL; // No longer owned
            
            if (cps_api_commit(xact) != cps_api_ret_code_OK)  break;

            result = true;
        } while (0);
         
        cps_api_transaction_close(xact);
    }
    
    if (obj != CPS_API_OBJECT_NULL)  cps_api_object_delete(obj);
    
    return (result);
}


#ifdef VM

bool sdi_entity_type_lkup(uint_t entity_type, sdi_entity_type_t *sdi_entity_type)
{
    switch (entity_type) {
    case PLATFORM_ENTITY_TYPE_PSU:
        *sdi_entity_type = SDI_ENTITY_PSU_TRAY;
        break;
    case PLATFORM_ENTITY_TYPE_FAN_TRAY:
        *sdi_entity_type = SDI_ENTITY_FAN_TRAY;
        break;
    case PLATFORM_ENTITY_TYPE_CARD:
        *sdi_entity_type = SDI_ENTITY_SYSTEM_BOARD;
        break;
    default:
        return (false);
    }

    return (true);
}

#endif

/***************************************************************************
 *
 * Event handling and checking
 */

static std::list<cps_api_object_t> cps_api_ev_list;
static pthread_mutex_t             cps_api_ev_list_mutex = PTHREAD_MUTEX_INITIALIZER;

// Event handler

bool cps_api_ev_hdlr(cps_api_object_t obj, void * const context)
{
    cps_api_object_t obj2;

    // Add to received event list

    obj2 = cps_api_object_create();
    if (obj2 != CPS_API_OBJECT_NULL) {
        if (cps_api_object_clone(obj2, obj)) {
            pthread_mutex_lock(&cps_api_ev_list_mutex);
      
            cps_api_ev_list.push_back(obj2);
      
            pthread_mutex_unlock(&cps_api_ev_list_mutex);
        } else {
            cps_api_object_delete_safe(obj2);
        }
    }

    if (obj2 == CPS_API_OBJECT_NULL) {
        printf("***ERROR*** Event lost\n");
    }
  
    return (true);
}

// Return number of queued events

uint_t cps_api_ev_cnt(void)
{
    return (cps_api_ev_list.size());
}

// Return next queued event 

cps_api_object_t cps_api_ev_next(void)
{
    cps_api_object_t result = CPS_API_OBJECT_NULL;

    pthread_mutex_lock(&cps_api_ev_list_mutex);

    if (!cps_api_ev_list.empty()) {
        result = cps_api_ev_list.front();
    
        cps_api_ev_list.pop_front();
    }

    pthread_mutex_unlock(&cps_api_ev_list_mutex);

    return (result);
}

// Flush queued events

uint_t cps_api_ev_flush(void)
{
    uint_t result = 0;

    pthread_mutex_lock(&cps_api_ev_list_mutex);

    for (; !cps_api_ev_list.empty(); ++result) {
        cps_api_object_t obj = cps_api_ev_list.front();

        cps_api_ev_list.pop_front();

        cps_api_object_delete_safe(obj);
    }

    pthread_mutex_unlock(&cps_api_ev_list_mutex);

    return (result);
}

/***************************************************************************
 *
 * Test utilities
 */

uint_t random_range(uint_t lower, uint_t upper)
{
    return (lower + (random() % (upper + 1 - lower)));
}

void random_permute(void *a, uint_t elem_size, uint_t num_elems)
{
    uint_t n, i, j;

    for (n = random_range(num_elems / 2, num_elems); n; --n) {
        uint8_t temp[elem_size], *p, *q;

        i = random_range(0, num_elems - 1);
        do {
            j = random_range(0, num_elems - 1);
        } while (j == i);

        p = (uint8_t *) a + i * elem_size;
        q = (uint8_t *) a + j * elem_size;

        memcpy(temp, p, elem_size);
        memcpy(p, q, elem_size);
        memcpy(q, temp, elem_size);
    }
}

void random_idxs(uint_t *a, uint_t size)
{
    uint_t i;

    for (i = 0; i < size; ++i)  a[i] = i;

    random_permute(a, sizeof(a[0]), size);
}

bool entity_present(cps_api_qualifier_t qual, uint_t entity_type, uint_t slot, bool &present)
{
    bool                  result = false;
    cps_api_object_t      req_obj, resp_obj;
    cps_api_object_attr_t a;

    req_obj = cps_api_object_create();
    if (req_obj == CPS_API_OBJECT_NULL)  return (false);

    dn_pas_obj_key_entity_set(req_obj, qual, true, entity_type, true, slot);

    if (!cps_api_get1(req_obj, resp_obj))  return (false);

    a = cps_api_object_attr_get(resp_obj, BASE_PAS_ENTITY_PRESENT);
    if (a != CPS_API_ATTR_NULL) {
        present = (cps_api_object_attr_data_uint(a) != 0);

        result = true;
    }
    
    cps_api_object_delete(resp_obj);

    return (result);
}


static const cps_api_qualifier_t cps_quals[] = {
    cps_api_qualifier_REALTIME,
    cps_api_qualifier_OBSERVED
};

bool try_foreach_qual(bool (*func)(cps_api_qualifier_t))
{
    bool   result = true;
    uint_t qual_idx;

    for (qual_idx = 0; qual_idx < ARRAY_SIZE(cps_quals); ++qual_idx) {
        TRY_FALSE_ACCUM((*func)(cps_quals[qual_idx]), result);
    }

    return (result);
}

bool try_foreach_qual_slot(
    bool   (*func)(cps_api_qualifier_t, uint_t),
    uint_t num_slots
                           )
{
    bool   result = true;
    uint_t qual_idx, slot;

    for (qual_idx = 0; qual_idx < ARRAY_SIZE(cps_quals); ++qual_idx) {
        for (slot = 1; slot <= num_slots; ++slot) {
            TRY_FALSE_ACCUM((*func)(cps_quals[qual_idx], slot), result);
        }
    }

    return (result);
}

bool try_foreach_qual_slot_idx(
    bool   (*func)(cps_api_qualifier_t, uint_t, uint_t),
    uint_t num_slots,
    uint_t n
                               )
{
    bool   result = true;
    uint_t qual_idx, slot, idx;

    for (qual_idx = 0; qual_idx < ARRAY_SIZE(cps_quals); ++qual_idx) {
        for (slot = 1; slot <= num_slots; ++slot) {
            for (idx = 1; idx <= n; ++idx) {
                TRY_FALSE_ACCUM((*func)(cps_quals[qual_idx], slot, idx), result);
            }
        }
    }

    return (result);
}

void test_init(void)
{
#ifdef VM
    sdi_db_reinit_database();   // Start with a known h/w config
#endif
    
    sleep(3);                   // Wait for any resulting events

    cps_api_ev_flush();         // Start with no events queued
}

bool test_fini(void)
{
    return (cps_api_ev_cnt() == 0); // Check no leftover events
}

/************************************************************************
 *
 * Test initial events
 */

TEST(pas_cps_test, init)
{
    uint_t              n, k;
    cps_api_object_t    obj;
    cps_api_key_t       *key;
    cps_api_qualifier_t qual;
    bool                entity_type_valid, slot_valid;
    uint_t              entity_type, slot;

    for (n = 0; ; ++n) {
        for (k = 0; k < 5; ++k) {
            obj = cps_api_ev_next();
            if (obj != CPS_API_OBJECT_NULL)  break;
            sleep(1);
        }

        if (obj == CPS_API_OBJECT_NULL)  break;
        
        key = cps_api_object_key(obj);

        EXPECT_TRUE(cps_api_key_element_at(key, CPS_OBJ_KEY_CAT_POS)
                    == cps_api_obj_CAT_BASE_PAS
                    );
        
        switch (cps_api_key_element_at(key, CPS_OBJ_KEY_SUBCAT_POS)) {
        case BASE_PAS_ENTITY_OBJ:
            dn_pas_obj_key_entity_get(obj,
                                      &qual,
                                      &entity_type_valid,
                                      &entity_type,
                                      &slot_valid,
                                      &slot
                                      );
            
            EXPECT_TRUE(qual == cps_api_qualifier_OBSERVED);
            EXPECT_TRUE(entity_type_valid && slot_valid);
            
            switch (entity_type) {
            case PLATFORM_ENTITY_TYPE_PSU:
                EXPECT_TRUE(slot >= 1 && slot <= 2);
                break;
                
            case PLATFORM_ENTITY_TYPE_FAN_TRAY:
                EXPECT_TRUE(slot >= 1 && slot <= 3);
                break;
                
            case PLATFORM_ENTITY_TYPE_CARD:
                EXPECT_TRUE(slot == 1);
                break;
                
            default:
                EXPECT_TRUE(false);
                continue;
            }
            
            EXPECT_TRUE(attr_bool_chk(obj, BASE_PAS_ENTITY_PRESENT, true));

            break;
            
        default:
            ;
        }

        cps_api_object_delete_safe(obj);
    }

    // Expect 1 event for each entity and 2 events for each for port

    EXPECT_TRUE(n == (6 + 2 * 32));
}

/***************************************************************************
 *
 * Test chassis subcategory
 */

bool chassis_attr_cps(cps_api_qualifier_t qual)
{
    bool                 passed = true;
    cps_api_key_t        key[1];
    cps_api_object_t     req_obj, resp_obj;
  
    cps_api_key_init(key,
                     qual,
                     cps_api_obj_CAT_BASE_PAS,
                     BASE_PAS_CHASSIS_OBJ,
                     0
                     );

    req_obj = cps_api_object_create();
    if (TRY_FALSE(req_obj != CPS_API_OBJECT_NULL)) {
        return (false);
    }
    cps_api_object_set_key(req_obj, key);

    if (TRY_FALSE(cps_api_get1(req_obj, resp_obj)))  return (false);
    
    do {
        if (TRY_FALSE_ACCUM(cps_api_key_matches(cps_api_object_key(resp_obj),
                                                key,
                                                true
                                                )
                            == 0,
                            passed
                            )
            ) {
            break;
        }

        // Check chassis object attributes
        
        TRY_FALSE_ACCUM(attr_uint_chk(resp_obj,
                                      BASE_PAS_CHASSIS_OPER_STATUS,
                                      BASE_CMN_OPER_STATUS_TYPE_UP
                                      ),
                        passed
                        );
        
        TRY_FALSE_ACCUM(attr_uint_chk(resp_obj,
                                      BASE_PAS_CHASSIS_FAULT_TYPE,
                                      PLATFORM_FAULT_TYPE_OK
                                      ),
                        passed
                        );
        
        // \todo
        // EEPROM values not currently known, so just check something returned
        // for each field -- Change to check for expected EEPROM values
        
        TRY_FALSE_ACCUM(attr_chk_len(resp_obj,
                                     BASE_PAS_CHASSIS_VENDOR_NAME,
                                     0
                                     ),
                        passed
                        );
        
        TRY_FALSE_ACCUM(attr_chk_len(resp_obj,
                                     BASE_PAS_CHASSIS_PRODUCT_NAME,
                                     0
                                     ),
                        passed
                        );
        
        TRY_FALSE_ACCUM(attr_chk_len(resp_obj,
                                     BASE_PAS_CHASSIS_HW_VERSION,
                                     0
                                     ),
                        passed
                        );
        
        TRY_FALSE_ACCUM(attr_chk_len(resp_obj,
                                     BASE_PAS_CHASSIS_PLATFORM_NAME,
                                     0
                                     ),
                        passed
                        );
        
        TRY_FALSE_ACCUM(attr_chk_len(resp_obj,
                                     BASE_PAS_CHASSIS_PPID,
                                     0
                                     ),
                        passed
                        );

        TRY_FALSE_ACCUM(attr_chk_len(resp_obj,
                                     BASE_PAS_CHASSIS_SERVICE_TAG,
                                     0
                                     ),
                        passed
                        );
        
        TRY_FALSE_ACCUM(attr_chk_len(resp_obj,
                                     BASE_PAS_CHASSIS_BASE_MAC_ADDRESSES,
                                     6
                                     ),
                        passed
                        );
        
        TRY_FALSE_ACCUM(attr_chk_len(resp_obj, 
                                     BASE_PAS_CHASSIS_NUM_MAC_ADDRESSES,
                                     1
                                     ),
                        passed
                        );
    } while (0);

    cps_api_object_delete(resp_obj);
  
    return (passed);
}

TEST(pas_cps_test, chassis_subcat)
{
    test_init();

    EXPECT_TRUE(try_foreach_qual(chassis_attr_cps));

    EXPECT_TRUE(test_fini());
};

/***************************************************************************
 *
 * Test entity counts
 */

bool entity_get_test(cps_api_qualifier_t qual)
{
    cps_api_object_t req_obj;
    uint_t           cnt;
    
    req_obj = cps_api_object_create();
    if (TRY_FALSE(req_obj != CPS_API_OBJECT_NULL)) {
        return (false);
    }

    dn_pas_obj_key_entity_set(req_obj, qual, false, 0, false, 0);

    if (TRY_FALSE(cps_api_get_cnt(req_obj, cnt)))  return (false);

    return (!TRY_FALSE(cnt == 6));
}

TEST(pas_cps_test, entity_subcat)
{
    test_init();

    EXPECT_TRUE(try_foreach_qual(entity_get_test));
  
    EXPECT_TRUE(test_fini());
}


bool num_entities_test(
    cps_api_qualifier_t qual, 
    uint_t              entity_type, 
    uint_t              num_entities
                       )
{
    cps_api_object_t req_obj;
    uint_t           cnt;
    
    req_obj = cps_api_object_create();
    if (TRY_FALSE(req_obj != CPS_API_OBJECT_NULL)) {
        return (false);
    }

    dn_pas_obj_key_entity_set(req_obj, qual, true, entity_type, false, 0);

    if (TRY_FALSE(cps_api_get_cnt(req_obj, cnt)))  return (false);

    return (!TRY_FALSE(cnt == num_entities));
}

bool num_psus_test(cps_api_qualifier_t qual)
{
    return (num_entities_test(qual, PLATFORM_ENTITY_TYPE_PSU, 2));
}

TEST(pas_cps_test, num_psus)
{
    test_init();

    EXPECT_TRUE(try_foreach_qual(num_psus_test));

    EXPECT_TRUE(test_fini());
}

bool num_fan_trays_test(cps_api_qualifier_t qual)
{
    return (num_entities_test(qual, PLATFORM_ENTITY_TYPE_FAN_TRAY, 3));
}

TEST(pas_cps_test, num_fan_trays)
{
    test_init();

    EXPECT_TRUE(try_foreach_qual(num_fan_trays_test));

    EXPECT_TRUE(test_fini());
}

bool num_cards_test(cps_api_qualifier_t qual)
{
    return (num_entities_test(qual, PLATFORM_ENTITY_TYPE_CARD, 1));
}

TEST(pas_cps_test, num_cards)
{
    test_init();

    EXPECT_TRUE(try_foreach_qual(num_cards_test));

    EXPECT_TRUE(test_fini());
}

/***************************************************************************
 *
 * Test entity attributes
 */

bool entity_attr_test(
    cps_api_qualifier_t qual,
    uint_t              entity_type,
    uint_t              slot,
    const char          * const entity_name,
    const char          * const vendor_name,
    const char          * const prod_name,
    const char          * const hw_version,
    const char          * const platform_name,
    const char          * const ppid,
    const char          * const service_tag
                      )
{
    bool                  passed = true;
    cps_api_object_t      req_obj, resp_obj;
    cps_api_qualifier_t   resp_qual;
    bool                  resp_entity_type_valid;
    uint_t                resp_entity_type;
    bool                  resp_slot_valid;
    uint_t                resp_slot;
    cps_api_object_attr_t a;
    
    req_obj = cps_api_object_create();
    if (TRY_FALSE(req_obj != CPS_API_OBJECT_NULL)) {
        return (false);
    }

    dn_pas_obj_key_entity_set(req_obj, qual, true, entity_type, true, slot);

    if (TRY_FALSE(cps_api_get1(req_obj, resp_obj)))  return (false);

    do {
        if (TRY_FALSE_ACCUM(obj_subcat_chk(resp_obj, BASE_PAS_ENTITY_OBJ),
                            passed
                            )
            ) {
            break;
        }

        dn_pas_obj_key_entity_get(resp_obj,
                                  &resp_qual,
                                  &resp_entity_type_valid,
                                  &resp_entity_type,
                                  &resp_slot_valid,
                                  &resp_slot
                                  );
    
        if (TRY_FALSE_ACCUM(resp_qual == qual
                            && resp_entity_type_valid 
                            && resp_entity_type == entity_type
                            && resp_slot_valid
                            && resp_slot == slot,
                            passed
                            )
            ) {
            break;
        }

        a = cps_api_object_attr_get(resp_obj, BASE_PAS_ENTITY_PRESENT);
        if (TRY_FALSE_ACCUM(a != CPS_API_ATTR_NULL
                            && cps_api_object_attr_len(a) == 1,
                            passed
                            )
            ) {
            break;
        }

        if (* (uint8_t *) cps_api_object_attr_data_bin(a) == 0) {
            fprintf(stderr,
                    "***WARNING*** Entity %s not installed, skipping test\n",
                    entity_name
                    );
            
            break;
        }
        
        TRY_FALSE_ACCUM(attr_str_chk(resp_obj, BASE_PAS_ENTITY_NAME, entity_name),
                        passed
                        );

        TRY_FALSE_ACCUM(attr_u64_chk(resp_obj,
                                     BASE_PAS_ENTITY_INSERTION_CNT,
                                     1
                                     ),
                        passed
                        );
        
        a = cps_api_object_attr_get(resp_obj,
                                    BASE_PAS_ENTITY_INSERTION_TIMESTAMP
                                    );
        TRY_FALSE_ACCUM(a != CPS_API_ATTR_NULL
                        && cps_api_object_attr_data_u64(a) != 0,
                        passed
                        );
        
        TRY_FALSE_ACCUM(attr_uint_chk(resp_obj,
                                      BASE_PAS_ENTITY_ADMIN_STATUS,
                                      BASE_CMN_ADMIN_STATUS_TYPE_UP
                                      ),
                        passed
                        );
        
        TRY_FALSE_ACCUM(attr_uint_chk(resp_obj,
                                      BASE_PAS_ENTITY_OPER_STATUS,
                                      BASE_CMN_OPER_STATUS_TYPE_UP
                                      ),
                        passed
                        );
        
        TRY_FALSE_ACCUM(attr_str_chk(resp_obj,
                                     BASE_PAS_ENTITY_VENDOR_NAME,
                                     vendor_name
                                     ),
                        passed
                        );
        
        TRY_FALSE_ACCUM(attr_str_chk(resp_obj,
                                     BASE_PAS_ENTITY_PRODUCT_NAME,
                                     prod_name
                                     ),
                        passed
                        );
        
        TRY_FALSE_ACCUM(attr_str_chk(resp_obj,
                                     BASE_PAS_ENTITY_HW_VERSION,
                                     hw_version
                                     ),
                        passed
                        );
        
        TRY_FALSE_ACCUM(attr_str_chk(resp_obj,
                                     BASE_PAS_ENTITY_PLATFORM_NAME,
                                     platform_name
                                     ),
                        passed
                        );
        
        TRY_FALSE_ACCUM(attr_str_chk(resp_obj,
                                     BASE_PAS_ENTITY_PPID,
                                     ppid
                                     ),
                        passed
                        );

        TRY_FALSE_ACCUM(attr_str_chk(resp_obj,
                                     BASE_PAS_ENTITY_SERVICE_TAG,
                                     service_tag 
                                     ),
                        passed
                        );
        
        if (entity_type == PLATFORM_ENTITY_TYPE_CARD) {
            TRY_FALSE_ACCUM(
                            attr_bool_chk(resp_obj,
                                          BASE_PAS_ENTITY_ENTITY_TYPE_CHOICE_CARD_POWER_ON,
                                          true
                                          ),
                            passed
                            );
        }
    } while (0);
        
    cps_api_object_delete(resp_obj);
  
    return (passed);
}

bool entity_psu_attr_test(cps_api_qualifier_t qual, uint_t slot)
{
    return (entity_attr_test(qual,
                             PLATFORM_ENTITY_TYPE_PSU,
                             slot,
                             psu_entity_names[slot - 1],
                             "",
                             "CN0T9FNW282983AR030",
                             "",
                             "",
                             "CN0T9FNW282983AR030",
                             ""
                             )
            );
}

bool entity_fan_tray_attr_test(cps_api_qualifier_t qual, uint_t slot)
{
    return (entity_attr_test(qual,
                             PLATFORM_ENTITY_TYPE_FAN_TRAY,
                             slot,
                             fan_tray_entity_names[slot - 1],
                             "",
                             "CN0MGDH8282983AR061",
                             "",
                             "",
                             "CN0MGDH8282983AR061",
                             ""
                             )
            );
}

bool entity_card_attr_test(cps_api_qualifier_t qual, uint_t slot)
{
    return (entity_attr_test(qual,
                             PLATFORM_ENTITY_TYPE_CARD,
                             slot,
                             card_entity_names[slot - 1],
                             "Dell",
                             "S6000",
                             "",
                             "",
                             "",
                             ""
                             )
            );
}

TEST(pas_cps_test, entity_attr)
{
    test_init();

    EXPECT_TRUE(try_foreach_qual_slot(entity_psu_attr_test, 2));
    EXPECT_TRUE(try_foreach_qual_slot(entity_fan_tray_attr_test, 3));
    EXPECT_TRUE(try_foreach_qual_slot(entity_card_attr_test, 1));

    EXPECT_TRUE(test_fini());
}

#ifdef VM

/***************************************************************************
 *
 * Test entity presence
 */

bool entity_presence_test(
    cps_api_qualifier_t qual,
    uint_t              entity_type,
    uint_t              slot
                          )
{
    bool                 passed = true;
    cps_api_object_t     req_obj = CPS_API_OBJECT_NULL;
    cps_api_object_t     ev_obj = CPS_API_OBJECT_NULL;
    cps_api_object_t     resp_obj = CPS_API_OBJECT_NULL;
    sdi_entity_type_t    sdi_entity_type;
    sdi_entity_hdl_t     entity_hdl;
    uint_t               k;
    bool                 present;
    cps_api_qualifier_t  resp_qual;
    bool                 resp_entity_type_valid, resp_slot_valid;
    uint_t               resp_entity_type, resp_slot;

    if (!sdi_entity_type_lkup(entity_type, &sdi_entity_type))  return (false);

    for (k = 0; k < 2; ++k) {
        present = (k == 1);

        // Set entity presence in h/w simulation
    
        if (TRY_FALSE_ACCUM(sdi_db_get_entity_handle(db_hdl,
                                                     sdi_entity_type,
                                                     slot,
                                                     &entity_hdl
                                                     )
                            == STD_ERR_OK,
                            passed
                            )
            ) {
            continue;
        }

        if (TRY_FALSE_ACCUM(sdi_db_entity_presence_set(db_hdl, entity_hdl, present)
                            == STD_ERR_OK,
                            passed
                            )
            ) {
            continue;
        }
    
        // Do CPS get for entity
    
        sleep(qual == cps_api_qualifier_REALTIME ? 0 : POLL_DELAY);

        req_obj = cps_api_object_create();
        if (TRY_FALSE_ACCUM(req_obj != CPS_API_OBJECT_NULL, passed)) {
            continue;
        }

        dn_pas_obj_key_entity_set(req_obj, qual, true, entity_type, true, slot);

        if (TRY_FALSE_ACCUM(cps_api_get1(req_obj, resp_obj), passed)) {
            continue;
        }

        do {
            if (TRY_FALSE_ACCUM(obj_subcat_chk(resp_obj, BASE_PAS_ENTITY_OBJ),
                                passed
                                )
                ) {
                break;
            }

            dn_pas_obj_key_entity_get(resp_obj,
                                      &resp_qual,
                                      &resp_entity_type_valid,
                                      &resp_entity_type,
                                      &resp_slot_valid,
                                      &resp_slot
                                      );
            
            if (TRY_FALSE_ACCUM(resp_qual == qual
                                && resp_entity_type_valid
                                && resp_entity_type == entity_type
                                && resp_slot_valid
                                && resp_slot == slot,
                                passed
                                )
                ) {
                break;
            }

            // Verify presence
            
            TRY_FALSE_ACCUM(attr_bool_chk(resp_obj,
                                          BASE_PAS_ENTITY_PRESENT,
                                          present ? 1 : 0
                                          ),
                            passed
                            );
        } while (0);
            
        cps_api_object_delete(resp_obj);
            
        // Check entity event recived
    
        sleep(qual == cps_api_qualifier_OBSERVED ? 0 : POLL_DELAY);
    
        ev_obj = cps_api_ev_next();
        if (!TRY_FALSE_ACCUM(ev_obj != CPS_API_OBJECT_NULL, passed)) {
            cps_api_qualifier_t ev_qual;
            bool                ev_entity_type_valid, ev_slot_valid;
            uint_t              ev_entity_type, ev_slot;

            // Event received

            // Verify event

            do {
                if (TRY_FALSE_ACCUM(obj_subcat_chk(ev_obj, BASE_PAS_ENTITY_OBJ),
                                    passed
                                    )
                    ) {
                    break;
                }

                dn_pas_obj_key_entity_get(ev_obj,
                                          &ev_qual,
                                          &ev_entity_type_valid,
                                          &ev_entity_type,
                                          &ev_slot_valid,
                                          &ev_slot
                                          );
                
                if (TRY_FALSE_ACCUM(ev_qual == cps_api_qualifier_OBSERVED
                                    && ev_entity_type_valid
                                    && ev_entity_type == entity_type
                                    && ev_slot_valid
                                    && ev_slot == slot,
                                    passed
                                    )
                    ) {
                    break;
                }

                TRY_FALSE_ACCUM(attr_bool_chk(ev_obj,
                                              BASE_PAS_ENTITY_PRESENT,
                                              present ? 1 : 0
                                              ),
                                passed
                                );
            } while (0);
            
            cps_api_object_delete_safe(ev_obj);
        }
    }

    return (passed);
}

bool psu_presence_test(cps_api_qualifier_t qual, uint_t slot)
{
    return (entity_presence_test(qual, PLATFORM_ENTITY_TYPE_PSU, slot));
}

bool fan_tray_presence_test(cps_api_qualifier_t qual, uint_t slot)
{
    return (entity_presence_test(qual, PLATFORM_ENTITY_TYPE_FAN_TRAY, slot));
}

bool card_presence_test(cps_api_qualifier_t qual)
{
    bool                 passed = true;
    cps_api_object_t     req_obj, resp_obj;
    cps_api_qualifier_t  resp_qual;
    bool                 resp_entity_type_valid, resp_slot_valid;
    uint_t               resp_entity_type, resp_slot;
    
    req_obj = cps_api_object_create();
    if (TRY_FALSE(req_obj != CPS_API_OBJECT_NULL)) {
        return (false);
    }

    dn_pas_obj_key_entity_set(req_obj,
                              qual,
                              true, PLATFORM_ENTITY_TYPE_CARD,
                              true, 1
                              );

    if (TRY_FALSE(cps_api_get1(req_obj, resp_obj)))  return (false);

    do {
        if (TRY_FALSE_ACCUM(obj_subcat_chk(resp_obj, BASE_PAS_ENTITY_OBJ),
                            passed
                            )
            ) {
            break;
        }

        dn_pas_obj_key_entity_get(resp_obj,
                                  &resp_qual,
                                  &resp_entity_type_valid,
                                  &resp_entity_type,
                                  &resp_slot_valid,
                                  &resp_slot
                                  );
        
        if (TRY_FALSE_ACCUM(resp_qual == qual
                            && resp_entity_type_valid
                            && resp_entity_type == PLATFORM_ENTITY_TYPE_CARD
                            && resp_slot_valid
                            && resp_slot == 1,
                            passed
                            )
            ) {
            break;
        }

        // Verify presence
        
        TRY_FALSE_ACCUM(attr_bool_chk(resp_obj, BASE_PAS_ENTITY_PRESENT, 1),
                        passed
                        );
    } while (0);
        
    cps_api_object_delete(resp_obj);
  
    return (passed);
}

TEST(pas_cps_test, entity_presence)
{
    test_init();

    EXPECT_TRUE(try_foreach_qual_slot(psu_presence_test, 2));
    EXPECT_TRUE(try_foreach_qual_slot(fan_tray_presence_test, 3));
    EXPECT_TRUE(try_foreach_qual(card_presence_test));

    EXPECT_TRUE(test_fini());
}

#endif /* defined(VM) */

#ifdef VM

/***************************************************************************
 *
 * Test entity faults
 */

bool entity_fault_test(
    cps_api_qualifier_t qual,
    uint_t              entity_type,
    uint_t              slot
                          )
{
    bool                 passed = true;
    cps_api_object_t     req_obj = CPS_API_OBJECT_NULL;
    cps_api_object_t     ev_obj = CPS_API_OBJECT_NULL;
    cps_api_object_t     resp_obj = CPS_API_OBJECT_NULL;
    sdi_entity_type_t    sdi_entity_type;
    sdi_entity_hdl_t     entity_hdl;
    uint_t               k;
    bool                 fault;
    cps_api_qualifier_t  resp_qual;
    bool                 resp_entity_type_valid, resp_slot_valid;
    uint_t               resp_entity_type, resp_slot;

    if (!sdi_entity_type_lkup(entity_type, &sdi_entity_type))  return (false);

    for (k = 0; k < 2; ++k) {
        fault = (k == 0);

        // Set entity fault in h/w simulation
    
        if (TRY_FALSE_ACCUM(sdi_db_get_entity_handle(db_hdl,
                                                     sdi_entity_type,
                                                     slot,
                                                     &entity_hdl
                                                     )
                            == STD_ERR_OK,
                            passed
                            )
            ) {
            continue;
        }

        if (TRY_FALSE_ACCUM(sdi_db_entity_fault_status_set(db_hdl, entity_hdl, fault)
                            == STD_ERR_OK,
                            passed
                            )
            ) {
            continue;
        }
    
        // Do CPS get for entity
    
        sleep(qual == cps_api_qualifier_REALTIME ? 0 : POLL_DELAY);

        req_obj = cps_api_object_create();
        if (TRY_FALSE_ACCUM(req_obj != CPS_API_OBJECT_NULL, passed)) {
            continue;
        }

        dn_pas_obj_key_entity_set(req_obj, qual, true, entity_type, true, slot);

        if (TRY_FALSE_ACCUM(cps_api_get1(req_obj, resp_obj), passed)) {
            continue;
        }
        
        do {
            if (TRY_FALSE_ACCUM(obj_subcat_chk(resp_obj, BASE_PAS_ENTITY_OBJ),
                                passed
                                )
                ) {
                break;
            }

            dn_pas_obj_key_entity_get(resp_obj,
                                      &resp_qual,
                                      &resp_entity_type_valid,
                                      &resp_entity_type,
                                      &resp_slot_valid,
                                      &resp_slot
                                      );
            
            if (TRY_FALSE_ACCUM(resp_qual == qual
                                && resp_entity_type_valid
                                && resp_entity_type == entity_type
                                && resp_slot_valid
                                && resp_slot == slot,
                                passed
                                )
                ) {
                break;
            }

            // Verify fault
            
            TRY_FALSE_ACCUM(attr_uint_chk(resp_obj,
                                          BASE_PAS_ENTITY_OPER_STATUS,
                                          fault
                                          ? BASE_CMN_OPER_STATUS_TYPE_FAIL
                                          : BASE_CMN_OPER_STATUS_TYPE_UP
                                          ),
                            passed
                            );

            TRY_FALSE_ACCUM(attr_uint_chk(resp_obj,
                                          BASE_PAS_ENTITY_FAULT_TYPE,
                                          fault
                                          ? PLATFORM_FAULT_TYPE_EHW
                                          : PLATFORM_FAULT_TYPE_OK
                                          ),
                            passed
                            );
        } while (0);
            
        cps_api_object_delete(resp_obj);
            
        // Check entity event recived
    
        sleep(qual == cps_api_qualifier_OBSERVED ? 0 : POLL_DELAY);
    
        ev_obj = cps_api_ev_next();
        if (!TRY_FALSE_ACCUM(ev_obj != CPS_API_OBJECT_NULL, passed)) {
            cps_api_qualifier_t ev_qual;
            bool                ev_entity_type_valid, ev_slot_valid;
            uint_t              ev_entity_type, ev_slot;

            // Event received

            // Verify event

            do {
                if (TRY_FALSE_ACCUM(obj_subcat_chk(ev_obj, BASE_PAS_ENTITY_OBJ),
                                    passed
                                    )
                    ) {
                    break;
                }

                dn_pas_obj_key_entity_get(ev_obj,
                                          &ev_qual,
                                          &ev_entity_type_valid,
                                          &ev_entity_type,
                                          &ev_slot_valid,
                                          &ev_slot
                                          );
                
                if (TRY_FALSE_ACCUM(ev_qual == cps_api_qualifier_OBSERVED
                                    && ev_entity_type_valid
                                    && ev_entity_type == entity_type
                                    && ev_slot_valid
                                    && ev_slot == slot,
                                    passed
                                    )
                    ) {
                    break;
                }
                
                TRY_FALSE_ACCUM(attr_uint_chk(ev_obj,
                                              BASE_PAS_ENTITY_OPER_STATUS,
                                              fault
                                              ? BASE_CMN_OPER_STATUS_TYPE_FAIL
                                              : BASE_CMN_OPER_STATUS_TYPE_UP
                                              ),
                                passed
                                );
                
                TRY_FALSE_ACCUM(attr_uint_chk(ev_obj,
                                              BASE_PAS_ENTITY_FAULT_TYPE,
                                              fault
                                              ? PLATFORM_FAULT_TYPE_EHW
                                              : PLATFORM_FAULT_TYPE_OK
                                              ),
                                passed
                                );
            } while (0);
            
            cps_api_object_delete_safe(ev_obj);
        }
    }

    return (passed);
}

bool psu_fault_test(cps_api_qualifier_t qual, uint_t slot)
{
    return (entity_fault_test(qual, PLATFORM_ENTITY_TYPE_PSU, slot));
}

bool fan_tray_fault_test(cps_api_qualifier_t qual, uint_t slot)
{
    return (entity_fault_test(qual, PLATFORM_ENTITY_TYPE_FAN_TRAY, slot));
}

TEST(pas_cps_test, entity_fault)
{
    test_init();

    EXPECT_TRUE(try_foreach_qual_slot(psu_fault_test, 2));
    EXPECT_TRUE(try_foreach_qual_slot(fan_tray_fault_test, 3));

    EXPECT_TRUE(test_fini());
}

#endif /* defined(VM) */

/***************************************************************************
 *
 * Test PSU attributes
 */

bool psu_attr_test(cps_api_qualifier_t qual, uint_t slot)
{
    bool                 passed = true, present;
    cps_api_object_t     req_obj, resp_obj;
    cps_api_qualifier_t  resp_qual;
    bool                 resp_slot_valid;
    uint_t               resp_slot;
    
    if (TRY_FALSE(entity_present(qual, PLATFORM_ENTITY_TYPE_PSU, slot, present))) {
        return (false);
    }
    if (!present) {
        fprintf(stderr,
                "***WARNING*** Entity %s not installed, skipping test\n",
                psu_entity_names[slot - 1]
                );
        
        return (true);
    }

    req_obj = cps_api_object_create();
    if (TRY_FALSE(req_obj != CPS_API_OBJECT_NULL)) {
        return (false);
    }

    dn_pas_obj_key_psu_set(req_obj, qual, true, slot);

    if (TRY_FALSE(cps_api_get1(req_obj, resp_obj)))  return (false);

    do {
        if (TRY_FALSE_ACCUM(obj_subcat_chk(resp_obj, BASE_PAS_PSU_OBJ),
                            passed
                            )
            ) {
            break;
        }

        dn_pas_obj_key_psu_get(resp_obj,
                               &resp_qual,
                               &resp_slot_valid,
                               &resp_slot
                               );

        if (TRY_FALSE_ACCUM(resp_qual == qual
                            && resp_slot_valid
                            && resp_slot == slot,
                            passed
                            )
            ) {
            break;
        }

        TRY_FALSE_ACCUM(attr_uint_chk(resp_obj,
                                      BASE_PAS_PSU_INPUT_TYPE,
                                      PLATFORM_INPUT_POWER_TYPE_AC
                                      ),
                        passed
                        );

        TRY_FALSE_ACCUM(attr_uint_chk(resp_obj,
                                      BASE_PAS_PSU_FAN_AIRFLOW_TYPE,
                                      PLATFORM_FAN_AIRFLOW_TYPE_UNKNOWN
                                      ),
                        passed
                        );
    } while (0);
        
    cps_api_object_delete(resp_obj);
  
    return (passed);
}

TEST(pas_cps_test, psu_attr)
{
    test_init();
  
    EXPECT_TRUE(try_foreach_qual_slot(psu_attr_test, 2));
  
    EXPECT_TRUE(test_fini());
}

/***************************************************************************
 *
 * Test fan tray attributes
 */

bool fan_tray_attr_test(cps_api_qualifier_t qual, uint_t slot)
{
    bool                 passed = true, present;
    cps_api_object_t     req_obj, resp_obj;
    cps_api_qualifier_t  resp_qual;
    bool                 resp_slot_valid;
    uint_t               resp_slot;
    
    if (TRY_FALSE(entity_present(qual, PLATFORM_ENTITY_TYPE_FAN_TRAY, slot, present))) {
        return (false);
    }
    if (!present) {
        fprintf(stderr,
                "***WARNING*** Entity %s not installed, skipping test\n",
                fan_tray_entity_names[slot - 1]
                );
        
        return (true);
    }

    req_obj = cps_api_object_create();
    if (TRY_FALSE(req_obj != CPS_API_OBJECT_NULL)) {
        return (false);
    }

    dn_pas_obj_key_fan_tray_set(req_obj, qual, true, slot);

    if (TRY_FALSE(cps_api_get1(req_obj, resp_obj)))  return (false);

    do {
        if (TRY_FALSE_ACCUM(obj_subcat_chk(resp_obj, BASE_PAS_FAN_TRAY_OBJ),
                            passed
                            )
            ) {
            break;
        }

        dn_pas_obj_key_fan_tray_get(resp_obj,
                                    &resp_qual,
                                    &resp_slot_valid,
                                    &resp_slot
                                    );

        if (TRY_FALSE_ACCUM(resp_qual == qual
                            && resp_slot_valid
                            && resp_slot == slot,
                            passed
                            )
            ) {
            break;
        }
        
        TRY_FALSE_ACCUM(attr_uint_chk(resp_obj,
                                      BASE_PAS_FAN_TRAY_FAN_AIRFLOW_TYPE,
                                      PLATFORM_FAN_AIRFLOW_TYPE_REVERSE
                                      ),
                        passed
                        );
    } while (0);
    
    cps_api_object_delete(resp_obj);
  
    return (passed);
}

TEST(pas_cps_test, fan_tray_attr)
{
    test_init();
  
    EXPECT_TRUE(try_foreach_qual_slot(fan_tray_attr_test, 3));
  
    EXPECT_TRUE(test_fini());
}

/***************************************************************************
 *
 * Test card attributes
 */

bool card_attr_test(cps_api_qualifier_t qual)
{
    bool                 passed = true;
    cps_api_object_t     req_obj, resp_obj;
    cps_api_qualifier_t  resp_qual;
    bool                 resp_slot_valid;
    uint_t               resp_slot;
    
    req_obj = cps_api_object_create();
    if (TRY_FALSE(req_obj != CPS_API_OBJECT_NULL)) {
        return (false);
    }

    dn_pas_obj_key_card_set(req_obj, qual, true, 1);

    if (TRY_FALSE(cps_api_get1(req_obj, resp_obj)))  return (false);

    do {
        if (TRY_FALSE_ACCUM(obj_subcat_chk(resp_obj, BASE_PAS_CARD_OBJ),
                            passed
                            )
            ) {
            break;
        }

        dn_pas_obj_key_card_get(resp_obj,
                                &resp_qual,
                                &resp_slot_valid,
                                &resp_slot
                                );
        
        if (TRY_FALSE_ACCUM(resp_qual == qual
                            && resp_slot_valid
                            && resp_slot == 1,
                            passed
                            )
            ) {
            break;
        }

        TRY_FALSE_ACCUM(attr_uint_chk(resp_obj, BASE_PAS_CARD_CARD_TYPE, 0xfe00),
                        passed
                        );
    } while (0);
        
    cps_api_object_delete(resp_obj);
  
    return (passed);
}

TEST(pas_cps_test, card_attr)
{
    test_init();

    EXPECT_TRUE(try_foreach_qual(card_attr_test));

    EXPECT_TRUE(test_fini());
}

/***************************************************************************
 *
 * Test fan attributes
 */

bool fan_attr_test(
    cps_api_qualifier_t qual,
    uint_t              entity_type,
    uint_t              slot,
    uint_t              fan_idx,
    uint_t              max_speed,
    uint_t              speed_margin
                   )
{
    bool                  passed = true, present;
    cps_api_object_t      req_obj, resp_obj;
    cps_api_qualifier_t   resp_qual;
    bool                  resp_entity_type_valid;
    uint_t                resp_entity_type;
    bool                  resp_slot_valid;
    uint_t                resp_slot;
    bool                  resp_fan_idx_valid;
    uint_t                resp_fan_idx;
    cps_api_object_attr_t a;
    uint_t                fan_speed;
    
    if (TRY_FALSE(entity_present(qual, entity_type, slot, present))) {
        return (false);
    }
    if (!present) {
        fprintf(stderr,
                "***WARNING*** Entity %s not installed, skipping test\n",
                entity_names[entity_type - 1][slot - 1]
                );
        
        return (true);
    }

    req_obj = cps_api_object_create();
    if (TRY_FALSE(req_obj != CPS_API_OBJECT_NULL)) {
        return (false);
    }

    dn_pas_obj_key_fan_set(req_obj,
                           qual,
                           true, entity_type,
                           true, slot,
                           true, fan_idx
                           );

    if (TRY_FALSE(cps_api_get1(req_obj, resp_obj)))  return (false);

    do {
        if (TRY_FALSE_ACCUM(obj_subcat_chk(resp_obj, BASE_PAS_FAN_OBJ),
                            passed
                            )
            ) {
            break;
        }

        dn_pas_obj_key_fan_get(resp_obj,
                               &resp_qual,
                               &resp_entity_type_valid,
                               &resp_entity_type,
                               &resp_slot_valid,
                               &resp_slot,
                               &resp_fan_idx_valid,
                               &resp_fan_idx
                               );

        if (TRY_FALSE_ACCUM(resp_qual == qual
                            && resp_entity_type_valid
                            && resp_entity_type == entity_type
                            && resp_slot_valid
                            && resp_slot == slot
                            && resp_fan_idx_valid
                            && resp_fan_idx == fan_idx,
                            passed
                            )
            ) {
            break;
        }

        TRY_FALSE_ACCUM(attr_uint_chk(resp_obj,
                                      BASE_PAS_FAN_OPER_STATUS,
                                      BASE_CMN_OPER_STATUS_TYPE_UP
                                      ),
                        passed
                        );
        
        TRY_FALSE_ACCUM(attr_uint_chk(resp_obj,
                                      BASE_PAS_FAN_FAULT_TYPE,
                                      PLATFORM_FAULT_TYPE_OK
                                      ),
                        passed
                        );
    
        TRY_FALSE_ACCUM(attr_uint_chk(resp_obj,
                                      BASE_PAS_FAN_MAX_SPEED, 
                                      max_speed
                                      ),
                        passed
                        );

        a = cps_api_object_attr_get(resp_obj, BASE_PAS_FAN_SPEED);
        if (TRY_FALSE_ACCUM(a != CPS_API_ATTR_NULL, passed))  break;

        fan_speed = cps_api_object_attr_data_uint(a);
        
        TRY_FALSE_ACCUM(((100 * abs((int) max_speed - (int) fan_speed)) / max_speed) <= speed_margin,
                        passed
                        );
    } while (0);
        
    cps_api_object_delete(resp_obj);
  
    return (passed);
}

bool psu_fan_attr_test(cps_api_qualifier_t qual, uint_t slot, uint_t idx)
{
    return (fan_attr_test(qual,
                          PLATFORM_ENTITY_TYPE_PSU,
                          slot,
                          idx,
                          19000,
                          10
                          )
            );
}

bool fan_tray_fan_attr_test(
    cps_api_qualifier_t qual,
    uint_t              slot,
    uint_t              idx
                            )
{
    return (fan_attr_test(qual,
                          PLATFORM_ENTITY_TYPE_FAN_TRAY,
                          slot,
                          idx,
                          18000,
                          10
                          )
            );
}

TEST(pas_cps_test, fan_attr)
{
    test_init();

    EXPECT_TRUE(try_foreach_qual_slot_idx(psu_fan_attr_test, 2, 1));
    EXPECT_TRUE(try_foreach_qual_slot_idx(fan_tray_fan_attr_test, 3, 2));

    EXPECT_TRUE(test_fini());
}

#ifdef VM

/***************************************************************************
 *
 * Test fan faults
 */

bool fan_fault_test(
    cps_api_qualifier_t qual,
    uint_t              entity_type,
    uint_t              slot,
    uint_t              fan_idx,
    const char *        sdi_alias
                          )
{
    bool                 passed = true;
    cps_api_object_t     req_obj = CPS_API_OBJECT_NULL;
    cps_api_object_t     ev_obj = CPS_API_OBJECT_NULL;
    cps_api_object_t     resp_obj = CPS_API_OBJECT_NULL;
    sdi_entity_type_t    sdi_entity_type;
    sdi_entity_hdl_t     entity_hdl;
    sdi_resource_hdl_t   fan_resource_hdl;
    uint_t               k;
    int                  fault;
    cps_api_qualifier_t  resp_qual;
    bool                 resp_entity_type_valid, resp_slot_valid, resp_fan_idx_valid;
    uint_t               resp_entity_type, resp_slot, resp_fan_idx;

    if (!sdi_entity_type_lkup(entity_type, &sdi_entity_type))  return (false);

    for (k = 0; k < 2; ++k) {
        fault = (k == 0);

        // Set entity fault in h/w simulation
    
        if (TRY_FALSE_ACCUM(sdi_db_get_entity_handle(db_hdl,
                                                     sdi_entity_type,
                                                     slot,
                                                     &entity_hdl
                                                     )
                            == STD_ERR_OK,
                            passed
                            )
            ) {
            continue;
        }

        if (TRY_FALSE_ACCUM(sdi_db_get_resource_handle_by_alias(db_hdl,
                                                                entity_hdl,
                                                                SDI_RESOURCE_FAN,
                                                                sdi_alias,
                                                                &fan_resource_hdl
                                                                )
                            == STD_ERR_OK,
                            passed
                            )
            ) {
            continue;
        }

        if (TRY_FALSE_ACCUM(sdi_db_int_field_set(db_hdl,
                                                 fan_resource_hdl,
                                                 TABLE_FAN,
                                                 FAN_FAULT,
                                                 &fault
                                                 )
                            == STD_ERR_OK,
                            passed
                            )
            ) {
            continue;
        }
    
        // Do CPS get for entity
    
        sleep(qual == cps_api_qualifier_REALTIME ? 0 : POLL_DELAY);

        req_obj = cps_api_object_create();
        if (TRY_FALSE_ACCUM(req_obj != CPS_API_OBJECT_NULL, passed)) {
            continue;
        }

        dn_pas_obj_key_fan_set(req_obj, qual, true, entity_type, true, slot, true, fan_idx);

        if (TRY_FALSE_ACCUM(cps_api_get1(req_obj, resp_obj), passed))  continue;
        
        do {
            if (TRY_FALSE_ACCUM(obj_subcat_chk(resp_obj, BASE_PAS_FAN_OBJ),
                                passed
                                )
                ) {
                break;
            }

            dn_pas_obj_key_fan_get(resp_obj,
                                   &resp_qual,
                                   &resp_entity_type_valid,
                                   &resp_entity_type,
                                   &resp_slot_valid,
                                   &resp_slot,
                                   &resp_fan_idx_valid,
                                   &resp_fan_idx
                                   );
            
            if (TRY_FALSE_ACCUM(resp_qual == qual
                                && resp_entity_type_valid
                                && resp_entity_type == entity_type
                                && resp_slot_valid
                                && resp_slot == slot
                                && resp_fan_idx_valid
                                && resp_fan_idx == fan_idx,
                                passed
                                )
                ) {
                break;
            }

            // Verify fault
            
            TRY_FALSE_ACCUM(attr_uint_chk(resp_obj,
                                          BASE_PAS_FAN_OPER_STATUS,
                                          fault
                                          ? BASE_CMN_OPER_STATUS_TYPE_FAIL
                                          : BASE_CMN_OPER_STATUS_TYPE_UP
                                          ),
                            passed
                            );

            TRY_FALSE_ACCUM(attr_uint_chk(resp_obj,
                                          BASE_PAS_FAN_FAULT_TYPE,
                                          fault
                                          ? PLATFORM_FAULT_TYPE_EHW
                                          : PLATFORM_FAULT_TYPE_OK
                                          ),
                            passed
                            );
        } while (0);
        
        cps_api_object_delete(resp_obj);
        
        // Check fan event recived
        
        sleep(qual == cps_api_qualifier_OBSERVED ? 0 : POLL_DELAY);
    
        ev_obj = cps_api_ev_next();
        if (!TRY_FALSE_ACCUM(ev_obj != CPS_API_OBJECT_NULL, passed)) {
            cps_api_qualifier_t ev_qual;
            bool                ev_entity_type_valid, ev_slot_valid, ev_fan_idx_valid;
            uint_t              ev_entity_type, ev_slot, ev_fan_idx;

            // Event received

            // Verify event

            do {
                if (TRY_FALSE_ACCUM(obj_subcat_chk(ev_obj, BASE_PAS_FAN_OBJ),
                                    passed
                                    )
                    ) {
                    break;
                }

                dn_pas_obj_key_fan_get(ev_obj,
                                       &ev_qual,
                                       &ev_entity_type_valid,
                                       &ev_entity_type,
                                       &ev_slot_valid,
                                       &ev_slot,
                                       &ev_fan_idx_valid,
                                       &ev_fan_idx
                                       );

                if (TRY_FALSE_ACCUM(ev_qual == cps_api_qualifier_OBSERVED
                                    && ev_entity_type_valid
                                    && ev_entity_type == entity_type
                                    && ev_slot_valid
                                    && ev_slot == slot
                                    && ev_fan_idx_valid
                                    && ev_fan_idx == fan_idx,
                                    passed
                                    )
                    ) {
                    break;
                }
                
                TRY_FALSE_ACCUM(attr_uint_chk(ev_obj,
                                              BASE_PAS_FAN_OPER_STATUS,
                                              fault
                                              ? BASE_CMN_OPER_STATUS_TYPE_FAIL
                                              : BASE_CMN_OPER_STATUS_TYPE_UP
                                              ),
                                passed
                                );
                
                TRY_FALSE_ACCUM(attr_uint_chk(ev_obj,
                                              BASE_PAS_FAN_FAULT_TYPE,
                                              fault
                                              ? PLATFORM_FAULT_TYPE_EHW
                                              : PLATFORM_FAULT_TYPE_OK
                                              ),
                                passed
                                );
            } while (0);

            cps_api_object_delete_safe(ev_obj);
        }
    }

    return (passed);
}

bool psu_fan_fault_test(cps_api_qualifier_t qual, uint_t slot)
{
    static const char *sdi_aliases[] = {
        "PSU 1 Fan", "PSU 2 Fan"
    };

    return (fan_fault_test(qual,
                           PLATFORM_ENTITY_TYPE_PSU,
                           slot,
                           1,
                           sdi_aliases[slot - 1]
                           )
            );
}

bool fan_tray_fan_fault_test(cps_api_qualifier_t qual, uint_t slot)
{
    static const char *sdi_aliases[][2] = {
        { "Fantray 1 Fan 1", "Fantray 1 Fan 2" },
        { "Fantray 2 Fan 1", "Fantray 2 Fan 2" },
        { "Fantray 3 Fan 1", "Fantray 3 Fan 2" }
    };

    bool   passed = true;
    uint_t fan_idx;

    for (fan_idx = 1; fan_idx <= 2; ++fan_idx) {
        if (!fan_fault_test(qual,
                            PLATFORM_ENTITY_TYPE_FAN_TRAY,
                            slot,
                            fan_idx,
                            sdi_aliases[slot - 1][fan_idx - 1]
                            )
            ) {
            passed = false;
        }
    }

    return (passed);
}

TEST(pas_cps_test, fan_fault)
{
    test_init();

    EXPECT_TRUE(try_foreach_qual_slot(psu_fan_fault_test, 2));
    EXPECT_TRUE(try_foreach_qual_slot(fan_tray_fan_fault_test, 3));

    EXPECT_TRUE(test_fini());
}

#endif /* defined(VM) */

#ifdef VM 

/***************************************************************************
 *
 * Test fan speed errors
 */

bool fan_speed_err_test(
    cps_api_qualifier_t qual,
    uint_t              entity_type,
    uint_t              slot,
    uint_t              fan_idx,
    const char          *sdi_alias,
    uint_t              max_speed
                          )
{
    bool                 passed = true;
    cps_api_object_t     req_obj = CPS_API_OBJECT_NULL;
    cps_api_object_t     ev_obj = CPS_API_OBJECT_NULL;
    cps_api_object_t     resp_obj = CPS_API_OBJECT_NULL;
    sdi_entity_type_t    sdi_entity_type;
    sdi_entity_hdl_t     entity_hdl;
    sdi_resource_hdl_t   fan_resource_hdl;
    uint_t               k;
    bool                 fault;
    int                  set_speed;
    cps_api_qualifier_t  resp_qual;
    bool                 resp_entity_type_valid, resp_slot_valid, resp_fan_idx_valid;
    uint_t               resp_entity_type, resp_slot, resp_fan_idx;

    if (!sdi_entity_type_lkup(entity_type, &sdi_entity_type))  return (false);

    for (k = 0; k < 2; ++k) {
        fault = (k == 0);

        // Set entity fault in h/w simulation
    
        if (TRY_FALSE_ACCUM(sdi_db_get_entity_handle(db_hdl,
                                                     sdi_entity_type,
                                                     slot,
                                                     &entity_hdl
                                                     )
                            == STD_ERR_OK,
                            passed
                            )
            ) {
            continue;
        }

        if (TRY_FALSE_ACCUM(sdi_db_get_resource_handle_by_alias(db_hdl,
                                                                entity_hdl,
                                                                SDI_RESOURCE_FAN,
                                                                sdi_alias,
                                                                &fan_resource_hdl
                                                                )
                            == STD_ERR_OK,
                            passed
                            )
            ) {
            continue;
        }

        set_speed = max_speed;
        if (fault)  set_speed /= 2;

        if (TRY_FALSE_ACCUM(sdi_db_int_field_set(db_hdl,
                                                 fan_resource_hdl,
                                                 TABLE_FAN,
                                                 FAN_SPEED,
                                                 &set_speed
                                                 )
                            == STD_ERR_OK,
                            passed
                            )
            ) {
            continue;
        }
    
        // Do CPS get for entity
    
        sleep(3 * POLL_DELAY);

        req_obj = cps_api_object_create();
        if (TRY_FALSE_ACCUM(req_obj != CPS_API_OBJECT_NULL, passed)) {
            continue;
        }

        dn_pas_obj_key_fan_set(req_obj, qual, true, entity_type, true, slot, true, fan_idx);

        if (TRY_FALSE_ACCUM(cps_api_get1(req_obj, resp_obj), passed))  continue;

        do {
            if (TRY_FALSE_ACCUM(obj_subcat_chk(resp_obj, BASE_PAS_FAN_OBJ),
                                passed
                                )
                ) {
                break;
            }

            dn_pas_obj_key_fan_get(resp_obj,
                                   &resp_qual,
                                   &resp_entity_type_valid,
                                   &resp_entity_type,
                                   &resp_slot_valid,
                                   &resp_slot,
                                   &resp_fan_idx_valid,
                                   &resp_fan_idx
                                   );

            if (TRY_FALSE_ACCUM(resp_qual == qual
                                && resp_entity_type_valid
                                && resp_entity_type == entity_type
                                && resp_slot_valid
                                && resp_slot == slot
                                && resp_fan_idx_valid
                                && resp_fan_idx == fan_idx,
                                passed
                                )
                ) {
                break;
            }

            // Verify fault
            
            TRY_FALSE_ACCUM(attr_uint_chk(resp_obj,
                                          BASE_PAS_FAN_OPER_STATUS,
                                          fault
                                          ? BASE_CMN_OPER_STATUS_TYPE_FAIL
                                          : BASE_CMN_OPER_STATUS_TYPE_UP
                                          ),
                            passed
                            );
            
            TRY_FALSE_ACCUM(attr_uint_chk(resp_obj,
                                          BASE_PAS_FAN_FAULT_TYPE,
                                          fault
                                          ? PLATFORM_FAULT_TYPE_EHW
                                          : PLATFORM_FAULT_TYPE_OK
                                          ),
                            passed
                            );
        } while (0);
        
        cps_api_object_delete(resp_obj);
        
        // Check fan event recived
        
        ev_obj = cps_api_ev_next();
        if (!TRY_FALSE_ACCUM(ev_obj != CPS_API_OBJECT_NULL, passed)) {
            cps_api_qualifier_t ev_qual;
            bool                ev_entity_type_valid, ev_slot_valid, ev_fan_idx_valid;
            uint_t              ev_entity_type, ev_slot, ev_fan_idx;

            // Event received

            // Verify event

            do {
                if (TRY_FALSE_ACCUM(obj_subcat_chk(ev_obj, BASE_PAS_FAN_OBJ),
                                    passed
                                    )
                    ) {
                    break;
                }

                dn_pas_obj_key_fan_get(ev_obj,
                                       &ev_qual,
                                       &ev_entity_type_valid,
                                       &ev_entity_type,
                                       &ev_slot_valid,
                                       &ev_slot,
                                       &ev_fan_idx_valid,
                                       &ev_fan_idx
                                       );
                
                if (TRY_FALSE_ACCUM(ev_qual == cps_api_qualifier_OBSERVED
                                    && ev_entity_type_valid
                                    && ev_entity_type == entity_type
                                    && ev_slot_valid
                                    && ev_slot == slot
                                    && ev_fan_idx_valid
                                    && ev_fan_idx == fan_idx,
                                    passed
                                    )
                    ) {
                    break;
                }
                
                TRY_FALSE_ACCUM(attr_uint_chk(ev_obj,
                                              BASE_PAS_FAN_OPER_STATUS,
                                              fault
                                              ? BASE_CMN_OPER_STATUS_TYPE_FAIL
                                              : BASE_CMN_OPER_STATUS_TYPE_UP
                                              ),
                                passed
                                );
                
                TRY_FALSE_ACCUM(attr_uint_chk(ev_obj,
                                              BASE_PAS_FAN_FAULT_TYPE,
                                              fault
                                              ? PLATFORM_FAULT_TYPE_EHW
                                              : PLATFORM_FAULT_TYPE_OK
                                              ),
                                passed
                                );
            } while (0);

            cps_api_object_delete_safe(ev_obj);
        }
    }

    return (passed);
}

bool psu_fan_speed_err_test(cps_api_qualifier_t qual, uint_t slot)
{
    static const char *sdi_aliases[] = {
        "PSU 1 Fan", "PSU 2 Fan"
    };

    return (fan_speed_err_test(qual,
                               PLATFORM_ENTITY_TYPE_PSU,
                               slot,
                               1,
                               sdi_aliases[slot - 1],
                               19000
                               )
            );
}

bool fan_tray_fan_speed_err_test(cps_api_qualifier_t qual, uint_t slot)
{
    static const char *sdi_aliases[][2] = {
        { "Fantray 1 Fan 1", "Fantray 1 Fan 2" },
        { "Fantray 2 Fan 1", "Fantray 2 Fan 2" },
        { "Fantray 3 Fan 1", "Fantray 3 Fan 2" }
    };

    bool   passed = true;
    uint_t fan_idx;

    for (fan_idx = 1; fan_idx <= 2; ++fan_idx) {
        if (!fan_speed_err_test(qual,
                                PLATFORM_ENTITY_TYPE_FAN_TRAY,
                                slot,
                                fan_idx,
                                sdi_aliases[slot - 1][fan_idx - 1],
                                18000
                                )
            ) {
            passed = false;
        }
    }

    return (passed);
}

TEST(pas_cps_test, fan_speed_err)
{
    test_init();

    EXPECT_TRUE(try_foreach_qual_slot(psu_fan_speed_err_test, 2));
    EXPECT_TRUE(try_foreach_qual_slot(fan_tray_fan_speed_err_test, 3));

    EXPECT_TRUE(test_fini());
}

#endif /* defined(VM) */

/***************************************************************************
 *
 * Test setting fan speed
 */

#ifdef VM

bool db_fan_speed_set(
    uint_t     entity_type,
    uint_t     slot,
    const char *sdi_alias,
    uint_t     speed
                      )
{
    sdi_entity_type_t  sdi_entity_type;
    sdi_entity_hdl_t   entity_hdl;
    sdi_resource_hdl_t resource_hdl;

    return (sdi_entity_type_lkup(entity_type, &sdi_entity_type)
            && sdi_db_get_entity_handle(db_hdl,
                                        sdi_entity_type,
                                        slot,
                                        &entity_hdl
                                        )
                == STD_ERR_OK
            && sdi_db_get_resource_handle_by_alias(db_hdl,
                                                   entity_hdl,
                                                   SDI_RESOURCE_FAN,
                                                   sdi_alias,
                                                   &resource_hdl
                                                   )
                == STD_ERR_OK
            && sdi_db_int_field_set(db_hdl,
                                    resource_hdl,
                                    TABLE_FAN,
                                    FAN_SPEED,
                                    (int *) &speed
                                    )
                == STD_ERR_OK
            );
}

#endif

bool fan_speed_set_test(
    cps_api_qualifier_t qual,
    uint_t              entity_type,
    uint_t              slot,
    uint_t              fan_idx,
    const char          *sdi_alias,
    uint_t              max_speed,
    uint_t              speed_margin
                          )
{
    enum {
        MAX_TRIES = 10
    };

    bool                  passed = true, present, abort, speed_good;
    cps_api_object_t      req_obj = CPS_API_OBJECT_NULL;
    cps_api_object_t      resp_obj = CPS_API_OBJECT_NULL;
    u_int                 set_speed, fan_speed;
    cps_api_qualifier_t   resp_qual;
    bool                  resp_entity_type_valid, resp_slot_valid, resp_fan_idx_valid;
    uint_t                k, resp_entity_type, resp_slot, resp_fan_idx;
    cps_api_object_attr_t a;
    uint_t                try_cnt;

    if (TRY_FALSE(entity_present(qual, entity_type, slot, present))) {
        return (false);
    }
    if (!present) {
        fprintf(stderr,
                "***WARNING*** Entity %s not installed, skipping test\n",
                entity_names[entity_type - 1][slot - 1]
                );
        
        return (true);
    }

    for (k = 1; k <= 2; ++k) {
        set_speed = (k * max_speed) / 2;

        req_obj = cps_api_object_create();
        if (TRY_FALSE_ACCUM(req_obj != CPS_API_OBJECT_NULL, passed)) {
            continue;
        }

        dn_pas_obj_key_fan_set(req_obj,
                               cps_api_qualifier_TARGET,
                               true, entity_type,
                               true, slot,
                               true, fan_idx
                               );

        if (TRY_FALSE_ACCUM(cps_api_object_attr_add_u16(req_obj,
                                                        BASE_PAS_FAN_SPEED,
                                                        set_speed
                                                        ),
                            passed
                            )
            ) {

            cps_api_object_delete(req_obj);
            
            continue;
        }
        
        if (TRY_FALSE_ACCUM(cps_api_set1(req_obj), passed))  continue;
        
#ifdef VM
        if (TRY_FALSE_ACCUM(db_fan_speed_set(entity_type,
                                             slot,
                                             sdi_alias,
                                             set_speed),
                            passed
                            )
            ) {
            continue;
        }
#endif

        abort = speed_good = false;
        for (try_cnt = 0; !abort && !speed_good && try_cnt < MAX_TRIES; ++try_cnt) {
            sleep(POLL_DELAY);  // Wait for fan to ramp to new speed
            
            req_obj = cps_api_object_create();
            if (TRY_FALSE_ACCUM(req_obj != CPS_API_OBJECT_NULL, passed)) {
                break;
            }
            
            dn_pas_obj_key_fan_set(req_obj, qual, true, entity_type, true, slot, true, fan_idx);
            
            if (TRY_FALSE_ACCUM(cps_api_get1(req_obj, resp_obj), passed))  continue;

            do {
                if (TRY_FALSE_ACCUM(obj_subcat_chk(resp_obj, BASE_PAS_FAN_OBJ),
                                    passed
                                    )
                    ) {
                    break;
                }

                dn_pas_obj_key_fan_get(resp_obj,
                                       &resp_qual,
                                       &resp_entity_type_valid,
                                       &resp_entity_type,
                                       &resp_slot_valid,
                                       &resp_slot,
                                       &resp_fan_idx_valid,
                                       &resp_fan_idx
                                       );
                
                if (TRY_FALSE_ACCUM(resp_qual == qual
                                    && resp_entity_type_valid
                                    && resp_entity_type == entity_type
                                    && resp_slot_valid
                                    && resp_slot == slot
                                    && resp_fan_idx_valid
                                    && resp_fan_idx == fan_idx,
                                    passed
                                    )
                    ) {
                    break;
                }
                
                // Verify speed
                
                a = cps_api_object_attr_get(resp_obj, BASE_PAS_FAN_SPEED);
                if (TRY_FALSE_ACCUM(a != CPS_API_ATTR_NULL, passed))  break;

                fan_speed = cps_api_object_attr_data_u16(a);
                
                if (((100 * abs((int) fan_speed - (int) set_speed)) / set_speed) <= speed_margin) {
                    speed_good = true;
                }
            } while (0);
            
            cps_api_object_delete(resp_obj);
        }

        TRY_FALSE_ACCUM(speed_good, passed);
        
        // Check no fan event recived
        
        TRY_FALSE_ACCUM(cps_api_ev_next() == CPS_API_OBJECT_NULL, passed);
    }

    return (passed);
}

bool psu_fan_speed_set_test(cps_api_qualifier_t qual, uint_t slot)
{
    static const char *sdi_aliases[] = {
        "PSU 1 Fan", "PSU 2 Fan"
    };

    return (fan_speed_set_test(qual,
                               PLATFORM_ENTITY_TYPE_PSU,
                               slot,
                               1,
                               sdi_aliases[slot - 1],
                               19000,
                               10
                               )
            );
}

bool fan_tray_fan_speed_set_test(cps_api_qualifier_t qual, uint_t slot)
{
    static const char *sdi_aliases[][2] = {
        { "Fantray 1 Fan 1", "Fantray 1 Fan 2" },
        { "Fantray 2 Fan 1", "Fantray 2 Fan 2" },
        { "Fantray 3 Fan 1", "Fantray 3 Fan 2" }
    };

    bool   passed = true;
    uint_t fan_idx;

    for (fan_idx = 1; fan_idx <= 2; ++fan_idx) {
        if (!fan_speed_set_test(qual,
                                PLATFORM_ENTITY_TYPE_FAN_TRAY,
                                slot,
                                fan_idx,
                                sdi_aliases[slot - 1][fan_idx - 1],
                                18000,
                                10
                                )
            ) {
            passed = false;
        }
    }

    return (passed);
}

TEST(pas_cps_test, fan_speed_set)
{
    test_init();

    EXPECT_TRUE(try_foreach_qual_slot(psu_fan_speed_set_test, 2));
    EXPECT_TRUE(try_foreach_qual_slot(fan_tray_fan_speed_set_test, 3));

    EXPECT_TRUE(test_fini());
}

/***************************************************************************
 *
 * Test LED attributes
 */

bool led_attr_test(
    cps_api_qualifier_t qual,
    uint_t              entity_type,
    uint_t              slot,
    char                *name
                   )
{
    bool                 passed = true;
    cps_api_object_t     req_obj, resp_obj;
    cps_api_qualifier_t  resp_qual;
    bool                 resp_entity_type_valid;
    uint_t               resp_entity_type;
    bool                 resp_slot_valid;
    uint_t               resp_slot;
    bool                 resp_led_name_valid;
    char                 *resp_led_name;
    uint_t               resp_led_name_len;
    
    req_obj = cps_api_object_create();
    if (TRY_FALSE(req_obj != CPS_API_OBJECT_NULL)) {
        return (false);
    }

    dn_pas_obj_key_led_set(req_obj,
                           qual, 
                           true, entity_type,
                           true, slot,
                           true, name, strlen(name)
                           );

    if (TRY_FALSE(cps_api_get1(req_obj, resp_obj)))  return (false);

    do {
        if (TRY_FALSE_ACCUM(obj_subcat_chk(resp_obj, BASE_PAS_LED_OBJ),
                            passed
                            )
            ) {
            break;
        }

        dn_pas_obj_key_led_get(resp_obj,
                               &resp_qual,
                               &resp_entity_type_valid,
                               &resp_entity_type,
                               &resp_slot_valid,
                               &resp_slot,
                               &resp_led_name_valid,
                               &resp_led_name,
                               &resp_led_name_len
                               );
        
        if (TRY_FALSE_ACCUM(resp_qual == qual
                            && resp_entity_type_valid
                            && resp_entity_type == entity_type
                            && resp_slot_valid
                            && resp_slot == slot
                            && resp_led_name_valid
                            && resp_led_name_len == strlen(name)
                            && memcmp(resp_led_name, name, resp_led_name_len) == 0,
                            passed
                            )
            ) {
            break;
        }

        // Verify name
    
        TRY_FALSE_ACCUM(attr_str_chk(resp_obj, BASE_PAS_LED_NAME, name), passed);

        // Verify oper status
    
        TRY_FALSE_ACCUM(attr_uint_chk(resp_obj,
                                      BASE_PAS_LED_OPER_STATUS,
                                      BASE_CMN_OPER_STATUS_TYPE_UP
                                      ),
                        passed
                        );

        // Verify on/off state
    
        TRY_FALSE_ACCUM(attr_bool_chk(resp_obj, BASE_PAS_LED_ON, 0), passed);
    } while (0);
        
    cps_api_object_delete(resp_obj);
  
    return (passed);
}

bool card_led_attr_test(cps_api_qualifier_t qual)
{
    static const char * const names[] = {
       "Power POST State Led",
       "Power Normal State Led",
       "Power Failed state Led",
       "Locater Function Disable Led",
       "Locater Function Enable Led",
       "System Booting State Led",
       "System Normal State Led",
       "System Non-critical State Led",
       "System Critical State Led",
       "Master Slave Led"
    };
  
    bool   passed = true;
    uint_t i;
  
    for (i = 0; i < ARRAY_SIZE(names); ++i) {
        TRY_FALSE_ACCUM(led_attr_test(qual,
                                      PLATFORM_ENTITY_TYPE_CARD,
                                      1,
                                      (char *) names[i]
                                      ),
                        passed
                        );
    }

    return (passed);
}

TEST(pas_cps_test, led_attr)
{
    test_init();

    EXPECT_TRUE(try_foreach_qual(card_led_attr_test));

    EXPECT_TRUE(test_fini());
}

/***************************************************************************
 *
 * Test temperature sensor attributes
 */

bool temp_attr_test(
    cps_api_qualifier_t qual,
    uint_t              entity_type,
    uint_t              slot,
    char                *name
                    )
{
    bool                 passed = true;
    cps_api_object_t     req_obj, resp_obj;
    cps_api_qualifier_t  resp_qual;
    bool                 resp_entity_type_valid;
    uint_t               resp_entity_type;
    bool                 resp_slot_valid;
    uint_t               resp_slot;
    bool                 resp_sensor_name_valid;
    char                 *resp_sensor_name;
    uint_t               resp_sensor_name_len;
    
    req_obj = cps_api_object_create();
    if (TRY_FALSE(req_obj != CPS_API_OBJECT_NULL)) {
        return (false);
    }

    dn_pas_obj_key_temperature_set(req_obj,
                                   qual,
                                   true, entity_type,
                                   true, slot,
                                   true, name, strlen(name)
                                   );

    if (TRY_FALSE(cps_api_get1(req_obj, resp_obj)))  return (false);

    do {
        if (TRY_FALSE_ACCUM(obj_subcat_chk(resp_obj, BASE_PAS_TEMPERATURE_OBJ),
                            passed
                            )
            ) {
            break;
        }

        dn_pas_obj_key_temperature_get(resp_obj,
                                       &resp_qual,
                                       &resp_entity_type_valid,
                                       &resp_entity_type,
                                       &resp_slot_valid,
                                       &resp_slot,
                                       &resp_sensor_name_valid,
                                       &resp_sensor_name,
                                       &resp_sensor_name_len
                                       );

        if (TRY_FALSE_ACCUM(resp_qual == qual
                            && resp_entity_type_valid
                            && resp_entity_type == entity_type
                            && resp_slot_valid
                            && resp_slot == slot
                            && resp_sensor_name_valid
                            && resp_sensor_name_len == strlen(name)
                            && memcmp(resp_sensor_name, name, resp_sensor_name_len) == 0,
                            passed
                            )
            ) {
            break;
        }
         
        // Verify name
    
        TRY_FALSE_ACCUM(attr_str_chk(resp_obj, 
                                     BASE_PAS_TEMPERATURE_NAME,
                                     name
                                     ),
                        passed
                        );
    
        // Verify oper status
    
        TRY_FALSE_ACCUM(attr_uint_chk(resp_obj,
                                      BASE_PAS_TEMPERATURE_OPER_STATUS,
                                      BASE_CMN_OPER_STATUS_TYPE_UP
                                      ),
                        passed
                        );

        // Verify temperature
    
        TRY_FALSE_ACCUM(attr_chk_len(resp_obj, BASE_PAS_TEMPERATURE_TEMPERATURE, 1),
                        passed
                        );

        // Verify shutdown threshold
    
        TRY_FALSE_ACCUM(attr_chk_len(resp_obj, BASE_PAS_TEMPERATURE_SHUTDOWN_THRESHOLD, 1),
                        passed
                        );

        // Verify treshold enable
    
        TRY_FALSE_ACCUM(attr_bool_chk(resp_obj, BASE_PAS_TEMPERATURE_THRESH_ENABLE, 0),
                        passed
                        );

    } while (0);
        
    cps_api_object_delete(resp_obj);
  
    return (passed);
}

bool card_temp_attr_test(cps_api_qualifier_t qual)
{
    static const char * const names[] = {
        "T2 temp sensor",
        "system-NIC temp sensor",
        "Ambient temp sensor",
        "CPU0 temp sensor",
        "CPU1 temp sensor"
    };
  
    bool   passed = true;
    uint_t i;
  
    for (i = 0; i < ARRAY_SIZE(names); ++i) {
        TRY_FALSE_ACCUM(temp_attr_test(qual,
                                       PLATFORM_ENTITY_TYPE_CARD,
                                       1,
                                       (char *) names[i]),
                        passed
                        );
    }

    return (passed);
}

TEST(pas_cps_test, temp_attr)
{
    test_init();

    EXPECT_TRUE(try_foreach_qual(card_temp_attr_test));

    EXPECT_TRUE(test_fini());
}

#ifdef VM

/***************************************************************************
 *
 * Test temperature
 */

bool db_temperature_set(
    uint_t     entity_type,
    uint_t     slot,
    const char *sensor_name,
    int        temperature
                 )
{
    sdi_entity_type_t  sdi_entity_type;
    sdi_entity_hdl_t   entity_hdl;
    sdi_resource_hdl_t resource_hdl;

    return (sdi_entity_type_lkup(entity_type, &sdi_entity_type)
            && sdi_db_get_entity_handle(db_hdl,
                                        sdi_entity_type,
                                        slot,
                                        &entity_hdl
                                        )
                == STD_ERR_OK
            && sdi_db_get_resource_handle_by_alias(db_hdl,
                                                   entity_hdl,
                                                   SDI_RESOURCE_TEMPERATURE,
                                                   sensor_name,
                                                   &resource_hdl
                                                   )
                == STD_ERR_OK
            && sdi_db_int_field_set(db_hdl,
                                    resource_hdl,
                                    TABLE_THERMAL_SENSOR,
                                    THERMAL_TEMPERATURE,
                                    &temperature
                                    )
                == STD_ERR_OK
            );
}

bool temperature_test(
    cps_api_qualifier_t qual,
    uint_t              entity_type,
    uint_t              slot,
    const char          *sensor_name
                      )
{
    bool                 passed = true;
    cps_api_object_t     req_obj = CPS_API_OBJECT_NULL;
    cps_api_object_t     resp_obj = CPS_API_OBJECT_NULL;
    uint_t               k;
    cps_api_qualifier_t  resp_qual;
    bool                 resp_entity_type_valid, resp_slot_valid, resp_sensor_name_valid;
    uint_t               resp_entity_type, resp_slot, resp_sensor_name_len;
    char                 *resp_sensor_name;
    int                  temperature;

    for (k = 0; k < 2; ++k) {
        temperature = (int)(random_range(0, 100)) - 50;

        // Set entity temperature in h/w simulation
    
        if (TRY_FALSE_ACCUM(db_temperature_set(entity_type,
                                               slot,
                                               sensor_name,
                                               temperature
                                               ),
                            passed
                            )
            ) {
            continue;
        }

        // Do CPS get for entity
    
        sleep(POLL_DELAY);

        req_obj = cps_api_object_create();
        if (TRY_FALSE_ACCUM(req_obj != CPS_API_OBJECT_NULL, passed)) {
            continue;
        }

        dn_pas_obj_key_temperature_set(req_obj,
                                       qual,
                                       true, entity_type,
                                       true, slot,
                                       true, (char *) sensor_name, strlen(sensor_name)
                                       );

        if (TRY_FALSE_ACCUM(cps_api_get1(req_obj, resp_obj), passed))  continue;

        do {
            if (TRY_FALSE_ACCUM(obj_subcat_chk(resp_obj, BASE_PAS_TEMPERATURE_OBJ),
                                passed
                                )
                ) {
                break;
            }

            dn_pas_obj_key_temperature_get(resp_obj,
                                           &resp_qual,
                                           &resp_entity_type_valid,
                                           &resp_entity_type,
                                           &resp_slot_valid,
                                           &resp_slot,
                                           &resp_sensor_name_valid,
                                           &resp_sensor_name,
                                           &resp_sensor_name_len
                                   );

            if (TRY_FALSE_ACCUM(resp_qual == qual
                                && resp_entity_type_valid
                                && resp_entity_type == entity_type
                                && resp_slot_valid
                                && resp_slot == slot
                                && resp_sensor_name_valid
                                && resp_sensor_name_len == strlen(sensor_name)
                                && memcmp(resp_sensor_name, sensor_name, resp_sensor_name_len) == 0,
                                passed
                                )
                ) {
                break;
            }

            // Verify temperature
            
            TRY_FALSE_ACCUM(attr_int_chk(resp_obj,
                                         BASE_PAS_TEMPERATURE_TEMPERATURE,
                                         temperature
                                         ),
                            passed
                            );            
        } while (0);
        
        cps_api_object_delete(resp_obj);
    }

    return (passed);
}

bool card_temperature_test(cps_api_qualifier_t qual, uint_t slot)
{
    static const char * const sensor_names[] = {
        "T2 temp sensor",
        "system-NIC temp sensor",
        "Ambient temp sensor",
        "CPU0 temp sensor",
        "CPU1 temp sensor"
    };

    bool   passed = true;
    uint_t i;

    for (i = 0; i < ARRAY_SIZE(sensor_names); ++i) {
        TRY_FALSE_ACCUM(temperature_test(qual,
                                         PLATFORM_ENTITY_TYPE_CARD,
                                         slot,
                                         sensor_names[i]
                                         ),
                        passed
                        );
    }
    
    return (passed);
}

TEST(pas_cps_test, temperature)
{
    test_init();

    EXPECT_TRUE(try_foreach_qual_slot(card_temperature_test, 1));

    EXPECT_TRUE(test_fini());
}

#endif

#ifdef VM

/***************************************************************************
 *
 * Test temperature thresholds
 */

bool thresh_en_dis(
    uint_t     entity_type,
    uint_t     slot,
    const char *sensor_name,
    bool       en_dis
                   )
{
    cps_api_object_t req_obj;

    req_obj = cps_api_object_create();
    if (req_obj == CPS_API_OBJECT_NULL)  return (false);

    dn_pas_obj_key_temperature_set(req_obj,
                                   cps_api_qualifier_TARGET,
                                   true, entity_type,
                                   true, slot,
                                   true, (char *) sensor_name, strlen(sensor_name)
                                   );

    if (!cps_api_object_attr_add_u8(req_obj,
                                    BASE_PAS_TEMPERATURE_THRESH_ENABLE,
                                    en_dis
                                    )
        ) {
        cps_api_object_delete(req_obj);

        return (false);
    }

    return (cps_api_set1(req_obj));
}

bool thresh_clr(
    uint_t     entity_type,
    uint_t     slot,
    const char *sensor_name
                )
{
    bool                         result = false;
    cps_api_object_t             req_obj;
    cps_api_transaction_params_t xact[1];

    req_obj = cps_api_object_create();
    if (req_obj == CPS_API_OBJECT_NULL)  return (false);

    dn_pas_obj_key_temp_thresh_set(req_obj,
                                   cps_api_qualifier_TARGET,
                                   true, entity_type,
                                   true, slot,
                                   true, (char *) sensor_name, strlen(sensor_name),
                                   false, 0
                                   );

    if (cps_api_transaction_init(xact) == cps_api_ret_code_OK) {
        do {
            if (cps_api_delete(xact, req_obj) != cps_api_ret_code_OK)  break;
            
            req_obj = CPS_API_OBJECT_NULL; // No longer owned
            
            if (cps_api_commit(xact) != cps_api_ret_code_OK)  break;

            result = true;
        } while (0);
         
        cps_api_transaction_close(xact);
    }
    
    if (req_obj != CPS_API_OBJECT_NULL)  cps_api_object_delete(req_obj);
    
    return (result);
}

bool thresh_set(
    uint_t     entity_type,
    uint_t     slot,
    const char *sensor_name,
    uint_t     thresh_idx,
    int        hi,
    int        lo
                )
{
    cps_api_object_t req_obj;

    req_obj = cps_api_object_create();
    if (req_obj == CPS_API_OBJECT_NULL)  return (false);

    dn_pas_obj_key_temp_thresh_set(req_obj,
                                   cps_api_qualifier_TARGET,
                                   true, entity_type,
                                   true, slot,
                                   true, (char *) sensor_name, strlen(sensor_name),
                                   true, thresh_idx
                                   );

    if (!cps_api_object_attr_add_u16(req_obj,
                                     BASE_PAS_TEMP_THRESHOLD_HI,
                                     (uint_t) hi
                                     )
        || !cps_api_object_attr_add_u16(req_obj,
                                        BASE_PAS_TEMP_THRESHOLD_LO,
                                        (uint_t) lo
                                        )
        ) {
        cps_api_object_delete(req_obj);

        return (false);
    }

    return (cps_api_set1(req_obj));
}

bool thresh_ev_chk(uint_t entity_type, uint_t slot, const char *sensor_name, int temperature, int threshold)
{
    cps_api_object_t    ev_obj;
    cps_api_qualifier_t ev_qual;
    bool                ev_entity_type_valid, ev_slot_valid, ev_sensor_name_valid;
    uint_t              ev_entity_type, ev_slot, ev_sensor_name_len;
    char                *ev_sensor_name;

    ev_obj = cps_api_ev_next();
    if (TRY_FALSE(ev_obj != CPS_API_OBJECT_NULL))  return (false);
    
    if (TRY_FALSE(obj_subcat_chk(ev_obj, BASE_PAS_TEMPERATURE_OBJ)))  return (false);

    dn_pas_obj_key_temperature_get(ev_obj,
                                   &ev_qual,
                                   &ev_entity_type_valid, &ev_entity_type,
                                   &ev_slot_valid, &ev_slot,
                                   &ev_sensor_name_valid, &ev_sensor_name, &ev_sensor_name_len
                                   );

    if (TRY_FALSE(ev_qual == cps_api_qualifier_OBSERVED
                  && ev_entity_type_valid && ev_entity_type == entity_type
                  && ev_slot_valid && ev_slot == slot
                  && ev_sensor_name_valid && ev_sensor_name_len == strlen(sensor_name) && memcmp(ev_sensor_name, sensor_name, ev_sensor_name_len) == 0
                  )
        ) {
        fprintf(stderr, "ev_qual:\tgot %u, expected %u\n", ev_qual, cps_api_qualifier_OBSERVED);
        fprintf(stderr, "ev_entity_type_valid:\tgot %u, expected %u\n", ev_entity_type_valid, true);
        if (ev_entity_type_valid) {
            fprintf(stderr, "ev_entity_type:\tgot %u, expected %u\n", ev_entity_type, entity_type);
        }
        fprintf(stderr, "ev_slot_valid:\tgot %u, expected %u\n", ev_slot_valid, true);
        if (ev_slot_valid) {
            fprintf(stderr, "ev_slot:\tgot %u, expected %u\n", ev_slot, slot);
        }
        fprintf(stderr, "ev_sensor_name_valid:\tgot %u, expected %u\n", ev_sensor_name_valid, true);
        if (ev_sensor_name_valid) {
            fprintf(stderr, "ev_sensor_name:\tgot %s, expected %s\n", std::string(ev_sensor_name, 0, ev_sensor_name_len).c_str(), sensor_name);
        }

        return (false);
    }
    
    if (TRY_FALSE(attr_int_chk(ev_obj, BASE_PAS_TEMPERATURE_TEMPERATURE, temperature)
                  && attr_int_chk(ev_obj, BASE_PAS_TEMPERATURE_LAST_THRESH, threshold)
                  )
        ) {
        return (false);
    }

    return (true);
}

int thresh_cmp(const void *a, const void *b)
{
    if (*(int *) a < *(int *) b)  return (-1);
    if (*(int *) a > *(int *) b)  return (1);
    return (0);
}

bool thresh_test(
    cps_api_qualifier_t qual,
    uint_t              entity_type,
    uint_t              slot,
    const char          *sensor_name
                      )
{
    uint_t num_thresh = random_range(1, 10);
    int    temperature, threshold, thresholds[20];
    uint_t thresh_idxs[10], i, n;
    bool   start_hif;

    // Generate thresholds

    for (i = 0, n = 2 * num_thresh; n > 0; --n, ++i) {
        thresholds[i] = (int)(random_range(0, 100)) - 50;
    }

    qsort(thresholds, 2 * num_thresh, sizeof(thresholds[0]), thresh_cmp);

    for (i = 2; i < (2 * num_thresh); i += 2) {
        thresholds[i]     += 5 * (i / 2);
        if (random_range(1, 10) == 10) {        
            thresholds[i + 1] = thresholds[i];
        } else {
            thresholds[i + 1] += 5 * (i / 2);
        }
    }

    random_idxs(thresh_idxs, ARRAY_SIZE(thresh_idxs));

    // Disable threshold checking

    if (TRY_FALSE(thresh_en_dis(entity_type, slot, sensor_name, false)))  return (false);

    // Set initial temperature

    start_hif = (random_range(0, 1) == 0);
    
    if (start_hif) {
        threshold   = thresholds[2 * num_thresh - 1];
        temperature = threshold + 1;
    } else {
        threshold   = -9999;
        temperature = thresholds[1] - 1;
    }

    if (TRY_FALSE(db_temperature_set(entity_type, slot, sensor_name, temperature)))  return (false);

    sleep(POLL_DELAY);

    // Clear all thresholds

    if (TRY_FALSE(thresh_clr(entity_type, slot, sensor_name)))  return (false);

    // Set thresholds

    for (i = 0; i < num_thresh; ++i) {
        if (TRY_FALSE(thresh_set(entity_type, slot, sensor_name, thresh_idxs[i] + 1, thresholds[2 * i + 1], thresholds[2 * i])))  return (false);
    }

    // Enable threshold checking

    if (TRY_FALSE(thresh_en_dis(entity_type, slot, sensor_name, true)))  return (false);

    // Check for threshold crossing event

    sleep(POLL_DELAY);

    if (TRY_FALSE(thresh_ev_chk(entity_type, slot, sensor_name, temperature, threshold)))  return (false);

    // Set new temperature
    
    if (start_hif) {
        threshold   = thresholds[0];
        temperature = threshold - 1;
    } else {
        threshold   = thresholds[2 * num_thresh - 1];
        temperature = threshold + 1;
    }

    if (TRY_FALSE(db_temperature_set(entity_type, slot, sensor_name, temperature)))  return (false);

    // Check for threshold crossing event

    sleep(POLL_DELAY);

    if (TRY_FALSE(thresh_ev_chk(entity_type, slot, sensor_name, temperature, threshold)))  return (false);

    if (num_thresh <= 1)  return (true);

    // Set new temperature

    i = random_range(1, 2 * num_thresh - 2);
    if (start_hif)  i |= 1;  else  i &= ~1;
    threshold   = thresholds[i];
    temperature = threshold + (start_hif ?  2 : -2);

    if (TRY_FALSE(db_temperature_set(entity_type, slot, sensor_name, temperature)))  return (false);

    // Check for threshold crossing event

    sleep(POLL_DELAY);

    if (TRY_FALSE(thresh_ev_chk(entity_type, slot, sensor_name, temperature, threshold)))  return (false);

    if (i == 0)  return (true);

    // Set new temperature

    if (start_hif)  i &= ~1;  else  i |= 1;
    threshold   = thresholds[i];
    temperature = threshold + (start_hif ? -2 : 2);

    if (TRY_FALSE(db_temperature_set(entity_type, slot, sensor_name, temperature)))  return (false);

    // Check for threshold crossing event

    sleep(POLL_DELAY);

    if (TRY_FALSE(thresh_ev_chk(entity_type, slot, sensor_name, temperature, threshold)))  return (false);
    
    return (true);
}

bool card_temperature_thresh_test(cps_api_qualifier_t qual, uint_t slot)
{
    static const char * const sensor_names[] = {
        "T2 temp sensor",
        "system-NIC temp sensor",
        "Ambient temp sensor",
        "CPU0 temp sensor",
        "CPU1 temp sensor"
    };

    bool   passed = true;
    uint_t i;

    for (i = 0; i < ARRAY_SIZE(sensor_names); ++i) {
        TRY_FALSE_ACCUM(thresh_test(qual,
                                    PLATFORM_ENTITY_TYPE_CARD,
                                    slot,
                                    sensor_names[i]
                                    ),
                        passed
                        );
    }
    
    return (passed);
}

TEST(pas_cps_test, temperature_thresh)
{
    test_init();

    EXPECT_TRUE(try_foreach_qual_slot(card_temperature_thresh_test, 1));

    EXPECT_TRUE(test_fini());
}

#endif

/***************************************************************************
 *
 * Test LEDs
 */

bool led_test(
    cps_api_qualifier_t qual,
    uint_t              entity_type,
    uint_t              slot,
    char                *led_name,
    bool                on
              )
{
    bool                passed = true;
    cps_api_object_t    req_obj, resp_obj;
    cps_api_qualifier_t resp_qual;
    bool                resp_entity_type_valid, resp_slot_valid, resp_led_name_valid;
    uint_t              resp_entity_type, resp_slot, resp_led_name_len;
    char                *resp_led_name;

    printf("***INFO*** Turning LED %s %s\n", led_name, on ? "on" : "off");

    req_obj = cps_api_object_create();
    if (TRY_FALSE(req_obj != 0))  return (false);

    dn_pas_obj_key_led_set(req_obj,
                           cps_api_qualifier_TARGET,
                           true, entity_type,
                           true, slot,
                           true, led_name, strlen(led_name)
                           );

    if (TRY_FALSE(cps_api_object_attr_add_u8(req_obj,
                                             BASE_PAS_LED_ON,
                                             on != 0
                                             )
                  )
        ) {
        cps_api_object_delete(req_obj);

        return (false);
    }
    
    if (TRY_FALSE(cps_api_set1(req_obj)))  return (false);

    req_obj = cps_api_object_create();
    if (TRY_FALSE(req_obj != 0))  return (false);

    dn_pas_obj_key_led_set(req_obj,
                           qual,
                           true, entity_type,
                           true, slot,
                           true, led_name, strlen(led_name)
                           );

    if (TRY_FALSE(cps_api_get1(req_obj, resp_obj)))  return (false);

    dn_pas_obj_key_led_get(resp_obj,
                           &resp_qual,
                           &resp_entity_type_valid, &resp_entity_type,
                           &resp_slot_valid, &resp_slot,
                           &resp_led_name_valid, &resp_led_name, &resp_led_name_len
                           );
    
    TRY_FALSE_ACCUM(resp_qual == qual
                    && resp_entity_type_valid
                    && resp_entity_type == entity_type
                    && resp_slot_valid
                    && resp_slot == slot
                    && resp_led_name_valid
                    && resp_led_name_len == strlen(led_name)
                    && memcmp(resp_led_name, led_name, resp_led_name_len) == 0,
                    passed
                    );

    TRY_FALSE_ACCUM(attr_bool_chk(resp_obj, BASE_PAS_LED_ON, on), passed);

    cps_api_object_delete(resp_obj);

    return (passed);
}

bool card_led_test(cps_api_qualifier_t qual)
{
    static const char * const led_grp1[] = {
        "Front Panel FAN Normal Operation Led",
        "Front Panel FAN Absent Led",
        "Front Panel FAN Failed Led"
    };

    static const char * const led_grp2[] = {
        "Front Panel PSU Absent Led"
    };

    static const char * const led_grp3[] = {
        "Fan2 Normal Operation Led",
        "Fan2 Absent Led",
        "Fan2 Failed Led"
    };

    static const char * const led_grp4[] = {
        "Fan1 Normal Operation Led",
        "Fan1 Absent Led",
        "Fan1 Failed Led"
    };

    static const char * const led_grp5[] = {
        "Fan0 Normal Operation Led",
        "Fan0 Absent Led",
        "Fan0 Failed Led"
    };

    static const char * const led_grp6[] = {
        "Power POST State Led",
        "Power Normal State Led",
        "Power Failed state Led"
    };

    static const char * const led_grp7[] = {    
        "Locater Function Disable Led",
        "Locater Function Enable Led"
    };

    static const char * const led_grp8[] = {    
        "System Booting State Led",
        "System Normal State Led",
        "System Non-critical State Led",
        "System Critical State Led"
    };

    static const char * const led_grp9[] = {    
        "Master Slave Led"
    };

    static const struct {
        const char * const *names;
        uint_t             cnt;
    } led_grps[] = {
        { led_grp1, ARRAY_SIZE(led_grp1) },
        { led_grp2, ARRAY_SIZE(led_grp2) },
        { led_grp3, ARRAY_SIZE(led_grp3) },
        { led_grp4, ARRAY_SIZE(led_grp4) },
        { led_grp5, ARRAY_SIZE(led_grp5) },
        { led_grp6, ARRAY_SIZE(led_grp6) },
        { led_grp7, ARRAY_SIZE(led_grp7) },
        { led_grp8, ARRAY_SIZE(led_grp8) },
        { led_grp9, ARRAY_SIZE(led_grp9) }
    };

    bool   passed = true;
    uint_t i, j, n;

    // All LEDs off

    for (i = 0; i < ARRAY_SIZE(led_grps); ++i) {
        for (j = 0; j < led_grps[i].cnt; ++j) {
            TRY_FALSE_ACCUM(led_test(qual,
                                     PLATFORM_ENTITY_TYPE_CARD,
                                     1,
                                     (char *) led_grps[i].names[j],
                                     false
                                     ),
                            passed
                            );
        }
    }

    // All LEDs on and off, one-by-one

    for (i = 0; i < ARRAY_SIZE(led_grps); ++i) {
        for (j = 0; j < led_grps[i].cnt; ++j) {
            TRY_FALSE_ACCUM(led_test(qual,
                                     PLATFORM_ENTITY_TYPE_CARD,
                                     1,
                                     (char *) led_grps[i].names[j],
                                     true
                                     ),
                            passed
                            );
            sleep(5);

            TRY_FALSE_ACCUM(led_test(qual,
                                     PLATFORM_ENTITY_TYPE_CARD,
                                     1,
                                     (char *) led_grps[i].names[j],
                                     false
                                     ),
                            passed
                            );
            sleep(5);
        }
    }

    // All LEDs in some random groups, and random order

    for (n = random_range(2, 5); n; --n) {
        i = random_range(0, ARRAY_SIZE(led_grps) - 1);

        {
            uint_t idxs[led_grps[i].cnt];

            random_idxs(idxs, led_grps[i].cnt);

            for (j = 0; j < led_grps[i].cnt; ++j) {
                TRY_FALSE_ACCUM(led_test(qual,
                                         PLATFORM_ENTITY_TYPE_CARD,
                                         1,
                                         (char *) led_grps[i].names[idxs[j]],
                                         true
                                         ),
                                passed
                                );

                sleep(5);
            }
        }
    }


    return (passed);
}

TEST(pas_cps_test, leds)
{
    test_init();

    EXPECT_TRUE(try_foreach_qual(card_led_test));

    EXPECT_TRUE(test_fini());
}

/***************************************************************************
 *
 * Test displays
 */

bool disp_test(
    cps_api_qualifier_t qual,
    uint_t              entity_type,
    uint_t              slot,
    char                *disp_name, 
    char                *mesg
               )
{
    bool                passed = true;
    cps_api_object_t    req_obj, resp_obj;
    cps_api_qualifier_t resp_qual;
    bool                resp_entity_type_valid, resp_slot_valid, resp_disp_name_valid;
    uint_t              resp_entity_type, resp_slot, resp_disp_name_len;
    char                *resp_disp_name;

    printf("***INFO*** Displaying \"%s\"\n", mesg);

    req_obj = cps_api_object_create();
    if (TRY_FALSE(req_obj != 0))  return (false);

    dn_pas_obj_key_display_set(req_obj,
                               cps_api_qualifier_TARGET,
                               true, entity_type,
                               true, slot,
                               true, disp_name, strlen(disp_name)
                               );

    if (TRY_FALSE(cps_api_object_attr_add(req_obj,
                                          BASE_PAS_DISPLAY_MESSAGE,
                                          mesg,
                                          strlen(mesg)
                                          )
                  )
        ) {
        cps_api_object_delete(req_obj);

        return (false);
    }
    
    if (TRY_FALSE(cps_api_set1(req_obj)))  return (false);

    req_obj = cps_api_object_create();
    if (TRY_FALSE(req_obj != 0))  return (false);

    dn_pas_obj_key_display_set(req_obj,
                               qual,
                               true, entity_type,
                               true, slot,
                               true, disp_name, strlen(disp_name)
                               );

    if (TRY_FALSE(cps_api_get1(req_obj, resp_obj)))  return (false);

    dn_pas_obj_key_display_get(resp_obj,
                               &resp_qual,
                               &resp_entity_type_valid, &resp_entity_type,
                               &resp_slot_valid, &resp_slot,
                               &resp_disp_name_valid, &resp_disp_name, &resp_disp_name_len
                               );
    
    TRY_FALSE_ACCUM(resp_qual == qual
                    && resp_entity_type_valid
                    && resp_entity_type == entity_type
                    && resp_slot_valid
                    && resp_slot == slot
                    && resp_disp_name_valid
                    && resp_disp_name_len == strlen(disp_name)
                    && memcmp(resp_disp_name, disp_name, resp_disp_name_len) == 0,
                    passed
                    );

    TRY_FALSE_ACCUM(attr_str_chk(resp_obj, BASE_PAS_DISPLAY_MESSAGE, mesg), passed);

    cps_api_object_delete(resp_obj);

    return (passed);
}

bool card_disp_test(cps_api_qualifier_t qual)
{
    static const char * const disp_names[] = {
        "Stack Led"
    };

    bool   passed = true;
    uint_t i;
    char   buf[2] = { 0 };

    for (i = 0; i < ARRAY_SIZE(disp_names); ++i) {
        for (buf[0] = '0'; buf[0] <= '9'; ++buf[0]) {
            TRY_FALSE_ACCUM(disp_test(qual,
                                      PLATFORM_ENTITY_TYPE_CARD,
                                      1,
                                      (char *) disp_names[i],
                                      buf
                                      ),
                            passed
                            );
            sleep(5);
        }
        
        TRY_FALSE_ACCUM(disp_test(qual,
                                  PLATFORM_ENTITY_TYPE_CARD,
                                  1,
                                  (char *) disp_names[i],
                                  (char *) ""
                                  ),
                        passed
                        );
        sleep(5);
    }

    return (passed);
}

TEST(pas_cps_test, displays)
{
    test_init();

    EXPECT_TRUE(try_foreach_qual(card_disp_test));

    EXPECT_TRUE(test_fini());
}

/***************************************************************************
 *
 * Test FUSE fan tray
 */

bool fuse_fan_tray_test(uint_t slot)
{
    bool passed = true;
    FILE *fp = NULL;
    char filename[132], inp[8];

    do {
        snprintf(filename, sizeof(filename),
                 "%s/class/entity/fan_tray/%u/present",
                 fuse_mount,
                 slot
                 );
    
        fp = fopen(filename, "r");
        if (TRY_FALSE_ACCUM(fp != NULL, passed))  break;
    
        fgets(inp, sizeof(inp), fp);
        TRY_FALSE_ACCUM(strcmp(inp, "1") == 0, passed);
    } while (0);

    if (fp != NULL)  fclose(fp);

    return (passed);
}

TEST(pas_fuse_test, fan_tray)
{
    uint_t slot;

    test_init();

    for (slot = 1; slot <= 3; ++slot) {
        EXPECT_TRUE(fuse_fan_tray_test(slot));
    }

    EXPECT_TRUE(test_fini());
}


/** PAS Status test */
bool pas_status_test(
    cps_api_qualifier_t qual
                    )
{
    bool                 passed = true;
    cps_api_get_params_t get_req[1];
    cps_api_object_t     req_obj, resp_obj;
    cps_api_qualifier_t  resp_qual;
    bool                 resp_slot_valid;
    uint_t               resp_slot;
    
    req_obj = cps_api_object_create();
    if (TRY_FALSE(req_obj != CPS_API_OBJECT_NULL)) {
        return (false);
    }

    dn_pas_obj_key_pas_status_set(req_obj,
                                  qual,
                                  true, 1 
                                 );

    cps_api_get_request_init(get_req);
    
    do {
        if (TRY_FALSE_ACCUM(cps_api_object_list_append(get_req->filters, req_obj),
                            passed
                            )
            ) {
            cps_api_object_delete_safe(req_obj);

            break;
        }
        
        if (TRY_FALSE_ACCUM(cps_api_get(get_req) == cps_api_ret_code_OK,
                            passed
                            )
            || TRY_FALSE_ACCUM(cps_api_object_list_size(get_req->list) == 1,
                               passed
                               )
            ) {
            break;
        }

        resp_obj = cps_api_object_list_get(get_req->list, 0);
        if (TRY_FALSE_ACCUM(resp_obj != CPS_API_OBJECT_NULL, passed))  break;

        dn_pas_obj_key_pas_status_get(resp_obj,
                                      &resp_qual,
                                      &resp_slot_valid,
                                      &resp_slot
                                     );

        // Verify oper status
    
        TRY_FALSE_ACCUM(attr_bool_chk(resp_obj,
                                      BASE_PAS_READY_STATUS,
                                      true
                                     ),
                        passed
                        );

    } while (0);
        
    cps_api_get_request_close(get_req);
  
    return (passed);
}

bool status_test(cps_api_qualifier_t qual)
{
    return (pas_status_test(qual));
}

TEST(pas_cps_test, pas_status)
{
    test_init();

    EXPECT_TRUE(try_foreach_qual(status_test));
}

/************************************************************************
 *
 * Test entity set
 */

bool entity_reboot_set(cps_api_qualifier_t qual, uint_t slot, uint8_t reboot_type)
{
    bool                passed = true;
    cps_api_object_t    req_obj, resp_obj;
    cps_api_qualifier_t resp_qual;
    bool                resp_entity_type_valid, resp_slot_valid;
    uint_t              resp_entity_type, resp_slot;

    req_obj = cps_api_object_create();
    if (TRY_FALSE(req_obj != 0))  return (false);

    dn_pas_obj_key_entity_set(req_obj,
                              cps_api_qualifier_TARGET,
                              true, PLATFORM_ENTITY_TYPE_CARD,
                              true, slot
                             );

    if (TRY_FALSE(cps_api_object_attr_add_u8(req_obj,
                                             BASE_PAS_ENTITY_ENTITY_TYPE_CHOICE_CARD_REBOOT,
                                             reboot_type
                                             )
                  )
        ) {
        cps_api_object_delete(req_obj);

        return (false);
    }
    
    if (TRY_FALSE(cps_api_set1(req_obj)))  return (false);

    req_obj = cps_api_object_create();
    if (TRY_FALSE(req_obj != 0))  return (false);

    dn_pas_obj_key_entity_set(req_obj, qual, true, PLATFORM_ENTITY_TYPE_CARD, true, slot);

    if (TRY_FALSE(cps_api_get1(req_obj, resp_obj)))  return (false);

    dn_pas_obj_key_entity_get(resp_obj,
                              &resp_qual,
                              &resp_entity_type_valid, 
                              &resp_entity_type,
                              &resp_slot_valid, 
                              &resp_slot
                             );
    
    TRY_FALSE_ACCUM(resp_qual == qual
                    && resp_entity_type_valid
                    && resp_entity_type == PLATFORM_ENTITY_TYPE_CARD
                    && resp_slot_valid
                    && resp_slot == slot,
                    passed
                    );

    TRY_FALSE_ACCUM(attr_uint_chk(resp_obj, 
                                  BASE_PAS_ENTITY_ENTITY_TYPE_CHOICE_CARD_REBOOT, 
                                  reboot_type), 
                                  passed);

    cps_api_object_delete(resp_obj);

    return (passed);
}


bool entity_set_test(void)
{
    bool    passed = true;
    uint_t  i;

    for (i = 0; i < ARRAY_SIZE(reboot_type); ++i) {
        if (TRY_FALSE_ACCUM(entity_reboot_set(cps_api_qualifier_TARGET, 1, reboot_type[i]),
                            passed
                           )
           ) {
           break;
        }
        sleep(5);
    }

    return (passed);
}


TEST(pas_cps_test, entity_set_attr)
{
    test_init();

    EXPECT_TRUE(entity_set_test());

    EXPECT_TRUE(test_fini());
}

/************************************************************************
 *
 * Test chassis set
 */

bool chassis_set(cps_api_qualifier_t qual, uint8_t reboot_type)
{
    bool                passed = true;
    cps_api_object_t    req_obj, resp_obj;
    cps_api_qualifier_t resp_qual;
    uint8_t             resp_reboot_type = 0;

    req_obj = cps_api_object_create();
    if (TRY_FALSE(req_obj != 0))  return (false);

    dn_pas_obj_key_chassis_set(req_obj,
                           cps_api_qualifier_TARGET,
                           reboot_type
                           );

    if (TRY_FALSE(cps_api_set1(req_obj)))  return (false);

    req_obj = cps_api_object_create();
    if (TRY_FALSE(req_obj != 0))  return (false);

    dn_pas_obj_key_chassis_set(req_obj, qual, reboot_type);

    if (TRY_FALSE(cps_api_get1(req_obj, resp_obj)))  return (false);

    dn_pas_obj_key_chassis_get(resp_obj,
                           &resp_qual,
                           &resp_reboot_type
                           );
    
    TRY_FALSE_ACCUM(resp_qual == qual
                    && resp_reboot_type == reboot_type,
                    passed
                    );

    cps_api_object_delete(resp_obj);

    return (passed);
}

bool chassis_set_test(void)
{
    bool    passed = true;
    uint_t  i;

    for (i = 0; i < ARRAY_SIZE(reboot_type); ++i) {
        if (TRY_FALSE_ACCUM(chassis_set(cps_api_qualifier_TARGET, reboot_type[i]),
                            passed
                           )
           ) {
           break;
        }
        sleep(5);
    }

    return (passed);
}

TEST(pas_cps_test, chassis_set_attr)
{
    test_init();

    EXPECT_TRUE(chassis_set_test());

    EXPECT_TRUE(test_fini());
}
/*===========================================================================*/

std_event_server_handle_t es_hdl;

bool db_init(void)
{
#ifdef VM

    // Open the h/w simulation db

    if (sdi_db_open(&db_hdl, 0) != STD_ERR_OK) {
        printf("***ERROR*** Failed to open sdi db\n");

        db_hdl = 0;

        return (false);
    }

#endif

    return (true);
}

bool ev_init(void)
{
    cps_api_key_t       key[1];
    cps_api_event_reg_t reg[1];

    // Start CPS API event service

    if (cps_api_event_service_init() != cps_api_ret_code_OK) {
        printf("***ERROR*** cps_api_event_service_init() failed\n");

        return (false);
    }

    if (cps_api_event_thread_init() != cps_api_ret_code_OK) {
        printf("***ERROR*** cps_api_event_thread_init() failed\n");

        return (false);
    }

    // Register event handler

    cps_api_key_init(key,
                     cps_api_qualifier_OBSERVED,
                     cps_api_obj_CAT_BASE_PAS,
                     0,
                     0
                     );
    cps_api_key_set_len(key, 2);

    reg->objects           = key;
    reg->number_of_objects = 1;

    if (cps_api_event_thread_reg(reg, cps_api_ev_hdlr, 0)
        != cps_api_ret_code_OK
        ) {
        printf("***ERROR*** cps_api_event_thread_reg() failed\n");

        return (false);
    }

    return (true);
}

bool init(void)
{
    return (db_init() && ev_init());
}

void fini(void)
{
#ifdef VM

    if (db_hdl)  sdi_db_close(db_hdl);

#endif
}

int main(int argc, char **argv)
{
    int    result = 0;
    bool   run_tests = true;

    ::testing::InitGoogleTest(&argc, argv);

    if (!::testing::GTEST_FLAG(list_tests)) {
        // Skip initialization if just listing tests

        if (argc != 2) {
            printf("usage: %s [gtest-options] <fuse-mnt-dir>\n", argv[0]);

            exit(1);
        }

        fuse_mount = argv[1];
    
        run_tests = init();

        sleep(3);                   // Wait for pald to start
    }

    if (::testing::GTEST_FLAG(random_seed) == 0) {
        time_t now;

        time(&now);

        ::testing::GTEST_FLAG(random_seed) = now;
    }

    printf("***INFO*** Using random seed %u\n", ::testing::GTEST_FLAG(random_seed));

    srandom(::testing::GTEST_FLAG(random_seed));

    // Run test cases
  
    if (run_tests)  result = RUN_ALL_TESTS();
  
    fini();

    return (result);
}
