#ifndef _SMTPD_H
#define _SMTPD_H

#define CODE_221 "221 Goodbye. Please recommend us to others!\r\n"
#define CODE_250 "250 OK\r\n"
#define CODE_250_ACCEPTED "250 OK message accepted\r\n"
#define CODE_451 "451 Requested action aborted: local error in processing\r\n"
#define CODE_500 "500 Eh? WTF was that?\r\n"
#define CODE_552 "552 Requested action aborted: local error in processing\r\n"

typedef int (*LoadMod) (MAILCONN *mconn);

int load(void);
void smtp_string_reply(const char *format, ...);
void smtp_code_reply(int code);

#endif /* __SMTPD_H */
