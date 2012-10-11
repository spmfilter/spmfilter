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
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <regex.h>

#include "spmfilter_config.h"
#include "smf_smtpd.h"
#include "smf_trace.h"
#include "smf_settings.h"
#include "smf_modules.h"
#include "smf_session.h"
#include "smf_session_private.h"
#include "smf_core.h"
#include "smf_lookup.h"
#include "smf_lookup_private.h"
#include "smf_message.h"
#include "smf_message_private.h"
#include "smf_internal.h"
#include "smf_dict.h"
#include "smf_server.h"

#define THIS_MODULE "smtpd"

int client_sock = 0;

void smf_smtpd_timeout_handler(int sig) {
    char *hostname = NULL;
    TRACE(TRACE_DEBUG,"session timeout exceeded");

    hostname = (char *)malloc(MAXHOSTNAMELEN);
    gethostname(hostname,MAXHOSTNAMELEN);
    smf_smtpd_string_reply(client_sock,"421 %s Error: timeout exceeded\r\n",hostname);
    free(hostname);

    kill(getppid(),SIGUSR2);
    exit(0);
}


static int smf_smtpd_handle_q_error(SMFSettings_T *settings, SMFSession_T *session) {
    switch (settings->module_fail) {
        case 1: return(1);
        case 2: smf_smtpd_code_reply(session->sock,552,settings->smtp_codes);
                return(0);
        case 3: smf_smtpd_code_reply(session->sock,451,settings->smtp_codes);
                return(0);
    }

    return 0;
}

static int smf_smtpd_handle_q_processing_error(SMFSettings_T *settings, SMFSession_T *session, int retval) {
    if (retval == -1) {
        switch (settings->module_fail) {
            case 1: return(1);
            case 2: smf_smtpd_code_reply(session->sock,552,settings->smtp_codes);
                    return(0);
            case 3: smf_smtpd_code_reply(session->sock,451,settings->smtp_codes);
                    return(0);
        }
    } else if(retval == 1) {
        if (session->response_msg != NULL) {
            char *smtp_response;
            asprintf(&smtp_response, "250 %s\r\n",session->response_msg);
            smf_smtpd_string_reply(session->sock,smtp_response);
            free(smtp_response);
        } else
            smf_smtpd_string_reply(session->sock,CODE_250_ACCEPTED);
        return(1);
    } else if(retval == 2) {
        return(2);
    } else {
        if (session->response_msg != NULL) {
            char *smtp_response;
            asprintf(&smtp_response,"%d %s\r\n",retval,session->response_msg);
            smf_smtpd_string_reply(session->sock,smtp_response);
            free(smtp_response);
        } else
            smf_smtpd_code_reply(session->sock,retval,settings->smtp_codes);
        return(1);
    }

    /* if none of the above matched, halt processing, this is just
     * for safety purposes
     */
    STRACE(TRACE_DEBUG, session->id, "no conditional matched, will stop queue processing!");
    return(0);
}

/* handle nexthop delivery error */
static int smf_smtpd_handle_nexthop_error(SMFSettings_T *settings, SMFSession_T *session) {
    char *out = NULL;
    asprintf(&out, "%d %s\r\n",settings->nexthop_fail_code,settings->nexthop_fail_msg);
    smf_smtpd_string_reply(session->sock,out);
    free(out);
    return 0;
}

int smf_smtpd_load_modules(SMFSession_T *session, SMFSettings_T *settings) {
    int ret;
    SMFProcessQueue_T *q;

    /* initialize the modules queue handler */
    q = smf_modules_pqueue_init(
        smf_smtpd_handle_q_error,
        smf_smtpd_handle_q_processing_error,
        smf_smtpd_handle_nexthop_error
    );

    if(q == NULL) {
        return(-1);
    }

    /* now tun the process queue */
    ret = smf_modules_process(q,session,settings);
    free(q);

    if(ret == -1) {
        STRACE(TRACE_DEBUG, session->id, "smtpd engine failed!");
        return(-1);
    } else if (ret == 1) {
        return(0);
    }

    if (session->response_msg != NULL) {
        char *smtp_response;
        asprintf(&smtp_response,"250 %s\r\n",session->response_msg);
        smf_smtpd_string_reply(session->sock,smtp_response);
        free(smtp_response);
    } else
        smf_smtpd_string_reply(session->sock,CODE_250_ACCEPTED);
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

int smf_smtpd_append_missing_headers(SMFSession_T *session, char *queue_dir, int mid, int to, int from, int date, int headers, char *nl) {
    FILE *new = NULL;
    FILE *old = NULL;
    char *tmpname = NULL;
    size_t len;
    char buf[BUFSIZE];
    time_t currtime;  
    char *t1 = NULL;
    char *t2 = NULL;


    asprintf(&tmpname,"%s/XXXXXX",queue_dir);
    if(mkstemp(tmpname) == -1) {
        STRACE(TRACE_ERR,session->id,"failed to create temporary file: %s (%d)",strerror(errno),errno);
        return -1;
    }
    
    if((new = fopen(tmpname, "w"))==NULL) {
        STRACE(TRACE_ERR,session->id,"unable to open temporary file: %s (%d)",strerror(errno), errno);
        return -1;
    }

    if (mid==0) {
        t1 = smf_message_generate_message_id();
        asprintf(&t2,"Message-Id: %s%s",t1,nl);
        if (fputs(t2, new)<=0) {
            STRACE(TRACE_ERR,session->id,"failed to write queue file: %s (%d)",strerror(errno),errno);
            return -1;
        }
        free(t2);
        free(t1);
    }

    if (date==0) {
        time(&currtime);  
        t1 = calloc(BUFSIZE,sizeof(char));                                                   
        strftime(t1,BUFSIZE,"Date: %a, %d %b %Y %H:%M:%S %z (%Z)",localtime(&currtime));
        smf_core_strcat_printf(&t1, "%s", nl);
        if (fputs(t1, new)<=0) {
            STRACE(TRACE_ERR,session->id,"failed to write queue file: %s (%d)",strerror(errno),errno);
            return -1;
        }
        free(t1);
    }

    if (from==0) {
        asprintf(&t1,"From: %s%s",session->envelope->sender,nl);
        if (fputs(t1, new)<=0) {
            STRACE(TRACE_ERR,session->id,"failed to write queue file: %s (%d)",strerror(errno),errno);
            return -1;
        }
        free(t1);
    }

    if (to==0) {
        asprintf(&t1,"To: undisclosed-recipients:;%s",nl);
        if (fputs(t1, new)<=0) {
            STRACE(TRACE_ERR,session->id,"failed to write queue file: %s (%d)",strerror(errno),errno);
            return -1;
        }
        free(t1);
    }

    if (headers==0) {
        asprintf(&t1,"%s",nl);
        if (fputs(t1, new)<=0) {
            STRACE(TRACE_ERR,session->id,"failed to write queue file: %s (%d)",strerror(errno),errno);
            return -1;
        }
        free(t1);
    }

    if((old = fopen(session->message_file, "r"))==NULL) {
        STRACE(TRACE_ERR,session->id,"unable to open queue file: %s (%d)",strerror(errno), errno);
        return -1;
    }

    while(!feof(old)) {
        if ((len = fread(&buf,sizeof(char),BUFSIZE,old)) <= 0) {
            STRACE(TRACE_ERR,session->id,"failed to read queue file: %s (%d)",strerror(errno),errno);
            return -1;
        }
        if (fwrite(buf,sizeof(char),len,new) <= 0) {
            STRACE(TRACE_ERR,session->id,"failed to write queue file: %s (%d)",strerror(errno),errno);
            return -1;
        }
    }
    fclose(old); 
    fclose(new);

    if (unlink(session->message_file)!=0) {
        STRACE(TRACE_ERR,session->id,"failed to remove queue file: %s (%d)",strerror(errno),errno);
        return -1;
    }

    if (rename(tmpname,session->message_file)!=0) {
        STRACE(TRACE_ERR,session->id,"failed to rename queue file: %s (%d)",strerror(errno),errno);
        return -1;
    }

    free(tmpname);

    return 0;
}

/* smtp answer with format string as arg */
void smf_smtpd_string_reply(int sock, const char *format, ...) {
    ssize_t len = 0;
    char *out = NULL;
    va_list ap;

    va_start(ap, format);
    
    if (vasprintf(&out,format,ap) <= 0) {
        TRACE(TRACE_ERR,"failed to write message");
        return;
    }

    if ((len = smf_internal_writen(sock,out,strlen(out))) != strlen(out)) {
        TRACE(TRACE_WARNING, "unexpected size [%d], expected [%d] bytes",strlen(out),len);
    } 
    free(out);
    va_end(ap);
}

void smf_smtpd_code_reply(int sock, int code, SMFDict_T *codes) {
    char *code_msg = NULL;
    char *code_str = NULL;
    char *out = NULL;
    ssize_t len = 0;

    asprintf(&code_str,"%d",code);
    code_msg = smf_dict_get(codes,code_str);
    free(code_str);

    if (code_msg!=NULL) {
        asprintf(&out,"%d %s\r\n",code,code_msg);
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

    if ((len = smf_internal_writen(sock,out,strlen(out))) != strlen(out)) {
        TRACE(TRACE_WARNING, "unexpected size [%d], expected [%d] bytes",strlen(out),len);
    } 
    free(out);
}

void smf_smtpd_process_data(SMFSession_T *session, SMFSettings_T *settings) {
	ssize_t br;
    char buf[MAXLINE];
    void *rl = NULL;
    FILE *spool_file;
    SMFMessage_T *message = smf_message_new();
    int found_mid = 0;
    int found_to = 0;
    int found_from = 0;
    int found_date = 0;
    int found_header = 0;
    regex_t regex;
    int reti;
    char *nl = NULL;

    reti = regcomp(&regex, "[A-Za-z0-9\._-]*:.*", 0);

	smf_core_gen_queue_file(settings->queue_dir, &session->message_file, session->id);
    if (session->message_file == NULL) {
        STRACE(TRACE_ERR,session->id,"got no spool file path");
        smf_smtpd_code_reply(session->sock, 552,settings->smtp_codes);
        return;
    }
    
    /* open the spool file */
    spool_file = fopen(session->message_file, "w+");
    if(spool_file == NULL) {
        STRACE(TRACE_ERR,session->id,"unable to open spool file: %s (%d)",strerror(errno), errno);
        smf_smtpd_code_reply(session->sock, 451, settings->smtp_codes);
        return;
    }

    STRACE(TRACE_DEBUG,session->id,"using spool file: '%s'", session->message_file); 
    smf_smtpd_string_reply(session->sock,"354 End data with <CR><LF>.<CR><LF>\r\n");

    // TODO: max_size < message_size -> reject
    while((br = smf_internal_readline(session->sock,buf,MAXLINE,&rl)) > 0) {
        if ((strncasecmp(buf,".\r\n",3)==0)||(strncasecmp(buf,".\n",2)==0)) break;
        if (strncasecmp(buf,".",1)==0) smf_smtpd_stuffing(buf);

        if (strncasecmp(buf,"Message-Id:",11)==0) found_mid = 1;
        if (strncasecmp(buf,"Date:",5)==0) found_date = 1;
        if (strncasecmp(buf,"To:",3)==0) found_to = 1;
        if (strncasecmp(buf,"From:",5)==0) found_from = 1;

        if (nl == NULL) nl = smf_internal_determine_linebreak(buf);

        if (found_header == 0) {
            reti = regexec(&regex, buf, 0, NULL, 0);
            if(reti == 0){
                found_header = 1;
            }
        }

        if (fwrite(buf, sizeof(char), strlen(buf), spool_file)<=0) {
            STRACE(TRACE_ERR,session->id,"failed to write queue file: %s (%d)",strerror(errno),errno);
            smf_smtpd_code_reply(session->sock, 451, settings->smtp_codes);
            fclose(spool_file);
            return;
        }
        session->message_size += br;
    }
    regfree(&regex);
    fclose(spool_file);
  
    if ((found_mid==0)||(found_to==0)||(found_from==0)||(found_date==0)) 
        smf_smtpd_append_missing_headers(session, settings->queue_dir,found_mid,found_to,found_from,found_date,found_header,nl);
    
    
    TRACE(TRACE_DEBUG,"data complete, message size: %d", (u_int32_t)session->message_size);

    if(smf_message_from_file(&message,session->message_file,1) != 0) {
        STRACE(TRACE_ERR, session->id, "smf_message_from_file() failed");
        smf_smtpd_code_reply(session->sock, 451, settings->smtp_codes);
        return;
    }

    session->envelope->message = message;
    smf_smtpd_load_modules(session,settings);

    STRACE(TRACE_DEBUG,session->id,"removing spool file %s",session->message_file);
    if (remove(session->message_file) != 0)
        STRACE(TRACE_ERR,session->id,"failed to remove queue file: %s (%d)",strerror(errno),errno);
}

void smf_smtpd_handle_client(SMFSettings_T *settings, int client) {
    char *hostname = NULL;
    int br;
    void *rl = NULL;
    char req[MAXLINE];
    char *req_value = NULL;
    char *t = NULL;
    int state=ST_INIT;
    SMFSession_T *session = smf_session_new();
    SMFListElem_T *elem = NULL;
    struct tms start_acct;
    struct sigaction action;
    
    start_acct = smf_internal_init_runtime_stats();

    /* send signal to parent that we've got a new client */
    kill(getppid(),SIGUSR1);

    session->sock = client;
    client_sock = client;

    hostname = (char *)malloc(MAXHOSTNAMELEN);
    gethostname(hostname,MAXHOSTNAMELEN);
    smf_smtpd_string_reply(session->sock,"220 %s spmfilter\r\n",hostname);


    /* set timeout */
    action.sa_handler = smf_smtpd_timeout_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if (sigaction(SIGALRM, &action, NULL) < 0) {
        TRACE(TRACE_ERR,"sigaction (SIGALRM) failed: %s",strerror(errno));
        exit(EXIT_FAILURE);
    }
    alarm(settings->smtpd_timeout);

    for (;;) {
        if ((br = smf_internal_readline(session->sock,req,MAXLINE,&rl)) < 1) 
            break; /* EOF or error */

        STRACE(TRACE_DEBUG,session->id,"client smtp dialog: [%s]",req);

        if (strncasecmp(req,"quit",4)==0) {
            STRACE(TRACE_DEBUG,session->id,"SMTP: 'quit' received"); 
            smf_smtpd_code_reply(session->sock,221,settings->smtp_codes);
            state = ST_QUIT;
            break;
        } else if( (strncasecmp(req, "helo", 4)==0) || (strncasecmp(req, "ehlo", 4)==0)) {
            /* An EHLO command MAY be issued by a client later in the session.
             * If it is issued after the session begins, the SMTP server MUST
             * clear all buffers and reset the state exactly as if a RSET
             * command had been issued.
             */
            alarm(settings->smtpd_timeout);
            
            if (state != ST_INIT) {
                smf_session_free(session);
                /* reinit session */
                session = smf_session_new();
                session->sock = client;
                STRACE(TRACE_DEBUG,session->id,"session reset, helo/ehlo recieved not in init state");
            }
            STRACE(TRACE_DEBUG,session->id,"SMTP: 'helo/ehlo' received");
            req_value = smf_smtpd_get_req_value(req,4);
            smf_session_set_helo(session,req_value);
            if (strncmp(req_value,req,strlen(req_value)) != 0) {
                if (strcmp(session->helo,"") == 0)  {
                    smf_smtpd_string_reply(session->sock,"501 Syntax: HELO hostname\r\n");
                } else {
                    STRACE(TRACE_DEBUG,session->id,"session->helo: [%s]",smf_session_get_helo(session));

                    if (strncasecmp(req, "ehlo", 4)==0) {
                        smf_smtpd_string_reply(session->sock,
                            "250-%s\r\n250-XFORWARD ADDR\r\n250 SIZE %i\r\n",hostname,settings->max_size);
                    } else {
                        smf_smtpd_string_reply(session->sock,"250 %s\r\n",hostname);
                    }
                    state = ST_HELO;
                }
            } else {
                smf_smtpd_string_reply(session->sock,"501 Syntax: HELO hostname\r\n");
            }
            free(req_value);
        } else if (strncasecmp(req,"xforward",8)==0) {
            alarm(settings->smtpd_timeout);
            STRACE(TRACE_DEBUG,session->id,"SMTP: 'xforward' received");
            t = strcasestr(req,"ADDR=");
            if (t != NULL) {
                t = strchr(t,'=');
                smf_core_strstrip(++t);
                smf_session_set_xforward_addr(session,t);
                STRACE(TRACE_DEBUG,session->id,"session->xforward_addr: [%s]",smf_session_get_xforward_addr(session));
                smf_smtpd_code_reply(session->sock,250,settings->smtp_codes);
                state = ST_XFWD;
            } else {
                smf_smtpd_string_reply(session->sock,"501 Syntax: XFORWARD attribute=value...\r\n");
            }
        } else if (strncasecmp(req, "mail from:", 10)==0) {
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

            alarm(settings->smtpd_timeout);
            STRACE(TRACE_DEBUG,session->id,"SMTP: 'mail from' received");
            if (state == ST_MAIL) {
                /* we already got the mail command */
                smf_smtpd_string_reply(session->sock,"503 Error: nested MAIL command\r\n");
            } else {
                req_value = smf_smtpd_get_req_value(req,10);
                if (strcmp(req_value,"") == 0) {
                    /* empty mail from? */
                    smf_smtpd_string_reply(session->sock,"501 Syntax: MAIL FROM:<address>\r\n");
                } else {
                    smf_envelope_set_sender(session->envelope,req_value);
                    STRACE(TRACE_DEBUG,session->id,"session->envelope->sender: [%s]",session->envelope->sender);
                    smf_smtpd_code_reply(session->sock,250,settings->smtp_codes);
                    state = ST_MAIL;
                }
                free(req_value);
                
            }
        } else if (strncasecmp(req, "rcpt to:", 8)==0) {
            alarm(settings->smtpd_timeout);
            STRACE(TRACE_DEBUG,session->id,"SMTP: 'rcpt to' received");
            if ((state != ST_MAIL) && (state != ST_RCPT)) {
                /* someone wants to break smtp rules... */
                smf_smtpd_string_reply(session->sock,"503 Error: need MAIL command\r\n");
            } else {
                req_value = smf_smtpd_get_req_value(req,8);
                if (strcmp(req_value,"") == 0) {
                    /* empty rcpt to? */
                    smf_smtpd_string_reply(session->sock,"501 Syntax: RCPT TO:<address>\r\n");
                } else {
                    smf_envelope_add_rcpt(session->envelope, req_value);
                    smf_smtpd_code_reply(session->sock,250,settings->smtp_codes);
                    elem = smf_list_tail(session->envelope->recipients);
                    STRACE(TRACE_DEBUG,session->id,"session->envelope->recipients: [%s]",(char *)smf_list_data(elem));
                    state = ST_RCPT;
                }
                free(req_value);
            }
        } else if (strncasecmp(req,"data", 4)==0) {
            alarm(settings->smtpd_timeout);
            if ((state != ST_RCPT) && (state != ST_MAIL)) {
                /* someone wants to break smtp rules... */
                smf_smtpd_string_reply(session->sock,"503 Error: need RCPT command\r\n");
            } else if ((state != ST_RCPT) && (state == ST_MAIL)) {
                /* we got the mail command but no rcpt to */
                smf_smtpd_string_reply(session->sock,"554 Error: no valid recipients\r\n");
            } else {
                state = ST_DATA;
                STRACE(TRACE_DEBUG,session->id,"SMTP: 'data' received");
                smf_smtpd_process_data(session,settings);
            }
        } else if (strncasecmp(req,"rset", 4)==0) {
            alarm(settings->smtpd_timeout);
            STRACE(TRACE_DEBUG,session->id,"SMTP: 'rset' received");
            smf_session_free(session);
            /* reinit session */
            session = smf_session_new();
            session->sock = client;
            smf_smtpd_code_reply(session->sock,250,settings->smtp_codes);
            state = ST_INIT;
        } else if (strncasecmp(req, "noop", 4)==0) {
            alarm(settings->smtpd_timeout);
            STRACE(TRACE_DEBUG,session->id,"SMTP: 'noop' received");
            smf_smtpd_code_reply(session->sock,250,settings->smtp_codes);
        } else {
            alarm(settings->smtpd_timeout);
            STRACE(TRACE_DEBUG,session->id,"SMTP: got unknown command");
            smf_smtpd_string_reply(session->sock,"502 Error: command not recognized\r\n");
        }
    }
    free(rl);
    free(hostname);
    
    /* client has finished */
    kill(getppid(),SIGUSR2);

    smf_internal_print_runtime_stats(start_acct,session->id);
    smf_session_free(session);

    exit(0);
}

int load(SMFSettings_T *settings) {
    int sd;

    TRACE(TRACE_INFO,"starting smtpd engine");

    if ((sd = smf_server_listen(settings)) < 0)
        exit(EXIT_FAILURE);

    smf_server_init(settings,sd);
    smf_server_loop(settings,sd,smf_smtpd_handle_client);

    return 0;
}
