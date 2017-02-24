/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2017 Axel Steiner and SpaceNet AG
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <sys/times.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <regex.h>
#include <stdint.h>
#include <pthread.h>

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/event.h>

#include "spmfilter_config.h"
#include "smf_smtpd.h"
#include "smf_smtp.h"
#include "smf_trace.h"
#include "smf_settings.h"
#include "smf_settings_private.h"
#include "smf_modules.h"
#include "smf_session.h"
#include "smf_core.h"
#include "smf_message.h"
#include "smf_message_private.h"
#include "smf_internal.h"
#include "smf_dict.h"
#include "smf_server.h"

#define THIS_MODULE "smtpd"

static int smf_smtpd_handle_q_error(SMFSettings_T *settings, SMFSession_T *session) {
    //switch (settings->module_fail) {
    //    case 1: return(1);
    //    case 2: smf_smtpd_code_reply(client,552,settings->smtp_codes);
    //            return(0);
    //    case 3: smf_smtpd_code_reply(client,451,settings->smtp_codes);
    //            return(0);
    //}

    return 0;
}

static int smf_smtpd_handle_q_processing_error(SMFSettings_T *settings, SMFSession_T *session, int retval) {
    //if (retval == -1) {
    //    switch (settings->module_fail) {
    //        case 1: return(1);
    //        case 2: smf_smtpd_code_reply(session->incoming,552,settings->smtp_codes);
    //                return(0);
    //        case 3: smf_smtpd_code_reply(session->incoming,451,settings->smtp_codes);
    //                return(0);
    //    }
    //} else if(retval == 1) {
    //    if (session->response_msg != NULL) {
    //        char *smtp_response;
    //        if (asprintf(&smtp_response, "250 %s\r\n",session->response_msg) != -1) {
    //            smf_smtpd_string_reply(session->incoming,smtp_response);
    //            free(smtp_response);
    //        } else
    //            smf_smtpd_string_reply(session->incoming,CODE_250_ACCEPTED);
    //    } else
    //        smf_smtpd_string_reply(session->incoming,CODE_250_ACCEPTED);
    //    return(1);
    //} else if(retval == 2) {
    //    return(2);
    //} else {
    //    if (session->response_msg != NULL) {
    //        char *smtp_response;
    //        if (asprintf(&smtp_response,"%d %s\r\n",retval,session->response_msg) != -1) {
    //            smf_smtpd_string_reply(session->incoming,smtp_response);
    //            free(smtp_response);
    //        } else
    //            smf_smtpd_code_reply(session->incoming,retval,settings->smtp_codes);
    //    } else
    //        smf_smtpd_code_reply(session->incoming,retval,settings->smtp_codes);
    //    return(1);
    //}

    /* if none of the above matched, halt processing, this is just
     * for safety purposes
     */
    STRACE(TRACE_DEBUG, session->id, "no conditional matched, will stop queue processing!");
    return(0);
}

/* handle nexthop delivery error */
static int smf_smtpd_handle_nexthop_error(SMFSettings_T *settings, SMFSession_T *session) {
    char *out = NULL;
    if (asprintf(&out, "%d %s\r\n",settings->nexthop_fail_code,settings->nexthop_fail_msg) == -1)
        TRACE(TRACE_ERR,"failed to write nexthop error message");
    //smf_smtpd_string_reply(session->incoming,out);
    free(out);
    return 0;
}

int smf_smtpd_process_modules(SMFServerClient_T *client) {
    SMFSession_T *session = client->session;
    SMFSettings_T *settings = client->settings;
    SMFProcessQueue_T *q = client->q;
    int ret;
    char *msg = NULL;
    /* now tun the process queue */
    ret = smf_modules_process(q,session,settings);

    if(ret == -1) {
        STRACE(TRACE_DEBUG, session->id, "smtpd engine failed!");
        return(-1);
    } else if (ret == 1) {
        return(0);
    }

    if (session->response_msg != NULL) {
        char *smtp_response;
        if (asprintf(&smtp_response,"250 %s\r\n",session->response_msg) != -1) {
            smf_smtpd_string_reply(client,smtp_response);
            free(smtp_response);
        } else
            smf_smtpd_string_reply(client,"250 Ok");
    } else {
        if (asprintf(&msg,"250 Ok: processed as %s\r\n",session->id) != -1) {
            smf_smtpd_string_reply(client,msg);
            free(msg);
        } else
            smf_smtpd_string_reply(client,"250 Ok");
    }
    return(0);
}

char *smf_smtpd_get_req_value(char *req, int jmp) {
    char *p = NULL;
    char *r = NULL;
    assert(req);

    p = req;
    p += jmp;

    /* jump over space, if exists */
    if (*p == (char)32) p++;
    r = strdup(p);
    
    return smf_core_strstrip(r);
}

/* dot-stuffing */
void smf_smtpd_stuffing(char chain[]) {
    int i, j;
    int found = 0;
    for (i=0,j=0; chain[i] != '\0'; i++) {
        if ((chain[i] != '.') || (found == 1)) {
            chain[j++]=chain[i];
        } else {
            found = 1;
        }
    }
    chain[j]='\0';
}

#define fputs_or_return(s, stream) \
    if (fputs(s, stream)<0) { \
        STRACE(TRACE_ERR,client->session->id,"failed to write queue file: %s (%d)",strerror(errno),errno); \
        fclose(stream); \
        return -1; \
    }

int smf_smtpd_append_missing_headers(SMFServerClient_T *client) {
    SMFSmtpdRuntimeData_T *rtd = (SMFSmtpdRuntimeData_T *)client->engine_data;
    int fd;
    FILE *new = NULL;
    FILE *old = NULL;
    char tmpname[PATH_MAX];
    size_t len;
    char buf[BUFSIZE];
    time_t currtime;  
    char *t1 = NULL;
    char *t2 = NULL;

    snprintf(tmpname, sizeof(tmpname), "%s/XXXXXX", client->settings->queue_dir);
    if ((fd = mkstemp(tmpname)) == -1) {
        STRACE(TRACE_ERR,client->session->id,"failed to create temporary file: %s (%d)",strerror(errno),errno);
        return -1;
    }
    
    close(fd);
    
    if((new = fopen(tmpname, "w"))==NULL) {
        STRACE(TRACE_ERR,client->session->id,"unable to open temporary file: %s (%d)",strerror(errno), errno);
        return -1;
    }

    if (rtd->found_mid==0) {
        t1 = smf_message_generate_message_id();
        if (asprintf(&t2,"Message-Id: %s%s",t1,rtd->nl) != -1) {
            fputs_or_return(t2, new);
            free(t2);
        }
        free(t1);
    }

    if (rtd->found_date==0) {
        time(&currtime);  
        t1 = calloc(BUFSIZE,sizeof(char));                                                   
        strftime(t1,BUFSIZE,"Date: %a, %d %b %Y %H:%M:%S %z (%Z)",localtime(&currtime));
        smf_core_strcat_printf(&t1, "%s", rtd->nl);
        fputs_or_return(t1, new);
        free(t1);
    }

    if (rtd->found_from==0) {
        if (asprintf(&t1,"From: %s%s",client->session->envelope->sender,rtd->nl) != -1) {
            fputs_or_return(t1, new);
            free(t1);
        }
    }

    if (rtd->found_to==0) {
        if (asprintf(&t1,"To: undisclosed-recipients:;%s",rtd->nl) != -1) {
            fputs_or_return(t1, new);
            free(t1);
        }
    }

    if (rtd->found_header==0) {
        if (asprintf(&t1,"%s",rtd->nl) != -1) {
            fputs_or_return(t1, new);
            free(t1);
        }
    }

    if((old = fopen(client->session->message_file, "r"))==NULL) {
        STRACE(TRACE_ERR,client->session->id,"unable to open queue file: %s (%d)",strerror(errno), errno);
        fclose(new);
        return -1;
    }

    while(!feof(old)) {
        if ((len = fread(&buf,sizeof(char),BUFSIZE,old)) < 0) {
            STRACE(TRACE_ERR,client->session->id,"failed to read queue file: %s (%d)",strerror(errno),errno);
            fclose(old);
            fclose(new);
            return -1;
        }
        if (fwrite(buf,sizeof(char),len,new) < 0) {
            STRACE(TRACE_ERR,client->session->id,"failed to write queue file: %s (%d)",strerror(errno),errno);
            fclose(old);
            fclose(new);
            return -1;
        }
    }
    fclose(old); 
    fclose(new);

    if (unlink(client->session->message_file)!=0) {
        STRACE(TRACE_ERR,client->session->id,"failed to remove queue file: %s (%d)",strerror(errno),errno);
        return -1;
    }

    if (rename(tmpname,client->session->message_file)!=0) {
        STRACE(TRACE_ERR,client->session->id,"failed to rename queue file: %s (%d)",strerror(errno),errno);
        return -1;
    }

    return 0;
}

/* smtp answer with format string as arg */
void smf_smtpd_string_reply(SMFServerClient_T *client, const char *format, ...) {
    char *out = NULL;
    va_list ap;
    struct evbuffer *tmp = evbuffer_new();

    va_start(ap, format);
    
    if (vasprintf(&out,format,ap) <= 0) {
        TRACE(TRACE_ERR,"failed to write message");
        return;
    }

    evbuffer_add(tmp, out, strlen(out));
    if (bufferevent_write_buffer(client->buf_ev, tmp)) {
        TRACE(TRACE_ERR,"Error sending data to client on fd %d\n", client->fd);
        smf_server_close_client(client);
    }
    free(out);
    va_end(ap);
    evbuffer_free(tmp);
}

void smf_smtpd_code_reply(SMFServerClient_T *client, int code, SMFDict_T *codes) {
    char *code_msg = NULL;
    char *code_str = NULL;
    char *out = NULL;
    struct evbuffer *tmp = evbuffer_new();

    if (asprintf(&code_str,"%d",code) != -1) {
        code_msg = smf_dict_get(codes,code_str);
        free(code_str);
    }
    
    if (code_msg!=NULL) {
        if (asprintf(&out,"%d %s\r\n",code,code_msg) == -1)
            TRACE(TRACE_ERR,"failed to write response message");
    } else {
        switch(code) {
            case 221:
                out = strdup(CODE_221);
                break;
            case 250:
                out = strdup(CODE_250);
                break;
            case 451:
                out = strdup(CODE_451);
                break;
            case 502:
                out = strdup(CODE_502);
                break;
            case 552:
                out = strdup(CODE_552);
                break;
            default:
                out = strdup(CODE_451);
                break;
        }
    }

    evbuffer_add(tmp,out,strlen(out));
    if (bufferevent_write_buffer(client->buf_ev, tmp)) {
        TRACE(TRACE_ERR,"Error sending data to client on fd %d\n", client->fd);
        smf_server_close_client(client);
    }
    evbuffer_free(tmp);
    free(out);
}

void smf_smtpd_process_quit(SMFServerClient_T *client, char *req) {
    SMFSmtpdRuntimeData_T *rtd = (SMFSmtpdRuntimeData_T *)client->engine_data;
    STRACE(TRACE_DEBUG,client->session->id,"SMTP: 'quit' received"); 
    smf_smtpd_code_reply(client,221,client->settings->smtp_codes);
    rtd->state = ST_QUIT;
}

void smf_smtpd_process_xforward(SMFServerClient_T *client, char *req) {
    SMFSmtpdRuntimeData_T *rtd = (SMFSmtpdRuntimeData_T *)client->engine_data;
    char *t = NULL;

    STRACE(TRACE_DEBUG,client->session->id,"SMTP: 'xforward' received");
    t = strcasestr(req,"ADDR=");
    if (t != NULL) {
        t = strchr(t,'=');
        smf_core_strstrip(++t);
        smf_session_set_xforward_addr(client->session,t);
        STRACE(TRACE_DEBUG,client->session->id,"session->xforward_addr: [%s]",smf_session_get_xforward_addr(client->session));
        smf_smtpd_code_reply(client,250,client->settings->smtp_codes);
        rtd->state = ST_XFWD;
    } else {
        smf_smtpd_string_reply(client,"501 Syntax: XFORWARD attribute=value...\r\n");
    }
}

void smf_smtpd_process_helo(SMFServerClient_T *client, char *req) {
    SMFSession_T *session = client->session;
    SMFSettings_T *settings = client->settings;
    SMFSmtpdRuntimeData_T *rtd = (SMFSmtpdRuntimeData_T *)client->engine_data; 
    char *req_value = NULL;

    /* An EHLO command MAY be issued by a client later in the session.
     * If it is issued after the session begins, the SMTP server MUST
     * clear all buffers and reset the state exactly as if a RSET
     * command had been issued.
     */ 
    if (rtd->state != ST_INIT) {
        smf_session_free(session);
        /* reinit session */
        session = smf_session_new();
        client->session = session;
        STRACE(TRACE_DEBUG,session->id,"session reset, helo/ehlo recieved not in init state");
    }
    STRACE(TRACE_DEBUG,session->id,"SMTP: 'helo/ehlo' received");
    req_value = smf_smtpd_get_req_value(req,4);
    smf_session_set_helo(session,req_value);
            
    if (strcmp(session->helo,"") == 0)  {
        smf_smtpd_string_reply(client,"501 Syntax: HELO hostname\r\n");
    } else {
        STRACE(TRACE_DEBUG,session->id,"session->helo: [%s]",smf_session_get_helo(session));

        if (strncasecmp(req, "ehlo", 4)==0) {
            smf_smtpd_string_reply(client,
                "250-%s\r\n250-XFORWARD ADDR\r\n250 SIZE %i\r\n",rtd->hostname,settings->max_size);
        } else {
            smf_smtpd_string_reply(client,"250 %s\r\n",rtd->hostname);
        }
        rtd->state = ST_HELO;
    } 
    free(req_value);
}

void smf_smtpd_process_from(SMFServerClient_T *client, char *req) {
    SMFSmtpdRuntimeData_T *rtd = (SMFSmtpdRuntimeData_T *)client->engine_data; 
    char *req_value = NULL;

    /* The MAIL command begins a mail transaction. Once started, 
     * a mail transaction consists of a transaction beginning command, 
     * one or more RCPT commands, and a DATA command, in that order. 
     * A mail transaction may be aborted by the RSET (or a new EHLO) 
     * command. There may be zero or more transactions in a session. 
     * MAIL MUST NOT be sent if a mail transaction is already open, 
     * e.g., it should be sent only if no mail transaction had been 
     * started in the session, or if the previous one successfully 
     * concluded with a successful DATA command, or if the previous 
     * one was aborted with a RSET.
     */

    STRACE(TRACE_DEBUG,client->session->id,"SMTP: 'mail from' received");
    if (rtd->state == ST_MAIL) {
        /* we already got the mail command */
        smf_smtpd_string_reply(client,"503 Error: nested MAIL command\r\n");
    } else {
        req_value = smf_smtpd_get_req_value(req,10);
        if (strcmp(req_value,"") == 0) {
            /* empty mail from? */
            smf_smtpd_string_reply(client,"501 Syntax: MAIL FROM:<address>\r\n");
        } else {
            smf_envelope_set_sender(client->session->envelope,req_value);
            STRACE(TRACE_DEBUG,client->session->id,"session->envelope->sender: [%s]",client->session->envelope->sender);
            smf_smtpd_code_reply(client,250,client->settings->smtp_codes);
            rtd->state = ST_MAIL;
        }
        free(req_value);
    }
}

void smf_smtpd_process_rset(SMFServerClient_T *client, char *req) {
    SMFSmtpdRuntimeData_T *rtd = (SMFSmtpdRuntimeData_T *)client->engine_data; 

    STRACE(TRACE_DEBUG,client->session->id,"SMTP: 'rset' received");
    smf_session_free(client->session);
    /* reinit session */
    client->session = smf_session_new();
    smf_smtpd_code_reply(client,250,client->settings->smtp_codes);
    rtd->state = ST_INIT;
}

void smf_smtpd_process_rcptto(SMFServerClient_T *client, char *req) {
    char *req_value = NULL;
    SMFSmtpdRuntimeData_T *rtd = (SMFSmtpdRuntimeData_T *)client->engine_data; 
    SMFListElem_T *elem = NULL;

    STRACE(TRACE_DEBUG,client->session->id,"SMTP: 'rcpt to' received");
    if ((rtd->state != ST_MAIL) && (rtd->state != ST_RCPT)) {
        /* someone wants to break smtp rules... */
        smf_smtpd_string_reply(client,"503 Error: need MAIL command\r\n");
    } else {
        req_value = smf_smtpd_get_req_value(req,8);
        if (strcmp(req_value,"") == 0) {
            /* empty rcpt to? */
            smf_smtpd_string_reply(client,"501 Syntax: RCPT TO:<address>\r\n");
        } else {
            smf_envelope_add_rcpt(client->session->envelope, req_value);
            smf_smtpd_code_reply(client,250,client->settings->smtp_codes);
            elem = smf_list_tail(client->session->envelope->recipients);
            STRACE(TRACE_DEBUG,client->session->id,"session->envelope->recipients: [%s]",(char *)smf_list_data(elem));
            rtd->state = ST_RCPT;
        }
        free(req_value);
    }
}

void smf_smtpd_process_data(SMFServerClient_T *client, char *req, size_t len) {
    SMFSettings_T *settings = client->settings;
    SMFSession_T *session = client->session;
    SMFSmtpdRuntimeData_T *rtd = (SMFSmtpdRuntimeData_T *)client->engine_data;
    //SMFProcessQueue_T *q = client->q;
    //ssize_t br;
    //char buf[MAXLINE];
    //char *req = NULL;
    //FILE *spool_file;
    SMFMessage_T *message = NULL;
    regex_t regex;
    int reti;
    char *mid = NULL;
    SMFListElem_T *e = NULL;

    reti = regcomp(&regex, "[A-Za-z0-9\\._-]*:.*", 0);

    if (session->message_file == NULL) {
	   if (smf_core_gen_queue_file(settings->queue_dir, &session->message_file, session->id) != 0) {
            STRACE(TRACE_ERR,session->id,"got no spool file path");
            smf_smtpd_code_reply(client, 552,settings->smtp_codes);
            // TODO: close client
            return;
        }

        /* open the spool file */
        client->spool_file = fopen(session->message_file, "w+");
        if(client->spool_file == NULL) {
            STRACE(TRACE_ERR,session->id,"unable to open spool file: %s (%d)",strerror(errno), errno);
            smf_smtpd_code_reply(client, 451, settings->smtp_codes);
            return;
        }

        STRACE(TRACE_DEBUG,session->id,"using spool file: '%s'", session->message_file); 
        smf_smtpd_string_reply(client,"354 End data with <CR><LF>.<CR><LF>\r\n");
    }
   
    if ((strncasecmp(req,".\r\n",3)!=0)&&(strncasecmp(req,".\n",2)!=0)) {
        if (strncasecmp(req,".",1)==0) smf_smtpd_stuffing(req);

        if ((strncasecmp(req,"Message-Id:",11)==0)&& (rtd->in_header==1)) rtd->found_mid = 1;
        if ((strncasecmp(req,"Date:",5)==0) && (rtd->in_header==1)) rtd->found_date = 1;
        if ((strncasecmp(req,"To:",3)==0) && (rtd->in_header==1)) rtd->found_to = 1;
        if ((strncasecmp(req,"From:",5)==0) && (rtd->in_header==1)) rtd->found_from = 1;

        if (rtd->nl == NULL) rtd->nl = smf_internal_determine_linebreak(req);

        if ((strncmp(req,"\n",1)==0)||(strncmp(req,"\r\n",2)==0)||(strncmp(req,"\r",1)==0))
            rtd->in_header = 0;

        if (rtd->found_header == 0) {
            reti = regexec(&regex, req, 0, NULL, 0);
            if(reti == 0) {
                rtd->found_header = 1;
            }
        }

        if (fwrite(req, sizeof(char), strlen(req), client->spool_file)<0) {
            STRACE(TRACE_ERR,session->id,"failed to write queue file: %s (%d)",strerror(errno),errno);
            smf_smtpd_code_reply(client, 451, settings->smtp_codes);
            fclose(client->spool_file);
            return;
        }
        session->message_size += len;
    } else{
        fclose(client->spool_file);
        STRACE(TRACE_DEBUG,session->id,"data complete, message size: %d", (u_int32_t)session->message_size);
        if ((rtd->found_mid==0)||(rtd->found_to==0)||(rtd->found_from==0)||(rtd->found_date==0)) 
            smf_smtpd_append_missing_headers(client);
    
        if ((session->message_size > smf_settings_get_max_size(settings))&&(smf_settings_get_max_size(settings) != 0)) {
            STRACE(TRACE_DEBUG,session->id,"max message size limit exceeded"); 
            smf_smtpd_string_reply(client,"552 message size exceeds fixed maximium message size\r\n");
        } else {
            message = smf_message_new();
            if(smf_message_from_file(&message,session->message_file,1) != 0) {
                STRACE(TRACE_ERR, session->id, "smf_message_from_file() failed");
                smf_smtpd_code_reply(client, 451, settings->smtp_codes);
                return;
            }

            mid = strdup(smf_message_get_message_id(message));
            mid = smf_core_strstrip(mid);
            STRACE(TRACE_INFO,session->id,"message-id=%s",mid);
            STRACE(TRACE_INFO,session->id,"from=<%s> size=%d",session->envelope->sender,(u_int32_t)session->message_size);
            e = smf_list_head(session->envelope->recipients);
            while(e != NULL) {
                STRACE(TRACE_INFO,session->id,"to=<%s> relay=%s",(char *)smf_list_data(e),settings->nexthop);
                e = e->next;
            }

            free(mid);
            session->envelope->message = message;
            smf_smtpd_process_modules(client);
        }


        STRACE(TRACE_DEBUG,session->id,"removing spool file %s",session->message_file);
        //if (remove(session->message_file) != 0)
        //    STRACE(TRACE_ERR,session->id,"failed to remove queue file: %s (%d)",strerror(errno),errno);
    }

    regfree(&regex);
}

static void smf_smtpd_write_cb(struct bufferevent *bev, void *arg) {
    SMFServerClient_T *client = (SMFServerClient_T *)arg;
    SMFSmtpdRuntimeData_T *rtd = (SMFSmtpdRuntimeData_T *)client->engine_data;

    if (rtd->state == ST_QUIT) {
        event_base_loopbreak(client->evbase);
    }
    
}

static void smf_smtpd_handle_client(struct bufferevent *bev, void *arg) {
    SMFServerClient_T *client = (SMFServerClient_T *)arg;
    SMFSettings_T *settings = client->settings;
    SMFSession_T *session = client->session;
    SMFSmtpdRuntimeData_T *rtd = (SMFSmtpdRuntimeData_T *)client->engine_data;
    struct evbuffer *input;
    char *req = NULL;
    size_t len = 0;

    input = bufferevent_get_input(bev);
    if ((len = evbuffer_get_length(input)) <= 0) {
        STRACE(TRACE_ERR,session->id,"invalid input");
        event_base_loopbreak(client->evbase);
    }

    req = calloc(sizeof(char), len + 1);
    if (evbuffer_remove(input, req, len) == -1) {
        STRACE(TRACE_ERR,session->id,"failed to read input buffer");
        return;
    }

    req[len] = '\0';
    // TODO: check states before cmd processing
    if (rtd->state != ST_DATA)
        STRACE(TRACE_DEBUG,session->id,"client smtp dialog: [%s]",req);

    if (strncasecmp(req,"quit",4)==0) {
        smf_smtpd_process_quit(client,req);
    } else if( (strncasecmp(req, "helo", 4)==0) || (strncasecmp(req, "ehlo", 4)==0)) {
        smf_smtpd_process_helo(client,req);
    } else if (strncasecmp(req,"xforward",8)==0) {
        smf_smtpd_process_xforward(client,req);
    } else if (strncasecmp(req, "mail from:", 10)==0) {
        smf_smtpd_process_from(client,req);
    } else if (strncasecmp(req, "rcpt to:", 8)==0) {
        smf_smtpd_process_rcptto(client,req);
    } else if (strncasecmp(req,"rset", 4)==0) {
        smf_smtpd_process_rset(client,req);
    } else if (strncasecmp(req, "noop", 4)==0) {
        STRACE(TRACE_DEBUG,session->id,"SMTP: 'noop' received");
        smf_smtpd_code_reply(client,250,settings->smtp_codes);
    } else if ((strncasecmp(req,"data", 4)==0) || (rtd->state == ST_DATA)) {
        if ((rtd->state != ST_DATA) && (rtd->state != ST_RCPT) && (rtd->state != ST_MAIL)) {
            /* someone wants to break smtp rules... */
            smf_smtpd_string_reply(client,"503 Error: need RCPT command\r\n");
        } else if ((rtd->state != ST_RCPT) && (rtd->state == ST_MAIL)) {
            /* we got the mail command but no rcpt to */
            smf_smtpd_string_reply(client,"554 Error: no valid recipients\r\n");
        } else {
            if (rtd->state != ST_DATA) {
                rtd->state = ST_DATA;
                STRACE(TRACE_DEBUG,session->id,"SMTP: 'data' received");
            }
            smf_smtpd_process_data(client,req,len);
        } 
    } else {
        STRACE(TRACE_DEBUG,session->id,"SMTP: got unknown command");
        smf_smtpd_string_reply(client,"502 Error: command not recognized\r\n");
    }

    free(req);
}

void smf_smtpd_timeout_cb(struct client *client) {
#if 0
    char *hostname = NULL;
    hostname = (char *)malloc(MAXHOSTNAMELEN);
    gethostname(hostname,MAXHOSTNAMELEN);
    smf_smtpd_string_reply(client,"421 %s Error: timeout exceeded\r\n",hostname);  
    free(hostname);
#endif
}

void smf_smtpd_engine_data_free(SMFServerClient_T *client) {
    SMFSmtpdRuntimeData_T *rtd = (SMFSmtpdRuntimeData_T *)client->engine_data;
   
    if (rtd->hostname != NULL)
        free(rtd->hostname);

    if (rtd->nl != NULL)
        free(rtd->nl);
}

/**
 * This function will be called by libevent when there is a connection
 * ready to be accepted.
 */
void smf_smtpd_accept_handler(struct evconnlistener *listener,
    evutil_socket_t fd, struct sockaddr *addr, int addrlen,
    void *arg) {
    SMFServerEngineCtx_T *ctx = (SMFServerEngineCtx_T *)arg;
    SMFServerClient_T *client = smf_server_client_create(fd, addr, addrlen);
    SMFServerJob_T *job;
    SMFSmtpdRuntimeData_T *rtd = NULL;
    SMFSession_T *session = smf_session_new();
    struct timeval tv;

    if ((rtd = malloc(sizeof(*rtd))) == NULL) {
        TRACE(TRACE_ERR,"failed to allocate memory for smtpd runtime data");
        return;
    }
    rtd->state = ST_INIT;
    rtd->hostname = (char *)malloc(MAXHOSTNAMELEN);
    rtd->found_mid = 0;
    rtd->found_from = 0;
    rtd->found_date = 0;
    rtd->found_to = 0;
    rtd->found_header = 0;
    rtd->in_header = 1;
    gethostname(rtd->hostname,MAXHOSTNAMELEN);
    rtd->nl = NULL;

    client->session = session;
    client->settings = ctx->settings;
    client->q = ctx->q;
    client->engine_data = (void *)rtd;
    client->engine_data_free_func = smf_smtpd_engine_data_free;
    bufferevent_setcb(client->buf_ev, smf_smtpd_handle_client, smf_smtpd_write_cb, smf_server_event_cb, client);
    bufferevent_enable(client->buf_ev, EV_READ|EV_WRITE);

    tv.tv_sec = ctx->settings->smtpd_timeout;
    tv.tv_usec = 0;
    client->timeout_cb = smf_smtpd_timeout_cb;
    bufferevent_set_timeouts(client->buf_ev, &tv, &tv);

    /* Create a job object and add it to the work queue. */
    if ((job = malloc(sizeof(*job))) == NULL) {
        TRACE(TRACE_WARNING,"failed to allocate memory for job state");
        smf_server_close_and_free_client(client);
        return;
    }
    job->job_function = smf_server_job_function;
    job->user_data = client;

    smf_server_workqueue_add_job(ctx->workqueue, job);
    TRACE(TRACE_DEBUG,"added job to workqueue");

    STRACE(TRACE_INFO,session->id, "connect from %s",client->client_addr);
    smf_smtpd_string_reply(client,"220 %s spmfilter\r\n",rtd->hostname);
}


int load(SMFSettings_T *settings) {
    SMFProcessQueue_T *q;
    SMFServerEngineCtx_T *ctx;

    /* initialize the modules queue handler */
    q = smf_modules_pqueue_init(
        smf_smtpd_handle_q_error,
        smf_smtpd_handle_q_processing_error,
        smf_smtpd_handle_nexthop_error
    );

    if(q == NULL) {
        TRACE(TRACE_ERR,"failed to initialize module queue");
        return -1;
    }

    ctx = (SMFServerEngineCtx_T *)calloc((size_t)1, sizeof(SMFServerEngineCtx_T));
    ctx->settings = settings;
    ctx->q = q;
    ctx->accept_cb = smf_smtpd_accept_handler;
    
    if (smf_server_listen(settings,ctx) != 0) {
        TRACE(TRACE_ERR,"failed to setup server");
        return -1;
    }
    
    free(q);

    return 0;
}

