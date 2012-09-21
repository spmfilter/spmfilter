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

#ifndef _SMF_INTERNAL_H
#define _SMF_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <sys/times.h>

#define READCBUF 512
#define MAXLINE 512
#define BUFSIZE 512

#define CRLF "\r\n"
#define LF "\n"
#define CR "\r"

typedef struct {
    int count;
    char *current;
    char buf[READCBUF];
} readline_t;

void _string_list_destroy(void *data);
char *_build_module_path(const char *libdir, const char *modname);

/* removes leading < and trailing > 
 * returns a newlly allocated pointer */
char *_strip_email_addr(char *addr);

ssize_t _readn(int fd, void *buf, size_t nbyte);
ssize_t _writen(int fd, const void *buf, size_t nbyte);
ssize_t _readline(int fd, void *buf, size_t nbyte, void **help);
ssize_t _readcbuf(int fd, char *buf, readline_t *rl);

struct tms _init_runtime_stats(void);
void _print_runtime_stats(struct tms start_acct, const char *sid);
char *_determine_linebreak(const char *s);

#ifdef __cplusplus
}
#endif

#endif  /* _SMF_INTERNAL_H */
