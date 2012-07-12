/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2012 Axel Steiner and SpaceNet AG
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <cmime.h>

#include "smf_email_address.h"
#include "smf_lookup.h"
#include "smf_lookup_private.h"

#define THIS_MODULE "email_address"

/** Creates a new SMFEmailAddress_T object */
SMFEmailAddress_T *smf_email_address_new(void) {
    SMFEmailAddress_T *ea = NULL;
    CMimeAddress_T *ca = cmime_address_new();
    ea = g_slice_new(SMFEmailAddress_T);
    ea->data = ca;
    ea->lr = NULL;
    ea->is_local = -1;

    return ea;
}

/** Free SMFEmailAddress_T object */
void smf_email_address_free(SMFEmailAddress_T *ea) {
    assert(ea);
    if (ea->data != NULL)
        cmime_address_free((CMimeAddress_T *)ea->data);
    if (ea->lr != NULL)
        smf_lookup_result_free(ea->lr);    
    g_slice_free(SMFEmailAddress_T,ea);
}

SMFEmailAddress_T *smf_email_address_parse_string(const char *addr) {
    SMFEmailAddress_T *ea = smf_email_address_new();
    CMimeAddress_T *a = NULL;

    assert(addr);

    a = cmime_address_parse_string(addr);
    if (ea->data != NULL) {
        cmime_address_free(ea->data);
    }
    
    ea->data = (void *)a;
    return ea;
}

char *smf_email_address_to_string(SMFEmailAddress_T *ea) {
    assert(ea);
    return (char *)cmime_address_to_string((CMimeAddress_T *)ea->data);
}

void smf_email_address_set_type(SMFEmailAddress_T *ea, SMFEmailAddressType_T t) {
    assert(ea);
    if (ea->data != NULL) 
        cmime_address_set_type((CMimeAddress_T *)ea->data,(CMimeAddressType_T )t);
}

SMFEmailAddressType_T smf_email_address_get_type(SMFEmailAddress_T *ea) {
    CMimeAddress_T *ca = NULL;
    assert(ea);

    if (ea->data != NULL) {
        ca = (CMimeAddress_T *)ea->data;
        return (SMFEmailAddressType_T)cmime_address_get_type(ca);
    } else
        return -1;
}

void smf_email_address_set_name(SMFEmailAddress_T *ea, const char *name) {
    assert(ea);
    assert(name);
    
    if(ea->data)
        cmime_address_set_name((CMimeAddress_T *)ea->data,name);

}

char *smf_email_address_get_name(SMFEmailAddress_T *ea) {
    CMimeAddress_T *ca = NULL;
    assert(ea);

    if (ea->data != NULL) {
        ca = (CMimeAddress_T *)ea->data;
        return cmime_address_get_name(ca);
    }

    return NULL;
}

void smf_email_address_set_email(SMFEmailAddress_T *ea, const char *email) {
    assert(ea);
    assert(email);

    if (ea->data)
        cmime_address_set_email((CMimeAddress_T *)ea->data,email);
}

char *smf_email_address_get_email(SMFEmailAddress_T *ea) {
    CMimeAddress_T *ca = NULL;
    assert(ea);

    if (ea->data != NULL) {
        ca = (CMimeAddress_T *)ea->data;
        return cmime_address_get_email(ca);
    }

    return NULL;
}

void smf_email_address_set_lr(SMFEmailAddress_T *ea,SMFLookupResult_T *lr) {
    assert(ea);
    assert(lr);
    if (ea->lr != NULL) {
        smf_lookup_result_free(ea->lr);
    }
    ea->lr = smf_lookup_result_new();
    ea->lr = lr;
}

SMFLookupResult_T *smf_email_address_get_lr(SMFEmailAddress_T *ea) {
    return ea->lr;
}

