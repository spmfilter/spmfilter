/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2010 Axel Steiner and SpaceNet AG
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

/* GLIB2 VERSION INFORMATION */
#define GLIB2_VERSION (GLIB_MAJOR_VERSION * 10000 \
	+ GLIB_MINOR_VERSION * 100 \
	+ GLIB_MICRO_VERSION)


/** Generate a new queue file name
 *
 * \buf pointer to unallocated buffer for filename, needs to
 *      free'd by caller if not required anymore
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_core_gen_queue_file(char **tempanme);

/** Extract a substring from given string
 *
 * \param pattern regular expression pattern
 * \param haystack string to search in
 * \param pos position to extract
 *
 * \returns extracted string
 */
char* smf_core_get_substring(const char *pattern, const char *haystack, int pos);

/** Generates a unique maildir filename
 *
 * \returns generated filename or NULL in case of error
 */
char *smf_core_get_maildir_filename(void);

/** expands placeholders in a user querystring
 *
 * \param format format string to use as input
 * \param addr email address to use for replacements
 * \buf pointer to unallocated buffer for expanded format string, needs to
 *      free'd by caller if not required anymore
 *
 * \returns the number of replacements made or -1 in case of error
 */
int smf_core_expand_string(char *format, char *addr, char **buf);

struct _SMFObject_T {
	void *data;
};

/** Decreases the reference count of object. When its reference count drops to
 *  0, the object is finalized (i.e. its memory is freed).
 *
 * \param object pointer to object
 */
void smf_core_object_unref(void *object);

#endif	/* _SMF_CORE_H */

