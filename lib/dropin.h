/*
    dropin.h -- header file for dropin.c
    Copyright (C) 2000-2003 Ivo Timmermans <ivo@o2w.nl>,
                  2000-2003 Guus Sliepen <guus@sliepen.eu.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    $Id: dropin.h,v 1.3 2003/08/24 20:38:20 guus Exp $
*/

#ifndef __DROPIN_H__
#define __DROPIN_H__

#include "fake-getaddrinfo.h"
#include "fake-getnameinfo.h"

#ifndef HAVE_DAEMON
extern int daemon(int, int);
#endif

#ifndef HAVE_GET_CURRENT_DIR_NAME
extern char *get_current_dir_name(void);
#endif

#ifndef HAVE_ASPRINTF
extern int asprintf(char **, const char *, ...);
#endif

#ifndef HAVE_GETNAMEINFO
extern int getnameinfo(const struct sockaddr *sa, size_t salen, char *host,
					   size_t hostlen, char *serv, size_t servlen, int flags);
#endif

#ifndef HAVE_GETTIMEOFDAY
extern int gettimeofday(struct timeval *, void *);
#endif

#ifndef HAVE_RANDOM
extern long int random(void);
#endif

#endif							/* __DROPIN_H__ */