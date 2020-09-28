/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2012 Axel Steiner, Sebastian Jäkel and SpaceNet AG
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

#define _GNU_SOURCE
#define THIS_MODULE "core"

#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/param.h>

#include "smf_core.h"
#include "smf_md5.h"

#define GETTIMEOFDAY(t) gettimeofday(t,(struct timezone *) 0)

char *smf_core_strstrip(char *s) {
    int start, end = strlen(s);
    for (start = 0; s[start] && isspace(s[start]); ++start) {}
    if (s[start]) {
        while (end > 0 && isspace(s[end-1]))
            --end;
    }
    memmove(s, &s[start], end - start);
    s[end - start] = '\0';
    
    return s;
}


char *smf_core_strlwc(char *s) {
    char *p;

    assert(s);
    
    p = s;

    while (*p) {
        *p = tolower(*p);
        p++;
    }
    return s;
}

char *smf_core_strcat_printf(char **s, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char *tmp = NULL;
    
    assert((*s));

    if (vasprintf(&tmp,fmt,ap) == -1) 
        return (*s);

    va_end(ap);
    (*s) = (char *)realloc((*s),strlen((*s)) + strlen(tmp) + sizeof(char));
    strcat((*s),tmp);
    free(tmp);
    return (*s);
}

char **smf_core_strsplit(const char *s, char *sep, int *nelems) {
    char **sl = NULL;   
    char *cp = NULL;
    char *tf = NULL;
    char *tmp = NULL;
    int count = 0;
    
    assert(s);
    assert(sep);
    
    tf = cp = strdup(s);
      
    while ((tmp = strsep(&cp, sep)) != NULL) {
        sl = (char**)realloc(sl, (sizeof(*sl) * (count+2)));
        sl[count] = strdup(tmp);
        count++; 
    }
    free(tf);
    sl[count] = '\0';

    if (nelems != NULL)
        *nelems = count;

    return(sl);
}

static void smf_core_strsplit_free(char **parts) {
    char **t = parts;

    while (*t != NULL) {
        free(*t);
        t++;
    }

    free(parts);
}

int smf_core_gen_queue_file(const char *queue_dir, char **tempname, const char *sid) {
    int fd;
    
    if (asprintf(&(*tempname),"%s/%s.XXXXXX",queue_dir,sid) == -1)
        return -1;

    if ((fd = mkstemp(*tempname)) == -1)
        return -1;
    close(fd);
    
    return 0;   
}

char *smf_core_md5sum(const char *data) {
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

char *smf_core_get_maildir_filename(void) {
    char *filename;
    char *hostname = NULL;
    struct timeval starttime;
    
    GETTIMEOFDAY(&starttime);
    hostname = (char *)malloc(MAXHOSTNAMELEN);
    gethostname(hostname,MAXHOSTNAMELEN);
    
    if (asprintf(&filename,"%lu.V%lu.%s",
            (unsigned long) starttime.tv_sec,
            (unsigned long) starttime.tv_usec,
            hostname) == -1) {
        free(hostname);
        return NULL;
    }

    free(hostname);
    return filename;
}

int smf_core_expand_string(const char *format, const char *addr, char **buf) {
    int rep_made = 0;
    int nelems;
    char **parts = smf_core_strsplit(addr, "@", &nelems);
    char *out;
    size_t out_size;
    int offs;

    assert(format != NULL);
    assert(addr != NULL);
    assert(buf != NULL);

    out_size = strlen(format) + 1;
    if ((out = malloc(out_size)) == NULL)  {
        return -1;
    }
    
    // Prepare the result with the format.
    // The '%<x>'-expressions are replaced later.
    strncpy(out, format, out_size);
    out[out_size - 1] = '\0';
    offs = 0;
    while(out[offs] != '\0') {
        if(out[offs] == '%') {
            const char *insert = NULL;

            switch(out[offs + 1]) {
                case 's':
                    insert = addr;
                    break;
                case 'u':
                    insert = parts[0];
                    break;
                case 'd':
                    if (nelems < 2) {
                        smf_core_strsplit_free(parts);
                        free(out);
                        return -1;
                    }

                    insert = parts[1];
                    break;
                default:
                    smf_core_strsplit_free(parts);
                    free(out);
                    return(-2);
                    break; /* never reached */
            }

            // Replace the both option-characters with the "insert"-string
            if (insert != NULL) {
                // New size of out: size of insert-string but without the two option-characters
                const size_t insert_len = strlen(insert);
                out_size += (insert_len - 2);
                out = realloc(out, out_size);
                
                // First move everything behind the option-characters
                memmove(out + offs + insert_len, out + offs + 2, strlen(out + offs + 2) + 1);
                // Now insert the "insert"-string at the current position
                memcpy(out + offs, insert, insert_len);
                offs += insert_len - 1;
            } else {
                offs++;
            }

            rep_made++;
        }

        offs++;
    }

    *buf = out;
    smf_core_strsplit_free(parts);
    return(rep_made);
}

int smf_core_copy_file(const char *source, const char *dest) {
    int out, result;
    
    if ((out = open(dest, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR)) == -1)
        return 0;
    
    result = smf_core_copy_to_fd(source, out);
    close(out);
    
    return result;
}

int smf_core_copy_to_fd(const char *source, int dest) {
    char buf[512];
    int in_fd;
    ssize_t nread;
    size_t nbytes = 0;
    
    if ((in_fd = open(source, O_RDONLY)) == -1)
        return -1;
    
    while ((nread = read(in_fd, buf, sizeof(buf))) != 0) {
        ssize_t nwritten = 0;
        
        if (nread == -1) {
            close(in_fd);
            return -1;
        }
        
        while (nwritten < nread) {
            ssize_t n;
            
            if ((n = write(dest, buf, nread)) == -1) {
                close(in_fd);
                return -1;
            }
            
            nwritten += n;
            nbytes += n;
        }
    }
    
    close(in_fd);

    return nbytes;
}

