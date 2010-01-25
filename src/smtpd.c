#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gmodule.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

#include "spmfilter.h"
#include "smtpd.h"

#define THIS_MODULE "smtpd"

MailConn_T *mconn;

/* SMTP States */
#define ST_INIT 0
#define ST_HELO 1
#define ST_XFWD 2
#define ST_MAIL 3
#define ST_RCPT 4
#define ST_DATA 5
#define ST_QUIT 6

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

/* smtp answer with format string as arg */
void smtp_string_reply(const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	vfprintf(stdout,format,ap);
	va_end(ap);
	fflush(stdout);
}

/** Write SMTP answer to stdout
 *
 * \param wanted smtp code
 */
void smtp_code_reply(int code) {
	char *code_msg;
	Settings_T *settings = get_settings();

	/* we don't need to free code_msg, will be
	 * freed by smtp_code_free() */
	code_msg = smtp_code_get(settings->smtp_codes,code);
	if (code_msg!=NULL) {
		fprintf(stdout,"%d %s\r\n",code,code_msg);  
	} else {
		switch(code) {
			case 221: 
				fprintf(stdout,CODE_221);
				break;
			case 250: 
				fprintf(stdout,CODE_250);
				break;
			case 451: 
				fprintf(stdout,CODE_451);
				break;
			case 502:
				fprintf(stdout,CODE_502);
				break;
			case 552:
				fprintf(stdout,CODE_552);
				break;
		}
	}
	fflush(stdout);
}

void load_modules(void) {
	GModule *module;
	LoadMod load_module;
	int i, ret;
	Message_T *msg = NULL;
	Settings_T *settings = get_settings();

	for(i = 0; settings->modules[i] != NULL; i++) {
		gchar *path;	
		TRACE(TRACE_DEBUG,"loading module %s",settings->modules[i]);
		
		if (g_str_has_prefix(settings->modules[i],"lib")) {
#ifdef __APPLE__
			path = g_module_build_path(LIB_DIR,g_strdup_printf("%s.dylib",g_strstrip(settings->modules[i])));
#else
			path = g_module_build_path(LIB_DIR,g_strstrip(settings->modules[i]));
#endif
		} else {
#ifdef __APPLE__
			path = g_module_build_path(LIB_DIR,g_strdup_printf("lib%s.dylib",g_strstrip(settings->modules[i])));
#else
			path = g_module_build_path(LIB_DIR,g_strdup_printf("lib%s",g_strstrip(settings->modules[i])));
#endif
		}
		module = g_module_open(path, G_MODULE_BIND_LAZY);
		if (!module) {
			g_free(path);
			TRACE(TRACE_ERR,"%s", g_module_error());
			switch (settings->module_fail) {
				case 1: continue;
				case 2: smtp_code_reply(552);
						return;
				case 3: smtp_code_reply(451);
						return;
			}
			return;
		}

		if (!g_module_symbol(module, "load", (gpointer *)&load_module)) {
			g_free(path);
			TRACE(TRACE_ERR,"%s", g_module_error());
			switch (settings->module_fail) {
				case 1: continue;
				case 2: smtp_code_reply(552);
						return;
				case 3: smtp_code_reply(451);
						return;
			}
			return;
		}

		/* module return codes:
		 * -1 = Error in processing, spmfilter will send 4xx Error to MTA
		 * 0 = All ok, the next plugin will be started.
		 * 1 = Further processing will be stopped. Email is not going
		 *     to be delivered to nexthop!
		 * 2 = Further processing will be stopped, no other plugin will 
		 *     be startet. spmfilter sends a 250 code
		 */
		ret = load_module(mconn);
		TRACE(TRACE_DEBUG,"module return code [%d]", ret);
		if (ret == -1) {
			if (!g_module_close(module))
				TRACE(TRACE_ERR,"%s", g_module_error());
			g_free(path);
			switch (settings->module_fail) {
				case 1: continue;
				case 2: smtp_code_reply(552);
						return;
				case 3: smtp_code_reply(451);
						return;
			}
			return;
		} else if (ret == 1) {
			if (!g_module_close(module))
				TRACE(TRACE_ERR,"%s", g_module_error());
			g_free(path);
			smtp_string_reply(CODE_250_ACCEPTED);
			return;
		} else if (ret == 2) {
			if (!g_module_close(module))
				TRACE(TRACE_ERR,"%s", g_module_error());
			g_free(path);
			smtp_string_reply(CODE_250_ACCEPTED);
			break;
		} else if (ret >= 400) {
			if (!g_module_close(module))
				TRACE(TRACE_ERR,"%s", g_module_error());
			g_free(path);
				
			smtp_code_reply(ret);
			return;
		}

		if (!g_module_close(module))
			TRACE(TRACE_ERR,"%s", g_module_error());
		
		g_free(path);
	}

	if (settings->nexthop != NULL ) {
		msg = g_slice_new(Message_T);
		msg->from = g_strdup(mconn->from->addr);
		msg->rcpts = g_malloc(sizeof(msg->rcpts[mconn->num_rcpts]));
		for (i = 0; i < mconn->num_rcpts; i++) {
			msg->rcpts[i] = g_strdup(mconn->rcpts[i]->addr);
		}

		msg->num_rcpts = mconn->num_rcpts;
		msg->message_file = g_strdup(mconn->queue_file);
		msg->nexthop = g_strup(settings->nexthop);
		if (smtp_delivery(msg) != 0) {
			TRACE(TRACE_ERR,"delivery to %s failed!",settings->nexthop);
			smtp_string_reply(g_strdup_printf("%d %s\r\n",settings->nexthop_fail_code,settings->nexthop_fail_msg));
			return;
		}
		for (i = 0; i < mconn->num_rcpts; i++) {
			g_free(msg->rcpts[i]);
		}
		g_free(msg->from);
		g_free(msg->message_file);
		g_free(msg->nexthop);
		g_slice_free(Message_T,msg);
	}
	
	smtp_string_reply(CODE_250_ACCEPTED);
	return;
}

void process_data(void) {
	GIOChannel *in, *out;
	gchar *line;
	gsize length;
	GError *error = NULL;

	gen_queue_file(&mconn->queue_file);
	if (mconn->queue_file == NULL) {
		TRACE(TRACE_ERR,"failed to create spool file!");
		smtp_code_reply(552);
		return;
	}
		
	TRACE(TRACE_DEBUG,"using spool file: '%s'", mconn->queue_file);
		
	smtp_string_reply("354 End data with <CR><LF>.<CR><LF>\r\n");
	
	/* start receiving data */
	in = g_io_channel_unix_new(STDIN_FILENO);
	if ((out = g_io_channel_new_file(mconn->queue_file,"w", &error)) == NULL) {
		smtp_string_reply("552 %s\r\n",error->message);
		g_error_free(error);
		return;
	}
	g_io_channel_set_encoding(in, NULL, NULL);
	g_io_channel_set_encoding(out, NULL, NULL);

	while (g_io_channel_read_line(in, &line, &length, NULL, NULL) == G_IO_STATUS_NORMAL) {
		if ((g_ascii_strcasecmp(line, ".\r\n")==0)||(g_ascii_strcasecmp(line, ".\n")==0)) break;
		if (g_ascii_strncasecmp(line,".",1)==0) stuffing(line);
		
		if (g_io_channel_write_chars(out, line, -1, &length, &error) != G_IO_STATUS_NORMAL) {
			smtp_string_reply("452 %s\r\n",error->message);
			g_io_channel_unref(out);
			g_io_channel_shutdown(out,TRUE,NULL);
			g_io_channel_unref(in);
			g_free(line);
			remove(mconn->queue_file);
			g_error_free(error);
			return;
		}
		mconn->msgbodysize+=strlen(line);
		g_free(line);
	}
	g_io_channel_shutdown(out,TRUE,NULL);
	g_io_channel_unref(out);
	g_io_channel_unref(in);


	TRACE(TRACE_DEBUG,"data complete, message size: %d", (u_int32_t)mconn->msgbodysize);

	load_modules();
	
	remove(mconn->queue_file);
	TRACE(TRACE_DEBUG,"removing spool file %s",mconn->queue_file);
	return;
}

int load(void) {
	char hostname[256];
	GIOChannel *in;
	char *line;
	int state=ST_INIT;
	Settings_T *settings = get_settings();
	
	mconn_new();

	gethostname(hostname,256);

	smtp_string_reply("220 %s spmfilter\r\n",hostname);
	mconn->num_rcpts = 0;
	in = g_io_channel_unix_new(STDIN_FILENO);
	g_io_channel_set_encoding(in, NULL, NULL);
	while (g_io_channel_read_line(in, &line, NULL, NULL, NULL) == G_IO_STATUS_NORMAL) {
		g_strstrip(line);
		TRACE(TRACE_DEBUG,"client smtp dialog: '%s'",line);
		
		if (g_ascii_strncasecmp(line,"quit",4)==0) {
			TRACE(TRACE_DEBUG,"SMTP: 'quit' received"); 
			smtp_code_reply(221);
			state = ST_QUIT;
			break;
		} else if (g_ascii_strncasecmp(line, "helo", 4)==0) {
			/* An EHLO command MAY be issued by a client later in the session.
			 * If it is issued after the session begins, the SMTP server MUST
			 * clear all buffers and reset the state exactly as if a RSET
			 * command had been issued.
			 */
			if (state != ST_INIT) {
				mconn_free();
				/* reinit mconn */
				mconn_new();
			}
			TRACE(TRACE_DEBUG,"SMTP: 'helo' received");
			mconn->helo = get_substring("^HELO\\s(.*)$",line, 1);
			TRACE(TRACE_DEBUG,"HELO: %s",mconn->helo);
			if (mconn->helo != NULL) {
				if (MATCH(mconn->helo,""))  {
					smtp_string_reply("501 Syntax: HELO hostname\r\n");
				} else {
					TRACE(TRACE_DEBUG,"mconn->helo: %s",mconn->helo);
					smtp_string_reply("250 %s\r\n",hostname);
					state = ST_HELO;
				}
			} else {
				smtp_string_reply("501 Syntax: HELO hostname\r\n");
			}
		} else if (g_ascii_strncasecmp(line, "ehlo", 4)==0) {
			/* Same here....clear all buffers, if ehlo command is
			 * received later...
			 */
			if (state != ST_INIT) {
				mconn_free();
				/* reinit mconn */
				mconn_new();
			}
			TRACE(TRACE_DEBUG,"SMTP: 'ehlo' received");
			mconn->helo = get_substring("^EHLO\\s(.*)$",line,1);
			if (mconn->helo != NULL) {
				if (MATCH(mconn->helo,"")) {
					smtp_string_reply("501 Syntax: EHLO hostname\r\n");
				} else {
					TRACE(TRACE_DEBUG,"mconn->helo: %s",mconn->helo);
					smtp_string_reply("250-%s\r\n250-XFORWARD NAME ADDR PROTO HELO SOURCE\r\n250 DSN\r\n",hostname);
					state = ST_HELO;
				}
			} else {
				smtp_string_reply("501 Syntax: HELO hostname\r\n");
			}
		} else if (g_ascii_strncasecmp(line,"xforward name",13)==0) {
			TRACE(TRACE_DEBUG,"SMTP: 'xforward name' received");
			mconn->xforward_addr = get_substring("^XFORWARD NAME=.* ADDR=(.*)$",line,1);
			TRACE(TRACE_DEBUG,"mconn->xforward_addr: %s",mconn->xforward_addr);
			smtp_code_reply(250);
			state = ST_XFWD;
		} else if (g_ascii_strncasecmp(line, "xforward proto", 13)==0) {
			smtp_code_reply(250);
			state = ST_XFWD;
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
				smtp_string_reply("503 Error: nested MAIL command\r\n");
			} else {
				mconn->from = g_slice_new(EmailAddress_T);
				mconn->from->addr = get_substring("^MAIL FROM:?\\W*(?:.*<)?([^>]*)(?:>)?", line, 1);
				if (mconn->from->addr != NULL){
					TRACE(TRACE_DEBUG,"mconn->from: %s",mconn->from->addr);
					if (MATCH(mconn->from->addr,"")) {
						/* check for emtpy string */
						smtp_string_reply("501 Syntax: MAIL FROM:<address>\r\n");
						g_slice_free(EmailAddress_T,mconn->from);
						mconn->from = NULL;
					} else {
						if (!MATCH(settings->backend,"undef")) {
								mconn->from->is_local = lookup_user(mconn->from->addr);
								TRACE(TRACE_DEBUG,"[%s] is local [%d]", mconn->from->addr,mconn->from->is_local);
						}
						smtp_code_reply(250);
						state = ST_MAIL;
					}
				} else {
					smtp_string_reply("501 Syntax: MAIL FROM:<address>\r\n");
					g_slice_free(EmailAddress_T,mconn->from);
					mconn->from = NULL;
				}
			}
		} else if (g_ascii_strncasecmp(line, "rcpt to:", 8)==0) {
			if ((state != ST_MAIL) && (state != ST_RCPT)) {
				/* someone wants to break smtp rules... */
				smtp_string_reply("503 Error: need MAIL command\r\n");
			} else {
				TRACE(TRACE_DEBUG,"SMTP: 'rcpt to' received");
				mconn->rcpts = g_realloc(mconn->rcpts,sizeof(mconn->rcpts[mconn->num_rcpts]));
				mconn->rcpts[mconn->num_rcpts] = g_slice_new(EmailAddress_T);
				mconn->rcpts[mconn->num_rcpts]->addr = get_substring("^RCPT TO:?\\W*(?:.*<)?([^>]*)(?:>)?", line, 1);
				if (mconn->rcpts[mconn->num_rcpts] != NULL) {
					if (MATCH(mconn->rcpts[mconn->num_rcpts]->addr,"")) {
						/* empty rcpt to? */
						smtp_string_reply("501 Syntax: RCPT TO:<address>\r\n");
						g_slice_free(EmailAddress_T,mconn->rcpts[mconn->num_rcpts]);
					} else {
						TRACE(TRACE_DEBUG,"mconn->rcpts[%d]: %s",mconn->num_rcpts, mconn->rcpts[mconn->num_rcpts]->addr);
						if (!MATCH(settings->backend,"undef")) {
							mconn->rcpts[mconn->num_rcpts]->is_local = lookup_user(mconn->rcpts[mconn->num_rcpts]->addr);
							TRACE(TRACE_DEBUG,"[%s] is local [%d]", mconn->rcpts[mconn->num_rcpts]->addr,mconn->rcpts[mconn->num_rcpts]->is_local);
						}
						smtp_code_reply(250);
						mconn->num_rcpts++;
						state = ST_RCPT;
					}
				} else {
					smtp_string_reply("501 Syntax: RCPT TO:<address>\r\n");
					g_slice_free(EmailAddress_T,mconn->rcpts[mconn->num_rcpts]);
				}
			}
		} else if (g_ascii_strncasecmp(line,"data", 4)==0) {
			if ((state != ST_RCPT) && (state != ST_MAIL)) {
				/* someone wants to break smtp rules... */
				smtp_string_reply("503 Error: need RCPT command\r\n");
			} else if ((state != ST_RCPT) && (state == ST_MAIL)) {
				/* we got the mail command but no rcpt to */
				smtp_string_reply("554 Error: no valid recipients\r\n");
			} else {
				state = ST_DATA;
				TRACE(TRACE_DEBUG,"SMTP: 'data' received");
				process_data();
			}
		} else if (g_ascii_strncasecmp(line,"rset", 4)==0) {
			TRACE(TRACE_DEBUG,"SMTP: 'rset' received");
			mconn_free();
			/* reinit mconn */
			mconn_new();
			smtp_code_reply(250);
			state = ST_INIT;
		} else if (g_ascii_strncasecmp(line, "noop", 4)==0) {
			TRACE(TRACE_DEBUG,"SMTP: 'noop' received");
			smtp_code_reply(250);
		} else if (g_ascii_strcasecmp(line,"")!=0){
			TRACE(TRACE_DEBUG,"SMTP: wtf?!");
			smtp_code_reply(502);
		} else {
			TRACE(TRACE_DEBUG,"SMTP: got empty line");
			smtp_string_reply("500 Error: bad syntax\r\n");
		}
		g_free(line);
	} 
	
	g_io_channel_shutdown(in,TRUE,NULL);
	g_io_channel_unref(in);

	mconn_free();
	
	return 0;
}

/** Initialize MailConn_T structure
 */

void mconn_new(void) {
	mconn = g_slice_new(MailConn_T);
	mconn->helo = NULL;
	mconn->from = NULL;
	mconn->queue_file = NULL;
	mconn->rcpts = NULL;
	mconn->xforward_addr = NULL;
}

/** Free MailConn_T structure
 */
void mconn_free(void) {
	int i;
	g_free(mconn->queue_file);
	g_free(mconn->helo);
	g_free(mconn->xforward_addr);

	if (mconn->from != NULL)
		g_free(mconn->from->addr);
	g_slice_free(EmailAddress_T,mconn->from);
	for (i = 0; i < mconn->num_rcpts; i++) {
		g_free(mconn->rcpts[i]->addr);
		g_slice_free(EmailAddress_T,mconn->rcpts[i]);
	}
	g_free(mconn->rcpts);
	g_slice_free(MailConn_T,mconn);

}

