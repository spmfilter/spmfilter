/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2010 Axel Steiner, Sebastian Jäkel and SpaceNet AG
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

#include "smf_core.h"


#define THIS_MODULE "utils"
#define GETTIMEOFDAY(t) gettimeofday(t,(struct timezone *) 0)

#define RE_MAIL "[a-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\\.[a-z0-9!#$%&'*+/=?^_`{|}~-]+)*@(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\\.)+[a-z0-9](?:[a-z0-9-]*[a-z0-9])?"


/** extract a substring from given string
 *
 * \param pattern regular expression pattern
 * \param haystack string to search in
 * \param pos position to extract
 *
 * \returns extracted string
 */
char *smf_core_get_substring(const char *pattern, const char *haystack, int pos) {
    if (haystack == NULL)
        return NULL;
    
    GRegex *re = NULL;
    GMatchInfo *match_info = NULL;
    char *value = NULL;

    re = g_regex_new(pattern, G_REGEX_CASELESS, 0, NULL);
    g_regex_match(re, haystack, 0, &match_info);
    if(g_match_info_matches(match_info)) {
        value = g_match_info_fetch(match_info, pos);
    } 
    
    g_match_info_free(match_info);
    g_regex_unref(re);

    return value;
}

/** Generate a new queue file name
 *
 * \buf pointer to unallocated buffer for filename, needs to
 *      free'd by caller if not required anymore
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_core_gen_queue_file(char *queue_dir, char **tempname) {
    /* create spooling file */
    *tempname = g_strdup_printf("%s/spmfilter.XXXXXX",queue_dir);
    if(g_mkstemp(*tempname) == -1)
        return -1;
    
    return 0;   
}

/** Generates a unique maildir filename
 *
 * \returns new allocated maildir filename
 */
char *smf_core_get_maildir_filename(void) {
    char *filename;
    char hostname[256];
    struct timeval starttime;
    
    GETTIMEOFDAY(&starttime);
    gethostname(hostname,256);
    
    filename = g_strdup_printf("%lu.V%lu.%s",
        (unsigned long) starttime.tv_sec,
        (unsigned long) starttime.tv_usec,
        hostname);
    
    return filename;
}

/** expands placeholders in a user querystring
 *
 * \param format format string to use as input
 * \param addr email address to use for replacements
 * \buf pointer to unallocated buffer for expanded format string, needs to
 *      free'd by caller if not required anymore
 *
 * \returns the number of replacements made or -1 in case of error
 */
int smf_core_expand_string(char *format, char *addr, char **buf) {
    int rep_made = 0;
    int pos = 0;
    int iter_size;
    char *it = format;
    char *iter;
    gchar **parts = g_strsplit(addr, "@", 2);

    /* allocate space for buffer
     * TODO: put buffer size declaration somewhere else
     */
    *buf = (char *)calloc(512,sizeof(char));
    if(*buf == NULL) {
        return(-1);
    }

    while(*it != '\0') {
        if(*it == '%') {
            *it++;
            switch(*it) {
                case 's':
                    iter = addr;
                    break;
                case 'u':
                    iter = parts[0];
                    break;
                case 'd':
                    iter = parts[1];
                    break;
                default:
                    return(-2);
                    break; /* never reached */
            }

            /* now copy the replacement text */
            iter_size = strlen(iter);
            memcpy((*buf + pos), iter, iter_size);
            pos += iter_size;

            *it++; /* jump over current */
            rep_made++;
        } else {
            (*buf)[pos++] = *it++;
        }
    }

    g_strfreev(parts);
    return(rep_made);
}

/*
void smf_core_object_unref(void *object) {
    g_object_unref(object);
}
*/

char *smf_md5sum(const char *data) {
    md5_state_t state;
    md5_byte_t digest[16];
    int offset = 0;
    char *hex = (char *)calloc(16*2+1,sizeof(char));

    md5_init(&state);
    md5_append(&state, (const md5_byte_t *)data, strlen(data));
    md5_finish(&state, digest);

    for(offset=0; offset<16; offset++) {
        sprintf(hex + offset * 2, "%02x", digest[offset]);
    }

    return(hex);
}

int smf_core_valid_address(const char *addr) {
    int match = -1;
    GRegex *re = NULL;
    GMatchInfo *match_info = NULL;

    re = g_regex_new(RE_MAIL, G_REGEX_CASELESS, 0, NULL);
    g_regex_match(re, addr, 0, &match_info);
    if(g_match_info_matches(match_info)) {
        match = 0;
    }

    g_match_info_free(match_info);
    g_regex_unref(re);

    return match;
}
