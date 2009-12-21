#ifndef SPMFILTER_H
#define SPMFILTER_H

#include <glib.h>
#include <gmime/gmime.h>
#include <sys/time.h>

#ifdef HAVE_ZDB
#include <URL.h>
#include <ResultSet.h>
#include <PreparedStatement.h>
#include <Connection.h>
#include <ConnectionPool.h>
#include <SQLException.h>
#endif

#ifdef HAVE_LDAP
#include <ldap.h>
#endif

#define GLIB2_VERSION (GLIB_MAJOR_VERSION * 10000 \
	+ GLIB_MINOR_VERSION * 100 \
	+ GLIB_MICRO_VERSION)

#define EMAIL_EXTRACT "(?:.*<)?([^>]*)(?:>)?"
#define MATCH(x,y) strcasecmp((x),(y))==0
#define FIELDSIZE 1024
#define GETTIMEOFDAY(t) gettimeofday(t,(struct timezone *) 0)

typedef struct {
	int debug;
	char *config_file;
	char *queue_dir;
	char *engine;
	char *spool_dir;
	gchar **modules;
	int module_fail;
	char *nexthop;
	int nexthop_fail_code;
	char *nexthop_fail_msg;
	char *backend;
	/* hash table for smtp codes */
	GHashTable *smtp_codes;
	
#ifdef HAVE_ZDB
	char *sql_driver;
	char *sql_name;
	char *sql_host;
	int sql_port;
	char *sql_user;
	char *sql_pass;
	char *encoding;
	char *sql_user_query;
	char *sql_encoding;
	int sql_max_connections;
#endif

#ifdef HAVE_LDAP
	char *ldap_uri;
	char *ldap_host;
	int ldap_port;
	char *ldap_binddn;
	char *ldap_bindpw;
	char *ldap_base;
	gboolean ldap_referrals;
#endif
} SETTINGS;
extern SETTINGS *settings;

typedef struct {
	char *from;
	char **rcpts;
	int num_rcpts;
	char *message_file;
	char *auth_user;
	char *auth_pass;
	char *nexthop;
} MESSAGE;

typedef struct {
	char *addr;
	int is_local;
} EMLADDR;

typedef struct {
	/* hello we received */
	char *helo;
	
	/* recipients */
	EMLADDR **rcpts;
	int num_rcpts;
	
	/* sender */
	EMLADDR *from;
	
	/* size of message body */
	size_t msgbodysize;
	
	/* this is our spooling file */
	char *queue_file;
	
	/* xfoward */
	char *xforward_addr;
} MAILCONN;

/* Logging and debugging stuff */
typedef enum {
	TRACE_EMERG = 1,
	TRACE_ALERT = 2,
	TRACE_CRIT = 4,
	TRACE_ERR = 8,
	TRACE_WARNING = 16,
	TRACE_NOTICE = 32,
	TRACE_INFO = 64,
	TRACE_DEBUG = 128,
	TRACE_LOOKUP = 256 // Logs at Debug Level
} trace_t;


#define TRACE(level, fmt...) trace(level, THIS_MODULE, __func__, __LINE__, fmt)
void trace(trace_t level, const char * module, const char * function, int line, const char *formatstring, ...);


/* 
 * Generate a new queue file name
 */
int gen_queue_file(char **tempanme);

char* get_substring(const char *pattern, const char *haystack, int pos);

/*
 * Parse a message file on disk and return a GMimeMessage object
 *
 * msg_path: path to message file
 */
GMimeMessage *parse_message(char *msg_path);

/*
 * Write a message to disk
 *
 * msg_path: path for the new message file
 * message: GMimeMessage object
 */
int write_message(char *msg_path, GMimeMessage *message);

/* 
 * Gets the value of the requested header if it exists or NULL otherwise.
 *
 * msg_path: path to message or queue file
 * header_name: name of the wanted header
 */
const char *get_header(char *msg_path, char *header_name);

/*
 * Adds an arbitrary header to the message, returns 0 on success.
 * 
 * msg_path: path to message or queue file
 * header_name: name of the new header
 * header_value: value for the new header
 */
int add_header(char *msg_path, char *header_name, char *header_value);

/*
 * Sets an arbitrary header, returns 0 on success.
 * 
 * msg_path: path to message or queue file
 * header_name: name of the header
 * header_value: new value for the header
 */
int set_header(char *msg_path, char *header_name, char *header_value);

/*
 * Removed the specified header if it exists, returns 0 on success
 *
 * msg_path: path to message or queue_file
 * header_name: name of the header
 */
int remove_header(char *msg_path, char *header_name);

int smtp_delivery(MESSAGE *msg_data);

/*
 * Generates a unique maildir filename
 */
char *get_maildir_filename(void);

#ifdef HAVE_ZDB
/*
 * Query database, returns ResultSet_T
 *
 * q: format query
 * .... query args
 */
int sql_user_exists(char *addr); 
ResultSet_T sql_query(const char *q, ...);
#endif

#ifdef HAVE_LDAP
/*
 * Query LDAP directory, returns LDAPMessage
 *
 * q: format query
 * .... query args
 */
LDAPMessage *ldap_query(const char *q, ...);
int ldap_user_exists(char *addr);
#endif

/** expands placeholders in a user querystring
 *
 * \param format format string to use as input
 * \param addr email address to use for replacements
 * \buf pointer to unallocated buffer for expanded format string, needs to
 *      free'd by caller if not required anymore
 *
 * \returns the number of replacements made or -1 in case of error
 */
int expand_query(char *format, char *addr, char **buf);

#endif
