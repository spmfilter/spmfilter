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
#include <glib.h>
#include <time.h>
#include <gmodule.h>
/*#include <glib/gstdio.h> */
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

#include "spmfilter_config.h"
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

#define THIS_MODULE "smtpd"

#define CODE_221 "221 Goodbye. Please recommend us to others!\r\n"
#define CODE_250 "250 OK\r\n"
#define CODE_250_ACCEPTED "250 OK message accepted\r\n"
#define CODE_451 "451 Requested action aborted: local error in processing\r\n"
#define CODE_502 "502 Command not implemented\r\n"
#define CODE_552 "552 Requested action aborted: local error in processing\r\n"

/* SMTP States */
#define ST_INIT 0
#define ST_HELO 1
#define ST_XFWD 2
#define ST_MAIL 3
#define ST_RCPT 4
#define ST_DATA 5
#define ST_QUIT 6

char *_get_req_value(char *req, int jmp) {
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

/* smtp answer with format string as arg */
void _smtpd_string_reply(int sock, const char *format, ...) {
	ssize_t len = 0;
	char *out = NULL;
	va_list ap;

	va_start(ap, format);
	
	if (vasprintf(&out,format,ap) <= 0) {
		TRACE(TRACE_ERR,"failed to write message");
		return;
	}

	if ((len = _writen(sock,out,strlen(out))) != strlen(out)) {
		TRACE(TRACE_WARNING, "unexpected size [%d], expected [%d] bytes",strlen(out),len);
	} 
	free(out);
	va_end(ap);
}

void _smtpd_code_reply(int sock, int code, SMFDict_T *codes) {
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

	if ((len = _writen(sock,out,strlen(out))) != strlen(out)) {
		TRACE(TRACE_WARNING, "unexpected size [%d], expected [%d] bytes",strlen(out),len);
	} 
	free(out);
}

/*=== BELOW IS NOT GLIB CLEAN ===*/

#if 0
#define RE_MAIL_FROM "^MAIL FROM:?\\W*(?:.*<)?([^>]*)(?:>)?(?:\\W*SIZE=(\\d+))?"

GPrivate* current_session_key = NULL; 

/* copy headers from message object to own GMimeHeaderList */
static void copy_header_func(const char *name, const char *value, gpointer data) {
#ifdef HAVE_GMIME24
	g_mime_header_list_append((GMimeHeaderList *)data,
			g_strdup(name),g_strdup(value));
#else
	g_mime_header_add((GMimeHeader *)data,
			g_strdup(name),g_strdup(value));
#endif
}

/* dot-stuffing */
void stuffing(char chain[]) {
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

/* error handler used when building module queue
 * return 1 if processing should continue, else 0
 */
static int handle_q_error(void *args) {
	SMFSettings_T *settings = smf_settings_get();
	SMFSession_T *session = (SMFSession_T *)args;
	switch (settings->module_fail) {
		case 1: return(1);
		case 2: smtpd_code_reply(session->sock_out,552);
				return(0);
		case 3: smtpd_code_reply(session->sock_out,451);
				return(0);
	}

	return(0);
}

/* handle processing errors when running queue 
 *
 * return codes:
 * -1 = Error in processing, spmfilter will send 4xx Error to MTA
 * 0 = All ok, the next plugin will be started.
 * 1 = Further processing will be stopped. Email is not going
 *     to be delivered to nexthop!
 * 2 = Further processing will be stopped, no other plugin will
 *     be startet. spmfilter sends a 250 code
 */
static int handle_q_processing_error(int retval, void *args) {
	SMFSettings_T *settings = smf_settings_get();
	SMFSession_T *session = (SMFSession_T *)args;

	if (retval == -1) {
		switch (settings->module_fail) {
			case 1: return(1);
			case 2: smtpd_code_reply(session->sock_out,552);
					return(0);
			case 3: smtpd_code_reply(session->sock_out,451);
					return(0);
		}
	} else if(retval == 1) {
		if (session->response_msg != NULL) {
			char *smtp_response;
			smtp_response = g_strdup_printf("250 %s\r\n",session->response_msg);
			smtpd_string_reply(session->sock_out,smtp_response);
			free(smtp_response);
		} else
			smtpd_string_reply(session->sock_out,CODE_250_ACCEPTED);
		return(1);
	} else if(retval == 2) {
		return(2);
	} else {
		if (session->response_msg != NULL) {
			char *smtp_response;
			smtp_response = g_strdup_printf("%d %s\r\n",retval,session->response_msg);
			smtpd_string_reply(session->sock_out,smtp_response);
			free(smtp_response);
		} else
			smtpd_code_reply(session->sock_out,retval);
		return(1);
	}

	/* if none of the above matched, halt processing, this is just
	 * for safety purposes
	 */
	TRACE(TRACE_DEBUG, "no conditional matched, will stop queue processing!");
	return(0);
}

/* handle nexthop delivery error */
static int handle_nexthop_error(void *args) {
	SMFSettings_T *settings = smf_settings_get();
	SMFSession_T *session = (SMFSession_T *)args;
	
	smtpd_string_reply(session->sock_out,g_strdup_printf(
		"%d %s\r\n",
		settings->nexthop_fail_code,
		settings->nexthop_fail_msg)
	);

	return(0);
}

int load_modules(SMFSession_T *session, SMFSettings_T *settings) {
	int ret;
	ProcessQueue_T *q;

	/* initialize the modules queue handler */
	q = smf_modules_pqueue_init(
		handle_q_error,
		handle_q_processing_error,
		handle_nexthop_error
	);

	if(q == NULL) {
		return(-1);
	}

	/* now tun the process queue */
	ret = smf_modules_process(q,session,settings);
	free(q);

	if(ret == -1) {
		TRACE(TRACE_DEBUG, "smtp engine failed to process modules!");
		return(-1);
	} else if (ret == 1) {
		return(0);
	}

	if (session->response_msg != NULL) {
		char *smtp_response;
		smtp_response = g_strdup_printf("250 %s\r\n",session->response_msg);
		smtpd_string_reply(session->sock_out,smtp_response);
		free(smtp_response);
	} else
		smtpd_string_reply(session->sock_out,CODE_250_ACCEPTED);
	return(0);
}

void process_data(SMFSession_T *session, SMFSettings_T *settings) {
	GIOChannel *in;
	GMimeStream *out;
	gchar *line;
	gsize length;
	FILE *fd;
	GMimeParser *parser;
	GMimeMessage *message;
	char *message_id;

	smf_core_gen_queue_file(&session->message_file);
	if (session->message_file == NULL) {
		TRACE(TRACE_ERR,"got no spool file path");
		smtpd_code_reply(session->sock_out, 552);
		return;
	}
		
	TRACE(TRACE_DEBUG,"using spool file: '%s'", session->message_file);
		
	smtpd_string_reply(session->sock_out,"354 End data with <CR><LF>.<CR><LF>\r\n");
	
	/* start receiving data */
	in = g_io_channel_unix_new(session->sock_in);
	g_io_channel_set_encoding(in, NULL, NULL);
	g_io_channel_set_close_on_unref(in,FALSE);

	if ((fd = fopen(session->message_file,"wb+")) == NULL) {
		TRACE(TRACE_ERR,"failed to create spool file %s: [%d - %s]\n",
				session->message_file,errno, strerror(errno));
		smtpd_code_reply(session->sock_out,552);

		return;
	}
	
	out = g_mime_stream_file_new(fd);

	while (g_io_channel_read_line(in, &line, &length, NULL, NULL) == G_IO_STATUS_NORMAL) {
		if ((g_ascii_strcasecmp(line, ".\r\n")==0)||(g_ascii_strcasecmp(line, ".\n")==0)) break;
		if (g_ascii_strncasecmp(line,".",1)==0) stuffing(line);
		
		if (g_mime_stream_write(out,line,length) == -1) {
			smtpd_string_reply(session->sock_out,CODE_451);
			g_object_unref(out);
			g_io_channel_unref(in);
			g_free(line);
			if (g_remove(session->message_file) != 0)
				TRACE(TRACE_ERR,"failed to remove queue file");
			return;
		}
		session->msgbodysize+=length;
		g_free(line);
	}

	g_io_channel_unref(in);
	/* extract message headers */
	g_mime_stream_flush(out);
	g_mime_stream_seek(out,0,0);

	parser = g_mime_parser_new_with_stream(out);
	message = g_mime_parser_construct_message(parser);
#ifdef HAVE_GMIME24
	session->envelope->message->headers = (void *)g_mime_header_list_new();
	g_mime_header_list_foreach(GMIME_OBJECT(message)->headers, copy_header_func, session->envelope->message->headers);
#else
	session->envelope=>headers = (void *)g_mime_header_new();
	g_mime_header_foreach(GMIME_OBJECT(message)->headers, copy_header_func, session->envelope->message->headers);
#endif
	smf_message_extract_addresses(&session->envelope);
	g_object_unref(parser);
	g_object_unref(message);
	g_object_unref(out);

	if (session->envelope->message->message_from != NULL) {
		if (session->envelope->message->message_from->addr == NULL) {
			// TODO: refactoring for new datatypes
//			smf_session_header_append(session,"From",g_strdup(session->envelope->sender->addr));
			TRACE(TRACE_DEBUG,"adding [from] header to message");
		}
	}

	if (session->envelope->message->message_to_num == 0) {
		// TODO: refactoring for new datatypes
//		smf_session_header_append(session,"To",g_strdup("undisclosed-recipients:;"));
		TRACE(TRACE_DEBUG,"adding [to] header to message");
	}

#if 0
	message_id = (char *)smf_session_header_get(session,"message-id");

	if (message_id == NULL) {
		message_id = smf_message_generate_message_id();
		TRACE(TRACE_DEBUG,"no message id found, adding [%s]",message_id);
		smf_session_header_append(session,"Message-ID",message_id);
		// FIXME: if mid is added, the id is not flushed to message, check smf_modules_flush_dirty()
		if (smf_modules_flush_dirty(session) != 0) {
			TRACE(TRACE_ERR,"message flush failed");
			smtpd_code_reply(session->sock_out,552);
			return;
		}
	}
#endif

	TRACE(TRACE_DEBUG,"data complete, message size: %d", (u_int32_t)session->msgbodysize);

	load_modules(session,settings);
	
	if (g_remove(session->message_file) != 0)
 		TRACE(TRACE_ERR,"failed to remove queue file");
	TRACE(TRACE_DEBUG,"removing spool file %s",session->message_file);
	return;
}

#endif

int load(SMFSettings_T *settings,int sock) {
	char *hostname = NULL;
	int br;
	void *rl = NULL;
	char req[MAXLINE];
	char *req_value = NULL;
	char *t = NULL;
	int state=ST_INIT;
	SMFSession_T *session = smf_session_new();
	clock_t start_process, stop_process;
	



	//GIOChannel *in;
	//gchar *line;
	//char *requested_size = NULL;
	//const char *mail_from_addr = NULL;
	//GRegex *re = NULL;
	//GMatchInfo *match_info = NULL;

	/* start clock, to see how long
	 * the processing time takes */
	start_process = clock();

	session->sock = sock;

	hostname = (char *)malloc(MAXHOSTNAMELEN);
    gethostname(hostname,MAXHOSTNAMELEN);
	_smtpd_string_reply(session->sock,"220 %s spmfilter\r\n",hostname);

	for (;;) {
		if ((br = _readline(session->sock,req,MAXLINE,&rl)) < 1) 
			break; /* EOF or error */

		TRACE(TRACE_DEBUG,"client smtp dialog: [%s]",req);

		if (strncasecmp(req,"quit",4)==0) {
			TRACE(TRACE_DEBUG,"SMTP: 'quit' received"); 
			_smtpd_code_reply(session->sock,221,settings->smtp_codes);
			state = ST_QUIT;
			break;
		} else if( (strncasecmp(req, "helo", 4)==0) || (strncasecmp(req, "ehlo", 4)==0)) {
			/* An EHLO command MAY be issued by a client later in the session.
			 * If it is issued after the session begins, the SMTP server MUST
			 * clear all buffers and reset the state exactly as if a RSET
			 * command had been issued.
			 */
			if (state != ST_INIT) {
				smf_session_free(session);
				/* reinit session */
				session = smf_session_new();
				session->sock = sock;
				TRACE(TRACE_DEBUG,"session reset, helo/ehlo recieved not in init state");
			}
			TRACE(TRACE_DEBUG,"SMTP: 'helo/ehlo' received");
			req_value = _get_req_value(req,4);
			smf_session_set_helo(session,req_value);
			if (strncmp(req_value,req,strlen(req_value)) != 0) {
				if (strcmp(session->helo,"") == 0)  {
					_smtpd_string_reply(session->sock,"501 Syntax: HELO hostname\r\n");
				} else {
					TRACE(TRACE_DEBUG,"session->helo: [%s]",smf_session_get_helo(session));

					if (strncasecmp(req, "ehlo", 4)==0) {
						_smtpd_string_reply(session->sock,
							"250-%s\r\n250-XFORWARD ADDR\r\n250 SIZE\r\n",hostname);
					} else {
						_smtpd_string_reply(session->sock,"250 %s\r\n",hostname);
					}
					state = ST_HELO;
				}
			} else {
				_smtpd_string_reply(session->sock,"501 Syntax: HELO hostname\r\n");
			}
			free(req_value);
		} else if (strncasecmp(req,"xforward",8)==0) {
			TRACE(TRACE_DEBUG,"SMTP: 'xforward' received");
			t = strcasestr(req,"ADDR=");
			if (t != NULL) {
				t = strchr(t,'=');
				smf_core_strstrip(++t);
				smf_session_set_xforward_addr(session,t);
				TRACE(TRACE_DEBUG,"session->xforward_addr: [%s]",smf_session_get_xforward_addr(session));
				_smtpd_code_reply(session->sock,250,settings->smtp_codes);
				state = ST_XFWD;
			} else {
				_smtpd_string_reply(session->sock,"501 Syntax: XFORWARD attribute=value...\r\n");
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
			TRACE(TRACE_DEBUG,"SMTP: 'mail from' received");
			if (state == ST_MAIL) {
				/* we already got the mail command */
				_smtpd_string_reply(session->sock,"503 Error: nested MAIL command\r\n");
			} else {
				req_value = _get_req_value(req,10);
				printf("REQVALUE [%s]\n",req_value);
				smf_envelope_set_sender(session->envelope,req_value);
				TRACE(TRACE_DEBUG,"session->envelope->sender: [%s]",session->envelope->sender->email);
				free(req_value);
				_smtpd_code_reply(session->sock,250,settings->smtp_codes);
				state = ST_MAIL;
			}
		}
	}
	free(rl);
#if 0
	session->envelope->num_rcpts = 0;
	in = g_io_channel_unix_new(session->sock_in);
	g_io_channel_set_encoding(in, NULL, NULL);
	g_io_channel_set_close_on_unref(in,FALSE);
	while (g_io_channel_read_line(in, &line, NULL, NULL, NULL) == G_IO_STATUS_NORMAL) {
		g_strstrip(line);
		TRACE(TRACE_DEBUG,"client smtp dialog: '%s'",line);
		

		} else if (g_ascii_strncasecmp(line, "mail from:", 10)==0) {
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
			TRACE(TRACE_DEBUG,"SMTP: 'mail from' received");
			if (state == ST_MAIL) {
				/* we already got the mail command */
				smtpd_string_reply(session->sock_out,"503 Error: nested MAIL command\r\n");
			} else {
				session->envelope->sender = g_slice_new(SMFEmailAddress_T);
				re = g_regex_new(RE_MAIL_FROM, G_REGEX_CASELESS, 0, NULL);
				g_regex_match(re, line, 0, &match_info);
				if(g_match_info_matches(match_info)) {
					mail_from_addr = g_match_info_fetch(match_info, 1);
					if (mail_from_addr != NULL) {
						session->envelope->sender->addr = g_strdup(mail_from_addr);
						free((char *)mail_from_addr);
					}
					if (settings->max_size != 0 )
						requested_size = g_match_info_fetch(match_info, 2);
				} else {
					smtpd_string_reply(session->sock_out,CODE_552);
					g_slice_free(SMFEmailAddress_T,session->envelope->sender);
					session->envelope->sender = NULL;
				}
				g_match_info_free(match_info);
				g_regex_unref(re);

				if (settings->max_size != 0) {
					if (requested_size != NULL) {
						unsigned long l;
						l = (unsigned long) strtol(requested_size,NULL,10);
						if (l > settings->max_size) {
							smtpd_string_reply(session->sock_out,"552 Message size limit exceeded\r\n");
							g_slice_free(SMFEmailAddress_T,session->envelope->sender);
							session->envelope->sender = NULL;
							continue;
						}
						free(requested_size);
					}
				}

				session->envelope->sender->lr = NULL;
				if (session->envelope->sender->addr != NULL){
					TRACE(TRACE_DEBUG,"session->sender: %s",session->envelope->sender->addr);
					if (g_ascii_strcasecmp(session->envelope->sender->addr,"") == 0) {
						/* check for emtpy string */
						smtpd_string_reply(session->sock_out,"501 Syntax: MAIL FROM:<address>\r\n");
						g_slice_free(SMFEmailAddress_T,session->envelope->sender);
						session->envelope->sender = NULL;
					} else {
						if (settings->backend != NULL) {
								smf_lookup_check_user(session->envelope->sender);
								TRACE(TRACE_DEBUG,"[%s] is local [%d]", session->envelope->sender->addr,session->envelope->sender->is_local);
						} else 
							session->envelope->sender->lr = NULL;

						smtpd_code_reply(session->sock_out,250);
						state = ST_MAIL;
					}
				} else {
					smtpd_string_reply(session->sock_out,"501 Syntax: MAIL FROM:<address>\r\n");
					g_slice_free(SMFEmailAddress_T,session->envelope->sender);
					session->envelope->sender = NULL;
				}
			}
		} else if (g_ascii_strncasecmp(line, "rcpt to:", 8)==0) {
			if ((state != ST_MAIL) && (state != ST_RCPT)) {
				/* someone wants to break smtp rules... */
				smtpd_string_reply(session->sock_out,"503 Error: need MAIL command\r\n");
			} else {
				TRACE(TRACE_DEBUG,"SMTP: 'rcpt to' received");

				/* reallocate memory to make room for additional recipients */
				session->envelope->rcpt = g_realloc(
					session->envelope->rcpt,
					sizeof(SMFEmailAddress_T) * (session->envelope->num_rcpts + 1)
				);

				/* allocate resources for the individual recipient */
				session->envelope->rcpt[session->envelope->num_rcpts] = g_slice_new(SMFEmailAddress_T);
				session->envelope->rcpt[session->envelope->num_rcpts]->addr = smf_core_get_substring("^RCPT TO:?\\W*(?:.*<)?([^>]*)(?:>)?", line, 1);
				session->envelope->rcpt[session->envelope->num_rcpts]->lr = NULL;
				if (session->envelope->rcpt[session->envelope->num_rcpts] != NULL) {
					if (strcmp(session->envelope->rcpt[session->envelope->num_rcpts]->addr,"") == 0) {
						/* empty rcpt to? */
						smtpd_string_reply(session->sock_out,"501 Syntax: RCPT TO:<address>\r\n");
						g_slice_free(SMFEmailAddress_T,session->envelope->rcpt[session->envelope->num_rcpts]);
					} else {
						TRACE(TRACE_DEBUG,"session->rcpt[%d]: %s",session->envelope->num_rcpts, session->envelope->rcpt[session->envelope->num_rcpts]->addr);
						if (settings->backend != NULL) {
							smf_lookup_check_user(session->envelope->rcpt[session->envelope->num_rcpts]);
							TRACE(TRACE_DEBUG,"[%s] is local [%d]", 
									session->envelope->rcpt[session->envelope->num_rcpts]->addr,
									session->envelope->rcpt[session->envelope->num_rcpts]->is_local);
						} else
							session->envelope->rcpt[session->envelope->num_rcpts]->lr = NULL;
						smtpd_code_reply(session->sock_out,250);
						session->envelope->num_rcpts++;
						state = ST_RCPT;
					}
				} else {
					smtpd_string_reply(session->sock_out,"501 Syntax: RCPT TO:<address>\r\n");
					g_slice_free(SMFEmailAddress_T,session->envelope->rcpt[session->envelope->num_rcpts]);
				}
			}
		} else if (g_ascii_strncasecmp(line,"data", 4)==0) {
			if ((state != ST_RCPT) && (state != ST_MAIL)) {
				/* someone wants to break smtp rules... */
				smtpd_string_reply(session->sock_out,"503 Error: need RCPT command\r\n");
			} else if ((state != ST_RCPT) && (state == ST_MAIL)) {
				/* we got the mail command but no rcpt to */
				smtpd_string_reply(session->sock_out,"554 Error: no valid recipients\r\n");
			} else {
				state = ST_DATA;
				TRACE(TRACE_DEBUG,"SMTP: 'data' received");
				process_data(session,settings);
			}
		} else if (g_ascii_strncasecmp(line,"rset", 4)==0) {
			TRACE(TRACE_DEBUG,"SMTP: 'rset' received");
			smf_session_free(session);
			/* reinit session */
			session = smf_session_new();
			session->sock_in = sock_in;
			session->sock_out = sock_out;
			smtpd_code_reply(session->sock_out,250);
			state = ST_INIT;
		} else if (g_ascii_strncasecmp(line, "noop", 4)==0) {
			TRACE(TRACE_DEBUG,"SMTP: 'noop' received");
			smtpd_code_reply(session->sock_out,250);
		} else if (g_ascii_strcasecmp(line,"")!=0){
			TRACE(TRACE_DEBUG,"SMTP: wtf?!");
			smtpd_code_reply(session->sock_out,502);
		} else {
			TRACE(TRACE_DEBUG,"SMTP: got empty line");
			smtpd_string_reply(session->sock_out,"500 Error: bad syntax\r\n");
		}
		g_free(line);
	} 
	g_io_channel_unref(in);
#endif 

	free(hostname);
	smf_session_free(session);

	/* processing is done, we can
	 * stop our clock */
	stop_process = clock();
	TRACE(TRACE_DEBUG,"processing time: %0.5f sec.", (float)(stop_process-start_process)/CLOCKS_PER_SEC);

	return 0;
}
