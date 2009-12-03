#ifndef _SMTPD_H
#define _SMTPD_H

/* SMTP States */
#define ST_INIT 0
#define ST_HELO 1
#define ST_MAIL 2
#define ST_RCPT 3
#define ST_DATA 4
#define ST_QUIT 5

#define CODE_221 "221 Goodbye. Please recommend us to others!\r\n"
#define CODE_250 "250 OK\r\n"
#define CODE_250_ACCEPTED "250 OK message accepted\r\n"
#define CODE_451 "451 Requested action aborted: local error in processing\r\n"
#define CODE_500 "500 Eh? WTF was that?\r\n"
#define CODE_552 "552 Requested action aborted: local error in processing\r\n"

typedef int (*LoadMod) (MAILCONN *mconn);

int load(MAILCONN *mconn);
void smtp_string_reply(const char *format, ...);
void smtp_code_reply(SETTINGS *settings,int code);

#endif /* __SMTPD_H */
