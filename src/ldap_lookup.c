#ifdef HAVE_LDAP
#include <glib.h>
#include <ldap.h>

#include "spmfilter.h"

#define THIS_MODULE "ldap_lookup"

static GPrivate *ldap_conn_key = NULL;

int ldap_connect(void) {
	LDAP *ld = NULL;
	int ret, err;
	int version;
	char *uri;
	SETTINGS *settings = g_private_get(settings_key);

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

LDAP * ldap_get_con(void) {
	LDAP * c = g_private_get(ldap_conn_key);
	if (!c) {
		ldap_connect();
		c = g_private_get(ldap_conn_key);
	}
	return c;
}

int ldap_disconnect(void) {
	LDAP *ld = ldap_get_con();
	if (ld != NULL) {
		ldap_unbind(ld);
		g_private_set(ldap_conn_key, NULL);
		TRACE(TRACE_LOOKUP, "unbind ldap server");
	}
}

int ldap_query(SETTINGS *settings) {
	ldap_get_con();
	ldap_disconnect();
	return 0;
}

#endif
