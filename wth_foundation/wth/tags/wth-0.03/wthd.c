/*
   wthd - the Weather daemon
   
   	 
   tcp service for interactive weather 
   
   	 tcp server program to read serial connected weatherstation interface
   	 
   
   features
      IPv4
   		TCP
   		iterative
   
   $Id: wthd.c,v 0.4 2001/09/14 15:39:39 jahns Exp jahns $
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

 
#include        "wth.h"


int
main(int argc, char **argv)
{
  /* network */
  int		       err, listenfd, telnetfd, connfd, maxfdp1, nready;
  struct sockaddr_in   servaddr, tnservaddr;
  char		       readline[MAXLINE];

  /* data */
  struct cmd           cmd;
  struct wthio         wio;
  int                  ndata = 0;
  int                  nobg = 0;
  int                  is_command = 0;
  int                  o;
  unsigned char        data[MAXBUFF];
  char                 *nakfram = "\x02\x01\x15\xe8\x03";
  char                 *rbuf;
  fd_set               rset;
	
  werrno = 0;

  initdata(&wio);
  initcmd(&cmd);
  readconfig(&cmd);
	
  /* parse commandline */
  while ((o = getopt(argc, argv, "dp:")) != -1) {
    switch(o) {
    case 'd':
      nobg = 1;
      break;
    case 'p':
      cmd.port = strdup(optarg);
      break;
    case '?':
      usaged(1,"command line error","");
      break;
    default:
      usaged(0, "", "");
    }
  }

  if ( nobg == 0 ) 
    daemon_init("wthd", LOGFACILITY);
  else if ( nobg == 1) {
    openlog("wthd", LOG_PID, LOGFACILITY);
    printf("wthd: ready\n");
  }
	
  /* create 1st TCP socket */
  listenfd = Socket(AF_INET, SOCK_STREAM, 0); 
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family      = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port        = htons(atoi(cmd.port));
  Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
  Listen(listenfd, LISTENQ);


  /* create 2nd TCP socket */
  telnetfd = Socket(AF_INET, SOCK_STREAM, 0);
  bzero(&tnservaddr, sizeof(tnservaddr));
  tnservaddr.sin_family      = AF_INET;
  tnservaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  tnservaddr.sin_port        = htons(atoi(cmd.tnport));
	
  Bind(telnetfd, (SA *) &tnservaddr, sizeof(tnservaddr));
  Listen(telnetfd, LISTENQ);

  FD_ZERO(&rset);
  maxfdp1 = max(listenfd, telnetfd) + 1;

  for ( ; ; ) {
    FD_SET(listenfd, &rset);
    FD_SET(telnetfd, &rset);

    if ( ( nready = select(maxfdp1, &rset, NULL, NULL, NULL)) < 0 ) {
      if ( errno == EINTR )
	continue;
      else
	syslog(LOG_INFO, "select error");
    }

    if ( FD_ISSET(listenfd, &rset)) {
      connfd = accept(listenfd, (SA *) NULL, NULL); 
      Read(connfd, readline, MAXLINE);
      cmd.command = o = atoi(readline);
      cmd.argcmd  = 0;
      cmd.netflg  = 0;
         
      if ( getsrd( data, &ndata, &cmd) == -1 ) {
	syslog(LOG_INFO,"error reading serial data");
	strncpy(data, nakfram, strlen(nakfram));
	ndata = strlen(nakfram);
      }
		 
      Write(connfd, data, ndata);
      syslog(LOG_INFO,"command request: %s\n", c(o)->descr);
      Close(connfd);
    }
		
    if ( FD_ISSET(telnetfd, &rset)) {
      connfd = Accept(telnetfd, (SA *) NULL, NULL);
      snprintf(readline, sizeof(readline), "\n\n%s\n\t%s\n>",
	       "Welcome to wth service","Type h for help");
      Write(connfd, readline, strlen(readline));
      cmd.argcmd = 0;
      cmd.netflg = 0;
      for ( ; ; ) {
	Read(connfd, readline, MAXLINE);

	if ( ( err = strncmp(readline, "h", 1)) == 0 ) {
	  snprintf(readline, sizeof(readline), tnusage(0,"",""));
	  Write(connfd, readline, strlen(readline));
	}
	else if ( ( err = strncmp(readline, "0", 1)) == 0) {
	  cmd.command = o = atoi(readline);
	  is_command = 1;
	}
	else if ( ( err = strncmp(readline, "1", 1)) == 0) {
	  cmd.command = o = atoi(readline);
	  is_command = 1;
	}
	else if ( ( err = strncmp(readline, "2", 1)) == 0) {
	  cmd.command = o = atoi(readline);
	  is_command = 1;
	}
	else if ( ( err = strncmp(readline, "3", 1)) == 0) {
	  cmd.command = o = atoi(readline);
	  is_command = 1;
	}
	else if ( ( err = strncmp(readline, "4", 1)) == 0) {
	  cmd.command = o = atoi(readline);
	  is_command = 1;
	}
	else if ( ( err = strncmp(readline, "5", 1)) == 0) {
	  cmd.command = o = atoi(readline);
	  is_command = 1;
	}
	else if ( ( err = strncmp(readline, "q", 1)) == 0)
	  break;
	else if ( ( err = strncmp(readline, "quit", 4)) == 0)
	  break;
	else {
	  snprintf(readline, sizeof(readline), tnusage(0,"",""));
	  Write(connfd, readline, strlen(readline));
	}
			
	if ( is_command == 1 ) {
	  rbuf = wcmd(&cmd, &wio);
	  printf("rbuf: %s\n", rbuf);
	  snprintf(readline, sizeof(readline), rbuf);
	  Write(connfd, readline, strlen(readline));
	  cmd.argcmd  = 0;
	  cmd.netflg  = 0;
	  is_command = 0;
	}
			
	snprintf(readline, sizeof(readline), ">");
	Write(connfd, readline, strlen(readline));
      }
      Close(connfd);
    }
		
  }
  return(0);
}


