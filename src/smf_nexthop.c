/* spmfilter - mail filtering framework
 * Copyright (C) 2012 Robin Doer and SpaceNet AG
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "smf_nexthop.h"
#include "smf_smtp.h"
#include "smf_trace.h"

#define THIS_MODULE "nexthop"

static int copy_recipients(SMFMessage_T *from, SMFEnvelope_T *to) {
    SMFList_T *rcpts;
    SMFListElem_T *elem;
    
    rcpts = smf_message_get_recipients(from);
    elem = smf_list_head(rcpts);
    
    while(elem != NULL) {
        char *addr = smf_email_address_to_string((SMFEmailAddress_T*)smf_list_data(elem));
        smf_envelope_add_rcpt(to, addr);
        free(addr);
        elem = elem->next;
    }
    
    return smf_list_size(rcpts);
}

static int smtp_delivery_nexthop(SMFSettings_T *settings, SMFSession_T *session) {
    SMFEnvelope_T *env = smf_session_get_envelope(session);
    SMFSmtpStatus_T *status = NULL;

    STRACE(TRACE_DEBUG, session->id, "will now deliver to nexthop [%s] by SMTP", settings->nexthop);

    if (env->sender == NULL)
        smf_envelope_set_sender(env, "<>");

    if (env->recipients->size == 0 && copy_recipients(env->message, env) == 0) {
        STRACE(TRACE_ERR,session->id,"got no recipients");
        return -1;
    }

    if (env->nexthop == NULL)
        smf_envelope_set_nexthop(env, settings->nexthop);

    status = smf_smtp_deliver(env, settings->tls, session->message_file,session->id);
    if (status->code != 250) {
        STRACE(TRACE_ERR,session->id,"delivery to [%s] failed!",settings->nexthop);
        STRACE(TRACE_ERR,session->id,"nexthop said: %d - %s", status->code,status->text);
        return -1;
    }

    smf_smtp_status_free(status);

    return 0;
}

static int file_delivery_nexthop(SMFSettings_T *settings, SMFSession_T *session) {
    FILE *src, *dest;
    char block[512];
    size_t nbytes;
    int result = 0;
    
    assert(settings);
    assert(session);

    STRACE(TRACE_DEBUG, session->id, "will now deliver to nexthop-file [%s]", settings->nexthop);
    
    if ((src = fopen(session->message_file, "r")) == NULL) {
        STRACE(TRACE_ERR, session->id, "Failed to open %s for reading: %s",
            session->message_file, strerror(errno));
        return -1;
    }
    
    if ((dest = fopen(settings->nexthop, "w")) == NULL) {
        STRACE(TRACE_ERR, session->id, "Failed to open %s for writing: %s",
            settings->nexthop, strerror(errno));
        fclose(src);
        return -1;
    }
    
    while (!feof(src)) {
        if ((nbytes = fread(block, 1, sizeof(block), src)) == 0 && ferror(src)) {
            STRACE(TRACE_ERR, "Failed to read from %s: %s",
                session->message_file, strerror(ferror(src)));
            result = -1;
            break;
        }
        if ((nbytes = fwrite(block, 1, nbytes, dest)) == 0 && ferror(dest)) {
            STRACE(TRACE_ERR, "Failed to write to %s: %s",
                settings->nexthop, strerror(ferror(dest)));
            result = -1;
            break;
        }
    }
    
    fclose(src);
    fclose(dest);
    
    return result;
}

NexthopFunction smf_nexthop_find(SMFSettings_T *settings) {
    struct stat fstat;
    
    assert(settings);
    
    if (settings->nexthop == NULL) {
        // Nothing is configured, no action required
        TRACE(TRACE_DEBUG, "Skipping nexthop");
        return NULL;
    }
    
    if (lstat(settings->nexthop, &fstat) == 0 && !S_ISDIR(fstat.st_mode)) {
        // Assume, that you can write to this file, pipe or similar
        return file_delivery_nexthop;
    } else {
        // Assume, that a SMTP-destination (format <host>[:<port>]) is configured
        return smtp_delivery_nexthop;
    }
}
