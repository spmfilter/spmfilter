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

#include "smf_part.h"

#define THIS_MODULE "part"

SMFPart_T *smf_part_new(void) {
    return (SMFPart_T *)cmime_part_new();
}

void smf_part_free(SMFPart_T *part) {
    cmime_part_free((CMimePart_T *)part);
}

void smf_part_set_content_type(SMFPart_T *part, const char *s) {
    cmime_part_set_content_type((CMimePart_T *)part, s);
}

char *smf_part_get_content_type(SMFPart_T *part) {
    return cmime_part_get_content_type((CMimePart_T *)part);
}

void smf_part_set_content_disposition(SMFPart_T *part, const char *s) {
    cmime_part_set_content_disposition((CMimePart_T *)part,s);
}

char *smf_part_get_content_disposition(SMFPart_T *part) {
    return cmime_part_get_content_disposition((CMimePart_T *)part);
}

void smf_part_set_content_transfer_encoding(SMFPart_T *part, const char *s) {
    cmime_part_set_content_transfer_encoding((CMimePart_T *)part,s);
}

char *smf_part_get_content_transfer_encoding(SMFPart_T *part) {
    return cmime_part_get_content_transfer_encoding((CMimePart_T *)part);
}

void smf_part_set_content_id(SMFPart_T *part, const char *s) {
    cmime_part_set_content_id((CMimePart_T *)part,s);
}

char *smf_part_get_content_id(SMFPart_T *part) {
    return cmime_part_get_content_id((CMimePart_T *)part);
}

void smf_part_set_content(SMFPart_T *part, const char *s) {
    cmime_part_set_content((CMimePart_T *)part,s);
}

void smf_part_set_postface(SMFPart_T *part, const char *s) {
    cmime_part_set_postface((CMimePart_T *)part,s);
}

char *smf_part_to_string(SMFPart_T *part, const char *nl) {
    return cmime_part_to_string((CMimePart_T *)part,nl);
}

int smf_part_from_file(SMFPart_T **part, char *filename, const char *nl) {
    return cmime_part_from_file((CMimePart_T **)part,filename,nl);
}

int smf_part_to_file(SMFPart_T *part, char *filename) {
    return cmime_part_to_file((CMimePart_T *)part,filename);
}

int smf_part_from_string(SMFPart_T **part, const char *content) {
    return cmime_part_from_string((CMimePart_T **)part,content);
}

