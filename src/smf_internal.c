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

#define THIS_MODULE "internal"
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>

#include "smf_internal.h"

void _string_list_destroy(void *data) {
    char *s = (char *)data;
    assert(data);
    free(s);
}

char *_build_module_path(const char *libdir, const char *modname) {
    char *path = NULL;
    char *t = NULL;

    if (strncmp(modname,"lib",3)==0) {
#ifdef __APPLE__
        asprintf(&t,"%s.dylib",modname);
#else
        t = strdup(modname);
#endif
    } else {
#ifdef __APPLE__
        asprintf(&t,"lib%s.dylib", modname);
#else
        asprintf(&t,"lib%s.so",modname);
#endif
    }
    asprintf(&path,"%s/%s",libdir,t);
    free(t);

    return path;
}

char *_strip_email_addr(char *addr) {
    char *p1 = NULL;
    char *p2 = NULL;
    char *out = NULL;
    size_t len1, len2, offset;  
    printf("IN [%s]\n",addr);
    if ((*addr != (char)32) && (*addr == '<')) {
        p2 = addr;
        p1 = ++p2;
    } else
        p1 = strchr(addr,'<');
    
    if (p1 != NULL) {
        len1 = strlen(++p1);
        if ((p2 = strchr(p1,'>')) != NULL) {
            len2 = strlen(p2);
            offset = len1 - len2;
            out = (char *)calloc(offset + 1, sizeof(char));
            strncpy(out,p1,offset);
            out[strlen(out)] = '\0';
        } else 
            out = strdup(addr);
    } else
        out = strdup(addr);
    
    printf("OUT [%s]\n",out);
    return out;
}

ssize_t _readn(int fd, void *buf, size_t nbyte) {
    size_t n;
    ssize_t br;
    char *p = buf;

    for (n = nbyte; n > 0; n -= br, p += br) {
        if ((br = read(fd,p,n)) < 0) {
            if (errno == EINTR)
                br = 0;
            else
                return -1;
        } else if (br == 0)
            return (nbyte - n);
    }

    return nbyte;
}

ssize_t _writen(int fd, const void *buf, size_t nbyte) {
    size_t n;
    ssize_t bw;
    const char *p = buf;

    for (n = nbyte; n > 0; n -= bw, p += bw) {
        if ((bw = write(fd,p,n)) < 0) {
            if (errno == EINTR)
                bw = 0;
            else
                return -1;
        } else if (bw == 0)
            return (nbyte - 1);
    }

    return nbyte;
}

ssize_t _readline(int fd, void *buf, size_t nbyte, void **help) {
    size_t n;
    ssize_t br;
    char c, *ptr = buf;
    readline_t *rl = *help;

    if (rl == NULL) {
        if ((rl = malloc(sizeof(readline_t))) == NULL)
            return -1;

        rl->count = 0;
        rl->current = rl->buf; 
        *help = rl;
    }   

    for (n = 1; n < nbyte; n++) {
        if ((br = _readcbuf(fd,&c,rl)) < 0)
            return -1;

        *ptr++ = c;
        if ((br == 0) || ( c == '\n'))
            break;
    }

    if ((br == 0) && (n == 1))
        return 0;

    *ptr = 0;
    return n;
}

ssize_t _readcbuf(int fd, char *buf, readline_t *rl) {
    while(rl->count < 1) {
        if ((rl->count = read(fd, rl->buf, sizeof(rl->buf))) < 0) {
            if (errno == EINTR)
                rl->count = 0;
            else
                return -1;
        } else if (rl->count == 0)
            return 0;

        rl->current = rl->buf;
    }

    *buf = *rl->current++;
    rl->count--;

    return 1;
}
