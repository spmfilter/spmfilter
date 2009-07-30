#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gmodule.h>
#include <syslog.h>
#include <unistd.h>
#include <gmime/gmime.h>
#include <fcntl.h>

#include "spmfilter.h"

typedef int (*LoadMod) (SETTINGS *settings, MAILCONN *mconn);

int load_modules(SETTINGS *settings, MAILCONN *mconn) {
	GModule *module;
	LoadMod load_module;
	int i, ret;
	MESSAGE *msg = NULL;
	
	for(i = 0; settings->modules[i] != NULL; i++) {
		gchar *path;
		if (settings->debug)
			syslog(LOG_DEBUG,"loading module %s",settings->modules[i]);
		
		if (g_str_has_prefix(settings->modules[i],"lib")) {
			path = g_module_build_path(LIB_DIR,settings->modules[i]);
		} else {
			path = g_module_build_path(LIB_DIR,g_strdup_printf("lib%s",settings->modules[i]));
		}
		module = g_module_open(path, G_MODULE_BIND_LAZY);
		if (!module) {
			if (settings->debug)
				syslog(LOG_DEBUG,"%s\n", g_module_error ());
			switch (settings->module_fail) {
				case 1: continue;
				default: return -1;
			}
			return;
		}

		if (!g_module_symbol (module, "load", (gpointer *)&load_module)) {
			if (settings->debug)
				syslog(LOG_DEBUG,"%s\n", g_module_error ());
			switch (settings->module_fail) {
				case 1: continue;
				default: return -1;
			}
			return;
		}

		ret = load_module(settings,mconn); 
		if (ret == -1) {
			g_module_close (module);
			switch (settings->module_fail) {
				case 1: continue;
				default: return -1;
			}
			return;
		} else if (ret == 1) {
			g_module_close (module);
			break;
		}

		if (!g_module_close (module)) {
			if (settings->debug)
				syslog(LOG_DEBUG,"%s\n", g_module_error ());
		}
		
		g_free(path);
	}
	
	if (settings->nexthop != NULL ) {
		msg = g_slice_new(MESSAGE);
		msg->from = g_strdup(mconn->from);
		msg->rcpt = mconn->rcpt;
		msg->message_file = g_strdup(mconn->queue_file);
		msg->nexthop = g_strup(settings->nexthop);
		if (smtp_delivery(settings, msg) != 0) {
			syslog(LOG_ERR,"delivery to %s failed!",settings->nexthop);
			return -1;
		}
		g_slice_free(MESSAGE,msg);
	}
	return 0;
}


int load(SETTINGS *settings,MAILCONN *mconn) {
	char *tempname;
	GIOChannel *in, *out;
	GMimeStream *gmin;
	GMimeMessage *message;
	GMimeParser *parser;
	gchar *line;
	gsize length;
	GError *error = NULL;
	HL *d;
	InternetAddressList *ia;
	InternetAddress *addr;
	
	/* create spooling file */
	tempname = g_strdup_printf("%s/spmfilter.XXXXXX",QUEUE_DIR);
	if (g_mkstemp(tempname) == -1) {
		g_free(tempname);
		syslog(LOG_ERR,"Can't create spooling file");
		return -1;
	}
	mconn->queue_file = g_strdup(tempname);
	
	if (settings->debug)
		syslog(LOG_DEBUG,"using spool file: '%s'", mconn->queue_file);
		
	/* start receiving data */
	in = g_io_channel_unix_new(STDIN_FILENO);
	if ((out = g_io_channel_new_file(mconn->queue_file,"w", &error)) == NULL) {
		syslog(LOG_ERR,"%s",error->message);
		return -1;
	}
	g_io_channel_set_encoding(in, NULL, NULL);
	g_io_channel_set_encoding(out, NULL, NULL);
	
	/* initialize GMime */
	g_mime_init (0);
	gmin = g_mime_stream_mem_new();

	while (g_io_channel_read_line(in, &line, &length, NULL, NULL) == G_IO_STATUS_NORMAL) {
		if (g_io_channel_write_chars(out, line, -1, &length, &error) != G_IO_STATUS_NORMAL) {
			syslog(LOG_ERR,"%s\r\n",error->message);
			g_io_channel_shutdown(out,TRUE,NULL);
			g_io_channel_unref(in);
			g_io_channel_unref(out);
			g_free(line);
			remove(mconn->queue_file);
			return -1;
		}
		mconn->msgbodysize+=strlen(line);
		g_mime_stream_write_string(gmin,line);
		g_free(line);
	}
	
	g_io_channel_shutdown(out,TRUE,NULL);
	g_io_channel_unref(in);
	g_io_channel_unref(out);

	if (settings->debug)
		syslog(LOG_DEBUG,"data complete, message size: %d", (u_int32_t)mconn->msgbodysize);
	
	/* parse email data and fill mconn struct*/
	g_mime_stream_seek(gmin,0,0);
	parser = g_mime_parser_new_with_stream (gmin);
	g_object_unref(gmin);
	message = g_mime_parser_construct_message (parser);
	mconn->from = get_substring(EMAIL_EXTRACT,g_mime_message_get_sender(message),1);
	if (settings->debug)
		syslog(LOG_DEBUG,"mconn->from: %s",mconn->from);
	
	ia = g_mime_message_get_all_recipients(message);
	while(ia) {
		addr = internet_address_list_get_address(ia);
		mconn->rcpt = g_slist_append(mconn->rcpt,get_substring(EMAIL_EXTRACT, internet_address_get_addr(addr), 1));
		if (settings->debug)
			syslog(LOG_DEBUG,"mconn->rcpt[%d]: %s",
				g_slist_length(mconn->rcpt)-1,
				g_slist_nth_data(mconn->rcpt,g_slist_length(mconn->rcpt)-1));
		ia = internet_address_list_next(ia);
	}
	
	/* check header */
	d = g_slice_new(HL);
	d->settings = settings;
	d->message = message;
	g_hash_table_foreach(settings->header_checks,header_check,d);
	g_slice_free(HL,d);
	
	if (load_modules(settings,mconn) != 0) {
		remove(mconn->queue_file);
		if(settings->debug)
			syslog(LOG_DEBUG,"removing spool file %s",mconn->queue_file);
		return -1;
	} else {
		remove(mconn->queue_file);
		if(settings->debug)
			syslog(LOG_DEBUG,"removing spool file %s",mconn->queue_file);
		return 0;
	}
}
