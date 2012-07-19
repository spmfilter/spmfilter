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

#define THIS_MODULE "email_address"

/** Creates a new SMFEmailAddress_T object */
SMFEmailAddress_T *smf_email_address_new(void) {
    CMimeAddress_T *ea = cmime_address_new();

    return (SMFEmailAddress_T *)ea;
}

/** Free SMFEmailAddress_T object */
void smf_email_address_free(SMFEmailAddress_T *ea) {
    assert(ea);
    cmime_address_free((CMimeAddress_T *)ea);
}

SMFEmailAddress_T *smf_email_address_parse_string(const char *addr) {
    CMimeAddress_T *a = NULL;

    assert(addr);

    a = cmime_address_parse_string(addr);
    return (SMFEmailAddress_T *)a;
}

char *smf_email_address_to_string(SMFEmailAddress_T *ea) {
    assert(ea);
    return (char *)cmime_address_to_string((CMimeAddress_T *)ea);
}

void smf_email_address_set_type(SMFEmailAddress_T *ea, SMFEmailAddressType_T t) {
    assert(ea);
    cmime_address_set_type((CMimeAddress_T *)ea,(CMimeAddressType_T )t);
}

SMFEmailAddressType_T smf_email_address_get_type(SMFEmailAddress_T *ea) {
    assert(ea);

    return (SMFEmailAddressType_T)cmime_address_get_type((CMimeAddress_T *)ea);
}

void smf_email_address_set_name(SMFEmailAddress_T *ea, const char *name) {
    assert(ea);
    assert(name);
    
    cmime_address_set_name((CMimeAddress_T *)ea,name);
}

char *smf_email_address_get_name(SMFEmailAddress_T *ea) {
    //CMimeAddress_T *ca = NULL;
    assert(ea);

    return (char *)cmime_address_get_name((CMimeAddress_T *)ea);
}

void smf_email_address_set_email(SMFEmailAddress_T *ea, const char *email) {
    assert(ea);
    assert(email);

    cmime_address_set_email((CMimeAddress_T *)ea,email);
}

char *smf_email_address_get_email(SMFEmailAddress_T *ea) {
    assert(ea);

    return (char *)cmime_address_get_email((CMimeAddress_T *)ea);
}
