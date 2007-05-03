/* net.c


   network related functions of wthc and wthd

   $Id: net.c,v 0.4 2001/09/14 15:41:30 jahns Exp jahns $
   $Revision: 0.4 $

   Copyright (C) 2000-2001 Volker Jahns <Volker.Jahns@thalreit.de>

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
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 
*/

#include "wth.h"
#include "wthnet.h"
#include "wthprv.h"

extern int daemon_proc; 

/* include sock_ntop */
char *
sock_ntop(const struct sockaddr *sa, socklen_t salen)
{
    char		portstr[7];
    static char str[128];		/* Unix domain is largest */

	switch (sa->sa_family) {
	case AF_INET: {
		struct sockaddr_in	*sin = (struct sockaddr_in *) sa;

		if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
			return(NULL);
		if (ntohs(sin->sin_port) != 0) {
			snprintf(portstr, sizeof(portstr), ".%d", ntohs(sin->sin_port));
			strcat(str, portstr);
		}
		return(str);
	}
/* end sock_ntop */

#ifdef	IPV6
	case AF_INET6: {
		struct sockaddr_in6	*sin6 = (struct sockaddr_in6 *) sa;

		if (inet_ntop(AF_INET6, &sin6->sin6_addr, str, sizeof(str)) == NULL)
			return(NULL);
		if (ntohs(sin6->sin6_port) != 0) {
			snprintf(portstr, sizeof(portstr), ".%d", ntohs(sin6->sin6_port));
			strcat(str, portstr);
		}
		return(str);
	}
#endif

#ifdef	AF_UNIX
	case AF_UNIX: {
		struct sockaddr_un	*unp = (struct sockaddr_un *) sa;

			/* OK to have no pathname bound to the socket: happens on
			   every connect() unless client calls bind() first. */
		if (unp->sun_path[0] == 0)
			strcpy(str, "(no pathname bound)");
		else
			snprintf(str, sizeof(str), "%s", unp->sun_path);
		return(str);
	}
#endif

#ifdef	HAVE_SOCKADDR_DL_STRUCT
	case AF_LINK: {
		struct sockaddr_dl	*sdl = (struct sockaddr_dl *) sa;

		if (sdl->sdl_nlen > 0)
			snprintf(str, sizeof(str), "%*s",
					 sdl->sdl_nlen, &sdl->sdl_data[0]);
		else
			snprintf(str, sizeof(str), "AF_LINK, index=%d", sdl->sdl_index);
		return(str);
	}
#endif
	default:
		snprintf(str, sizeof(str), "sock_ntop: unknown AF_xxx: %d, len %d",
				 sa->sa_family, salen);
		return(str);
	}
    return (NULL);
}

char *
Sock_ntop(const struct sockaddr *sa, socklen_t salen)
{
	char	*ptr;

	if ( (ptr = sock_ntop(sa, salen)) == NULL) {
	  syslog(LOG_INFO,"sock_ntop error");	/* inet_ntop() sets errno */
	  werrno = ENET;
	  return (NULL);
	}
	return(ptr);
}


int
daemon_init(const char *pname, int facility)
{
	int		i;
	pid_t	pid;

	if ( (pid = Fork()) != 0)
		exit(0);			/* parent terminates */

	/* 41st child continues */
	setsid();				/* become session leader */

	signal(SIGHUP, SIG_IGN);
	if ( (pid = Fork()) != 0)
		exit(0);			/* 1st child terminates */

	/* 42nd child continues */
	daemon_proc = 1;		/* for our err_XXX() functions */

	chdir("/");				/* change working directory */

	umask(0);				/* clear our file mode creation mask */

	for (i = 0; i < MAXFD; i++)
		close(i);

	openlog(pname, LOG_PID, facility);

	return(0);
}



int
tcp_listen(const char *host, const char *serv, socklen_t *addrlenp)
{
	int				listenfd, n;
	const int		on = 1;
	struct addrinfo	hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0) {
		syslog(LOG_INFO,"tcp_listen error for %s, %s: %s",
			   host, serv, gai_strerror(n));
		werrno = ENET;
		return (-1);
	}
	ressave = res;

	do {
		listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (listenfd < 0)
			continue;		/* error, try next one */

		Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
		if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
			break;			/* success */

		Close(listenfd);	/* bind error, close and try next one */
	} while ( (res = res->ai_next) != NULL);

	if (res == NULL) {	/* errno from final socket() or bind() */
	  syslog(LOG_INFO,"tcp_listen error for %s, %s", host, serv);
	  werrno = ENET;
	  return(-1);
	}

	Listen(listenfd, LISTENQ);

	if (addrlenp)
		*addrlenp = res->ai_addrlen;	/* return size of protocol address */

	freeaddrinfo(ressave);

	return(listenfd);
}
/* end tcp_listen */

/*
 * We place the wrapper function here, not in wraplib.c, because some
 * XTI programs need to include wraplib.c, and it also defines
 * a Tcp_listen() function.
 */

int
Tcp_listen(const char *host, const char *serv, socklen_t *addrlenp)
{
	return(tcp_listen(host, serv, addrlenp));
}



int
Setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
  if (setsockopt(fd, level, optname, optval, optlen) < 0) {
	werrno = errno;
	syslog(LOG_INFO,"setsockopt error: %s",
		   strerror(werrno));
	return(-1);
  }
  return(0);
}


/* include Listen */
int
Listen(int fd, int backlog)
{
	char	*ptr;

		/*4can override 2nd argument with environment variable */
	if ( (ptr = getenv("LISTENQ")) != NULL)
		backlog = atoi(ptr);

	if (listen(fd, backlog) < 0) {
	  werrno = errno;
	  syslog(LOG_INFO, "Listen: listen error: %s",
			 strerror(werrno));
	  return(-1);
	}
	return(0);
}
/* end Listen */



/* include Socket */
int
Socket(int family, int type, int protocol)
{
	int		n;

	if ( (n = socket(family, type, protocol)) < 0) {
	  syslog(LOG_INFO,"socket error");
	  werrno = ENET;
	  return (-1);
	}
	return(n);
}
/* end Socket */



int
Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
	int		n;

again:
	if ( (n = accept(fd, sa, salenptr)) < 0) {
#ifdef	EPROTO
		if (errno == EPROTO || errno == ECONNABORTED)
#else
		if (errno == ECONNABORTED)
#endif
			goto again;
		else {
		  syslog(LOG_INFO,"accept error");
		  werrno = errno;
		  return(-1);
		}
	}
	return(n);
}



int
Bind(int fd, const struct sockaddr *sa, socklen_t salen)
{
  if (bind(fd, sa, salen) < 0) {
	werrno = errno;
	syslog(LOG_INFO,"Bind: bind error: %s",
		   strerror(werrno));
	return(-1);
  }
  return(0);
}


/* getnrd 
   
   write command to wthd and read response
   IPv4 version
 
*/
int getnrd(unsigned char *data, int *mdat, struct cmd *pcmd) {
    int                  sockfd, n, len;
    char                 sendline[MAXLINE] = "1";
    struct sockaddr_in   servaddr;

    struct in_addr **pptr, *addrs[2];
    struct hostent *hp;
    struct servent *sp;
	
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;

    /* hostname or ipaddress */
    if (inet_pton(AF_INET, pcmd->hostname, &servaddr.sin_addr) == 1) {
	  addrs[0] = &servaddr.sin_addr;
	  addrs[1] = NULL;
	  pptr = &addrs[0];
    } else if ( (hp = gethostbyname(pcmd->hostname)) != NULL) {
	  pptr = (struct in_addr **) hp->h_addr_list;
    } else {
	  werrno = h_errno;
	  syslog(LOG_INFO,"hostname error for %s: %s", 
		 pcmd->hostname, hstrerror(h_errno));
	  return (-1);
	  }
    /* port number or service name */
    if ( (n = atoi(pcmd->port)) > 0)
	  servaddr.sin_port = htons(n);
    else if ( (sp = getservbyname(pcmd->port, "tcp")) != NULL)
	  servaddr.sin_port = sp->s_port;
    else {
	  syslog(LOG_INFO,"getservbyname error for %s", pcmd->port);
	  werrno = ENET;
	  return(-1);
    }
    /* connect to server */
    for ( ; *pptr != NULL; pptr++) {
      if ( ( sockfd = Socket(AF_INET, SOCK_STREAM, 0)) == -1 )
	return(-1);

      memmove(&servaddr.sin_addr, *pptr, sizeof(struct in_addr));
      Sock_ntop((SA *) &servaddr, sizeof(servaddr));
	  
      if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) == 0)
	break; /* success */

      werrno = errno;
      syslog(LOG_INFO, "getnrd: connect error: %s",
	     strerror(werrno));
      return(-1);

      close(sockfd);
    }
    if (*pptr == NULL ) {
      syslog(LOG_INFO, "unable to connect");
      werrno = ENET;
      return(-1);
    }

    /* write command to server */
    snprintf(sendline, sizeof(sendline), "%d\r\n", (*pcmd).command);
    if ( Writen(sockfd, sendline, 1) == -1 )
      return(-1);
    
    /* read response. rwstephens unp p.10 */
    while ( ( n = read(sockfd, data, MAXBUFF)) > 0) {
      data[n] = 0;
    }

    /* code doesn't work. why?
       if (Readline(sockfd, line, MAXLINE) == 0)
       err_quit("getnrd: server terminated prematurely");
    */

    len = wstrlen(data);
    *mdat = len;
	
    return(0);
}



