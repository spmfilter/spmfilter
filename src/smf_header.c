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

#include "smf_header.h"

#define THIS_MODULE "header"

SMFHeader_T *smf_header_new(void) {
    CMimeHeader_T *h = cmime_header_new();

    return (SMFHeader_T *)h;
}

void smf_header_free(SMFHeader_T *header) {
    assert(header);
    cmime_header_free((CMimeHeader_T *)header);
}

void smf_header_set_name(SMFHeader_T *header, const char *name) {
    assert(header);
    assert(name);

    cmime_header_set_name((CMimeHeader_T *)header,name);
}

char *smf_header_get_name(SMFHeader_T *header) {
    CMimeHeader_T *h = (CMimeHeader_T *)header;
    assert(header);
    return cmime_header_get_name(h);
}

void smf_header_set_value(SMFHeader_T *header, const char *value, int overwrite) {
    assert(header);
    assert(value);

    cmime_header_set_value((CMimeHeader_T *)header,value,overwrite);
}

char *smf_header_get_value(SMFHeader_T *header,int pos) {
    assert(header);
    return cmime_header_get_value((CMimeHeader_T *)header,pos);
}

char *smf_header_to_string(SMFHeader_T *header) {
    assert(header);
    return cmime_header_to_string((CMimeHeader_T *)header);
}

int smf_header_get_count(SMFHeader_T *header) {
    assert(header);
    return (int)((CMimeHeader_T *)header)->count;
}

