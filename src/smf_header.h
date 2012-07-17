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

#ifndef _SMF_HEADER_H
#define _SMF_HEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cmime.h>
#include "smf_core.h"


/*!
 * @struct SMFHeader_T smf_header.h
 * @brief Represents a message header
 */
typedef struct CMimeHeader_T SMFHeader_T;

/*!
 * @fn SMFHeader_T *smf_header_new(void)
 * @brief Creates a new SMFHeader_T object
 * @returns SMFHeader_T pointer, or NULL on failure
 */
SMFHeader_T *smf_header_new(void);

/*!
 * @fn void smf_header_free(SMFHeader_T *header)
 * @brief Free a SMFHeader_T object 
 * @param header SMFHeader_T pointer
 */
void smf_header_free(SMFHeader_T *header);

/*!
 * @fn int smf_header_set_name(SMFHeader_T *header, const char *name)
 * @brief Set a header name, if aleready exists, name will be overwritten
 * @param header SMFHeader_T pointer
 * @param name name of header
 */
void smf_header_set_name(SMFHeader_T *header, const char *name);

/*!
 * @fn char *smf_header_get_name(SMFHeader_T *header)
 * @brief Return header name
 * @param header SMFHeader_T object
 * @returns header name
 */
char *smf_header_get_name(SMFHeader_T *header);

/*!
 * @fn void smf_header_set_value(SMFHeader_T *header, const char *value, int overwrite)
 * @brief Set/append a header value
 * @param header SMFHeader_T pointer
 * @param value header value
 * @param overwrite 1 will overwrite value, 0 append
 */
void smf_header_set_value(SMFHeader_T *header, const char *value, int overwrite);

/*!
 * @fn char *smf_header_get_value(SMFHeader_T *header, int pos)
 * @brief Return header value (at given position, if header appears more than once)
 * @param header SMFHeader_T pointer
 * @param pos Position of appearance in email
 * @returns header value
 */
char *smf_header_get_value(SMFHeader_T *header,int pos);

/*! 
 * @fn int smf_header_get_count(SMFHeader_T *header)
 * @brief Get header value count
 * @param header SMFHeader_T object
 * @returns header value count
 */
int smf_header_get_count(SMFHeader_T *header);

/*!
 * @fn char *smf_header_to_string(SMFHeader_T *header)
 * @brief Return full header as newly allocated string
 * @param header a SMFHeader_T object
 * @returns full header as newly allocated string
 */
char *smf_header_to_string(SMFHeader_T *header);

#ifdef __cplusplus
}
#endif

#endif  /* _SMF_MESSAGE_H */