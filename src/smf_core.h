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

#ifndef _SMF_CORE_H
#define	_SMF_CORE_H

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include <stdarg.h>

#include "spmfilter_config.h"
#include "smf_settings.h"
#include "smf_trace.h"
#include "smf_md5.h"

/**
 * @fn char *smf_core_strstrip(char *s)
 * @brief Removes leading and trailing whitespace from a string.
 * This function doesn't allocate or reallocate any memory; it 
 * modifies string in place. The pointer to string is returned 
 * to allow the nesting of functions.
 * @param s String to parse.
 * @return intput string
 */
char *smf_core_strstrip(char *s); 

/**
 * @fn char *smf_core_strlwc(const char *s)
 * @brief Convert a string to lowercase.
 * @param s String to convert.
 * @return ptr to string
 */
char *smf_core_strlwc(char *s);

/**
 * @fn char *smf_core_strcat_printf(char *s, const char *fmt, ...)
 * @brief Append format string to string and reallocate memory.
 * The pointer to string is returned to allow the nesting of functions.
 * @param string to append to 
 * @param fmt format string
 * @return new appended string
 */
char *smf_core_strcat_printf(char **s, const char *fmt, ...);

/**
 * @fn char **smf_core_strsplit(char *s, char *sep)
 * @brief Split a given string
 * @param s String to split
 * @param sep separator
 * @return a newly-allocated NULL-terminated array of strings. 
 */
char **smf_core_strsplit(char *s, char *sep);

/** 
 * @fn int smf_core_gen_queue_file(char *queue_dir, char **tempname)
 * @brief Generate a new queue file name
 * @param queue_dir path to queue directory
 * @param tempname pointer to unallocated buffer for filename, needs to
 * free'd by caller if not required anymore
 * @return 0 on success or -1 in case of error
 */
int smf_core_gen_queue_file(char *queue_dir, char **tempname);

/**
 * @fn char *smf_md5sum(const char *data)
 * @brief Generate md5 hexdigest for string
 * @param data String to generate md5sum for
 * @returns Pointer to hexdigest string on success, NULL on error
 */
char *smf_core_md5sum(const char *data);

/**
 * @fn char *smf_core_get_maildir_filename(void)
 * @brief Generates a unique maildir filename
 * @return newly allocated pointer with generated filename 
 * or NULL in case of error
 */
char *smf_core_get_maildir_filename(void);

/**
 * @fn int smf_core_expand_string(char *format, char *addr, char **buf)
 * @brief expands placeholders in a user querystring
 * @param format format string to use as input
 * @param addr email address to use for replacements
 * @param buf pointer to unallocated buffer for expanded format string, 
 *        needs to free'd by caller if not required anymore
 * @return the number of replacements made or -1 in case of error
 */
int smf_core_expand_string(char *format, char *addr, char **buf);

/** TODO: OLD STUFF **/


/** Extract a substring from given string
 *
 * \param pattern regular expression pattern
 * \param haystack string to search in
 * \param pos position to extract
 *
 * \returns extracted string
 */
char* smf_core_get_substring(const char *pattern, const char *haystack, int pos);


/** Check if given string is a valid email address
 *
 * \param addr string to check
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_core_valid_address(const char *addr);

#endif	/* _SMF_CORE_H */
