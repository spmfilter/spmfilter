#ifndef _SMTPD_H
#define _SMTPD_H

/* SMTP States */
#define ST_INIT 0
#define ST_HELO 1
#define ST_MAIL 2
#define ST_RCPT 3
#define ST_DATA 4
#define ST_QUIT 5

typedef int (*LoadMod) (SETTINGS *settings, MAILCONN *mconn);

int load(SETTINGS *settings, MAILCONN *mconn);
void smtp_chat_reply(const char *format, ...);

#endif /* __SMTPD_H */
