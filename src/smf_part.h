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

/*!
 * @file smf_part.h
 * @brief Defines the #SMFPart_T data type and related functions
 * @details #SMFPart_T represents an Mime part.
 * @details To create a new #SMFPart_T, use smf_part_new()
 * @details To destroy a #SMFPart_T use smf_part_free()
 */


#ifndef _SMF_PART_H
#define _SMF_PART_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cmime.h>

/*!
 * @typedef SMFPart_T
 * @brief A Mime part
 */
typedef CMimePart_T SMFPart_T;

/*!
 * @fn SMFPart_T *smf_part_new(void)
 * @brief Creates a new SMFPart_T object
 * @returns SMFPart_T pointer, or NULL on failure
 */
SMFPart_T *smf_part_new(void);

/*!
 * @fn void smf_part_free(SMFPart_T *part)
 * @brief Frees a SMFPart_T object
 * @param part a CMimePart_T object
 */
void smf_part_free(SMFPart_T *part);

/*!
 * @fn void smf_part_set_content_type(SMFPart_T *part, const char *s)
 * @brief Set the content type for a mime part
 * @param part a SMFPart_T object
 * @param s content type string
 */
void smf_part_set_content_type(SMFPart_T *part, const char *s);

/*!
 * @fn char *smf_part_get_content_type(SMFPart_T *part)
 * @brief Return content type of mime part
 * @param part a SMFPart_T object
 * @returns mime parts content type
 */
char *smf_part_get_content_type(SMFPart_T *part);

/*!
 * @fn void smf_part_set_content_disposition(SMFPart_T *part, const char *s)
 * @brief Set content disposition for mime part
 * @param part a SMFPart_T object
 * @param s content disosition string
 */
void smf_part_set_content_disposition(SMFPart_T *part, const char *s);

/*!
 * @fn char *smf_part_get_content_disposition(SMFPart_T *part)
 * @brief Return content disposition of mime part
 * @param part a SMFPart_T object
 * @returns mime parts content disposition
 */
char *smf_part_get_content_disposition(SMFPart_T *part);

/*!
 * @fn void smf_part_set_content_transfer_encoding(SMFPart_T *part, const char *s)
 * @brief Set content transfer encoding for mime part
 * @param part a SMFPart_T object 
 * @param s content transfer encoding string
 */
void smf_part_set_content_transfer_encoding(SMFPart_T *part, const char *s);

/*!
 * @fn char *smf_part_get_content_transfer_encoding(SMFPart_T *part)
 * @brief Return content transfer encoding of mime part
 * @param part a SMFPart_T object
 * @returns mime parts content transfer encoding
 */
char *smf_part_get_content_transfer_encoding(SMFPart_T *part);

/*!
 * @fn void smf_part_set_content_id(SMFPart_T *part, const char *s)
 * @brief Set content id for mime part
 * @param part a SMFPart_T object
 * @param s content id string
 */
void smf_part_set_content_id(SMFPart_T *part, const char *s);

/*!
 * @fn char *smf_part_get_content_id(SMFPart_T *part)
 * @brief Return content id of mime part
 * @param part a SMFPart_T object
 * @returns newly allocated char pointer with content id of mime part 
 */
char *smf_part_get_content_id(SMFPart_T *part);

/*!
 * @fn void smf_part_set_content(SMFPart_T *part, const char *s)
 * @brief Set content  for mime part
 * @param part a SMFPart_T object 
 * @param s mime part content
 */
void smf_part_set_content(SMFPart_T *part, const char *s);

/*!
 * @def smf_part_get_content(part)
 * @returns content of mime part
 */
#define smf_part_get_content(part) (part->content);

/*!
 * @fn void smf_part_set_postface(SMFPart_T *part, const char *s)
 * @brief Set mime parts postface
 * @param part a SMFPart_T object
 * @param s postface string
 */
void smf_part_set_postface(SMFPart_T *part, const char *s);

/*!
 * @def smf_part_get_postface(part)
 * @returns postface of mime part 
 */
#define smf_part_get_postface(part) (part->postface);

/*!
 * @fn char *smf_part_to_string(SMFPart_T *part, const char *nl)
 * @brief Return complete mime part as string, inclusive all mime headers
 * @param part a SMFPart_T object
 * @param nl newline character which should be used. If NULL newline
 *  character will be determined automatically.
 * @returns a newly allocated string with complete mime part
 */
char *smf_part_to_string(SMFPart_T *part, const char *nl);

/*!
 * @fn int smf_part_from_file(SMFPart_T **part, char *filename, const char *nl)
 * @brief Create a SMFPart_T object from file
 * @param part out param to return the new part
 * @param filename path to file
 * @param nl newline character which should be used. If NULL newline
 *  character will be determined automatically.
 * @returns 0 on success, -1 on stat error, -2 if not a regular file
 */
int smf_part_from_file(SMFPart_T **part, char *filename, const char *nl);

/*!
 * @fn int smf_part_to_file(SMFPart_T *part, char *filename)
 * @brief Create a file from SMFPart_T object
 * @param part SMFPart_T object
 * @param filename path to out file
 * @returns 0 on success, -1 on error
 */
int smf_part_to_file(SMFPart_T *part, char *filename);

/*!
 * @fn int smf_part_from_string(SMFPart_T **part, const char *content)
 * @brief Parse given string and create a new SMFPart_T object
 * @param part out param to return the new mime part object
 * @param content mime part string to parse
 * @returns 0 on success or -1 in case of error
 */
int smf_part_from_string(SMFPart_T **part, const char *content);

#ifdef __cplusplus
}
#endif

#endif  /* _SMF_PART_H */

