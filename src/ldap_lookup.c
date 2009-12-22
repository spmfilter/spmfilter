#ifdef HAVE_LDAP
#include <glib.h>
#include <ldap.h>
#include <stdarg.h>

#include "spmfilter.h"

#define THIS_MODULE "ldap_lookup"

static GPrivate *ldap_conn_key = NULL;

int ldap_connect(void) {
	LDAP *ld = NULL;
	int ret, err;
	int version;
	char *uri;

	if (! g_thread_supported()) g_thread_init(NULL);
	if (! ldap_conn_key) ldap_conn_key = g_private_new(g_free);

	TRACE(TRACE_LOOKUP, "connecting to ldap server on [%s]",settings->ldap_host);
	if (settings->ldap_uri) {
		if ((ret = ldap_initialize(&ld, settings->ldap_uri) != LDAP_SUCCESS)) 
			TRACE(TRACE_ERR, "ldap_initialize() returned [%d]", ret);
	} else {
		uri = g_strdup_printf("ldap://%s:%d",settings->ldap_host,settings->ldap_port);
		if ((ret = ldap_initialize(&ld, uri)) != LDAP_SUCCESS) 
			TRACE(TRACE_ERR, "ldap_initialize() returned [%d]", ret);

		g_free(uri);
	}
	version = LDAP_VERSION3;
	ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &version);

	if (settings->ldap_referrals) {
		ldap_set_option(ld, LDAP_OPT_REFERRALS, (void *)LDAP_OPT_ON);
		TRACE(TRACE_LOOKUP, "set ldap referrals to on");
	} else {
		ldap_set_option(ld, LDAP_OPT_REFERRALS, (void *)LDAP_OPT_OFF);
		TRACE(TRACE_LOOKUP, "set ldap referrals to off");
	}
	
	TRACE(TRACE_LOOKUP, "binding to ldap server as [%s] / [xxxxxxxx]",  settings->ldap_binddn);
	
	if ((err = ldap_bind_s(ld, settings->ldap_binddn, settings->ldap_bindpw, LDAP_AUTH_SIMPLE))) {
		TRACE(TRACE_ERR, "ldap_bind_s failed: %s",  ldap_err2string(err));
		return -1;
	}
	TRACE(TRACE_LOOKUP, "successfully bound to ldap server" );
	
	g_private_set(ldap_conn_key, ld);
	
	return 0;
}

LDAP * ldap_con_get(void) {
	LDAP * c = g_private_get(ldap_conn_key);
	if (!c) {
		ldap_connect();
		c = g_private_get(ldap_conn_key);
	}
	return c;
}

int ldap_disconnect(void) {
	LDAP *ld = ldap_con_get();
	if (ld != NULL) {
		ldap_unbind(ld);
		g_private_set(ldap_conn_key, NULL);
		TRACE(TRACE_LOOKUP, "unbind ldap server");
	}
}

LDAPMessage *ldap_query(const char *q, ...) {
	va_list ap, cp;
	char *query;
	LDAPMessage *msg = NULL;
	LDAP *ld = ldap_con_get();
	
	va_start(ap, q);
	va_copy(cp, ap);
	query = g_strdup_vprintf(q, cp);
	va_end(cp);
	g_strstrip(query);
	
	TRACE(TRACE_LOOKUP,"[%p] [%s]",ld,query);
	
	if (ldap_search_s(ld,settings->ldap_base,LDAP_SCOPE_SUBTREE,query,NULL,0,&msg) != LDAP_SUCCESS) 
		TRACE(TRACE_ERR,"[%p] query [%s] failed",ld, query);
	
	if(ldap_count_entries(ld,msg) <= 0)
		TRACE(TRACE_LOOKUP,"[%p] nothing found",ld);
	
	return msg;
}

int ldap_user_exists(char *addr) {
	char *query;
	LDAP *ld = ldap_con_get();

	TRACE(TRACE_LOOKUP,"[%p] [%s]",ld,query);
	
}

#endif
