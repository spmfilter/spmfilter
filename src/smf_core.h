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
 * @file smf_core.h
 * @brief Various helper functions
 */

#ifndef _SMF_CORE_H
#define	_SMF_CORE_H

/*!
 * @fn char *smf_core_strstrip(char *s)
 * @brief Removes leading and trailing whitespace from a string.
 *        This function doesn't allocate or reallocate any memory; it 
 *        modifies string in place. The pointer to string is returned 
 *        to allow the nesting of functions.
 * @param s String to parse.
 * @return intput string
 */
char *smf_core_strstrip(char *s); 

/*!
 * @fn char *smf_core_strlwc(char *s)
 * @brief Convert a string to lowercase.
 * @param s String to convert.
 * @return ptr to string
 */
char *smf_core_strlwc(char *s);

/*!
 * @fn char *smf_core_strcat_printf(char **s, const char *fmt, ...)
 * @brief Append format string to string and reallocate memory.
 *        The pointer to string is returned to allow the nesting of functions.
 * @param s string to append to 
 * @param fmt format string
 * @return new appended string
 */
char *smf_core_strcat_printf(char **s, const char *fmt, ...);

/*!
 * @fn char **smf_core_strsplit(const char *s, char *sep, int *nelems)
 * @brief Split a given string
 * @param s String to split
 * @param sep separator
 * @param nelems If set to non-NULL, then the function stores here the number of
 *               elements in the resulting array.
 * @return a newly-allocated NULL-terminated array of strings. 
 */
char **smf_core_strsplit(const char *s, char *sep, int *nelems);

/*!
 * @fn int smf_core_gen_queue_file(const char *queue_dir, char **tempname, const char *sid)
 * @brief Generate a new queue file name
 * @param queue_dir path to queue directory
 * @param tempname pointer to unallocated buffer for filename, needs to
 * free'd by caller if not required anymore
 * @param sid current session id
 * @return 0 on success or -1 in case of error
 */
int smf_core_gen_queue_file(const char *queue_dir, char **tempname, const char *sid);

/*!
 * @fn char *smf_core_md5sum(const char *data)
 * @brief Generate md5 hexdigest for string
 * @param data String to generate md5sum for
 * @returns Pointer to hexdigest string on success, NULL on error
 */
char *smf_core_md5sum(const char *data);

/*!
 * @fn char *smf_core_get_maildir_filename(void)
 * @brief Generates a unique maildir filename
 * @return newly allocated pointer with generated filename 
 * or NULL in case of error
 */
char *smf_core_get_maildir_filename(void);

/*!
 * @fn int smf_core_expand_string(const char *format, const char *addr, char **buf)
 * @brief expands placeholders in a user querystring
 * @param format format string to use as input
 * @param addr email address to use for replacements
 * @param buf pointer to unallocated buffer for expanded format string, 
 *        needs to free'd by caller if not required anymore
 * @return the number of replacements made or -1 in case of error
 */
int smf_core_expand_string(const char *format, const char *addr, char **buf);

/*!
 * @fn smf_core_copy_file(const char *source, const char *dest)
 * @brief Copies the content of the source file into dest
 * @param source Path of the source file
 * @param dest Path of the destination file. If file already exists, then the current
 *             content is replaced with the content of the source file.
 * @return the number of bytes copied
 */
int smf_core_copy_file(const char *source, const char *dest);

/*!
 * @fn int smf_core_copy_to_fd(const char *source, int dest)
 * @brief Copies the content of the source file into an already open file-descriptor.
 * This function is similar to smf_core_copy_file(). But rather than opening the destination
 * file, this function uses an already open file-descriptor as a destination.
 * @param source Path of the source file
 * @param dest On open file-descriptior, where the content is written into.
 * @return the number of bytes copied
 */
int smf_core_copy_to_fd(const char *source, int dest);

#endif	/* _SMF_CORE_H */
