/* $Id: fake-getnameinfo.h,v 1.2 2003/08/24 20:38:20 guus Exp $ */

#ifndef _FAKE_GETNAMEINFO_H
#define _FAKE_GETNAMEINFO_H

#ifndef HAVE_GETNAMEINFO
int getnameinfo(const struct sockaddr *sa, size_t salen, char *host, 
                size_t hostlen, char *serv, size_t servlen, int flags);
#endif /* !HAVE_GETNAMEINFO */

#ifndef NI_MAXSERV
# define NI_MAXSERV 32
#endif /* !NI_MAXSERV */
#ifndef NI_MAXHOST
# define NI_MAXHOST 1025
#endif /* !NI_MAXHOST */

#endif /* _FAKE_GETNAMEINFO_H */