/*

  cmd.c
  wth command handler

  $Id$
  $Revision$

  Copyright (C) 2000-2001,2005,2007,2009 Volker Jahns <Volker.Jahns@thalreit.de>

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
#define MAXWAIT  10
#define DATAWAIT 300

char *lckfile;

/* remove trailing eol character */
void 
chomp( char *s) {
  while(*s && *s != '\n' && *s != '\r') s++;
  *s = 0;
}


/* sig_chld handler handles waidpid */
void
sig_chld( int signo) {

  pid_t pid;
  int stat;

  while ( ( pid = waitpid(-1, &stat, WNOHANG)) > 0 )
    syslog(LOG_INFO, "child %d terminated", pid);

  return;  
}

/*
  cmd_hd  interactive commands

  based on udpservselect01.c by R.W.Stephans, UNIX Network Programming

*/
void *
cmd_hd( void *arg) {
  int listenfd, connfd, maxfdp1, nready;
  struct sockaddr_in cliaddr, tnservaddr;
  socklen_t clilen;
  fd_set rset;
  const int on = 1;
  pid_t childpid;
  void sig_chld(int);

  /* create telnet TCP socket */
  listenfd = Socket(AF_INET, SOCK_STREAM, 0);

  bzero(&tnservaddr, sizeof(tnservaddr));
  tnservaddr.sin_family      = AF_INET;
  tnservaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  tnservaddr.sin_port        = htons(atoi(wsconf.port));
  printf("cmd: wsconf.port: %s\n", wsconf.port);
  Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

  Bind(listenfd, (SA *) &tnservaddr, sizeof(tnservaddr));

  Listen(listenfd, LISTENQ);

  signal(SIGCHLD, sig_chld);
  FD_ZERO(&rset);
  maxfdp1 = listenfd + 1;

  for ( ; ; ) {
    
    FD_SET(listenfd, &rset);

    if ( ( nready = select(maxfdp1, &rset, NULL, NULL, NULL)) < 0 ) {
      if ( errno == EINTR )
        continue;
      else
        syslog(LOG_INFO, "select error");
    }
                
    if ( FD_ISSET(listenfd, &rset)) {
      clilen = sizeof( cliaddr);
      connfd = Accept(listenfd, (SA *) &cliaddr, &clilen);

      if ( ( childpid = Fork()) == 0) { /* child process */
	Close(listenfd);
	docmd( connfd);
	exit(0);
      }
      Close(connfd); /* parent process closes connected socket */
     
    }
                
  }
  return(0);
}


/* docmd: command processing wth telnet */
int
docmd( int sockfd) {
  int err, ntok;
  char readline[MAXLINE+1], response[MAXLINE+1];
  char *rbuf, *sbuf;
  char *token;
  char *sep = "\\/:;=- ";

  snprintf(response, sizeof(response), "\n\n%s\n%s\n>",
	   "Welcome to wth service","Type help for help");
  Write(sockfd, response, strlen(response));

  for ( ; ; ) {
    snprintf(readline,1," ");
    Read(sockfd, readline, MAXLINE);
    chomp(readline);
    rbuf = strdup(readline);

    printf("docmd: inputstring: \"%s\"\n", rbuf);
    ntok=0; token = strtok_r(rbuf, sep, &sbuf);
    printf("docmd: token done\n");

    if ( token == NULL) {
      printf("docmd: token\n");
    }

    if ( token == NULL) {
      printf("docmd: token NULL\n");
      snprintf(response, sizeof(response), tnusage(0, "", "")); 
      Write(sockfd, response, strlen(response));
    } else if ( ( err = strncmp( token,"help",4)) == 0) {
      snprintf(response, sizeof(response), "%s", tnhelp( sbuf));
      Write(sockfd, response, strlen(response));
    } else if ( ( err = strncmp( token, "exec",4)) == 0 ) {
      snprintf(response, sizeof(response), "%s", execmd( sbuf));
      Write(sockfd, response, strlen(response));
    } else if ( ( err = strncmp( token, "show",4)) == 0 ) {
      snprintf(response, sizeof(response), "%s", showcmd(sbuf));
      Write(sockfd, response, strlen(response));
    } else if ( ( err = strncmp( token, "reload",4)) == 0 ) {
      printf("docmd: %s\n\n", initcmd(token)); 
    } else if ( ( err = strncmp( token, "restart",4)) == 0 ) {
      printf("docmd: %s\n\n", initcmd(token));
      initcmd(sbuf);
    } else if ( ( err = strncmp(readline, "quit", 1)) == 0) {
      break;
    } else {
      printf("docmd: provide HELP (unknown command)\n");
      snprintf(response, sizeof(response), tnusage(0, "", "")); 
      Write(sockfd, response, strlen(response));
    }
    snprintf(response, sizeof(response), ">");
    Write(sockfd, response, strlen(response));
  }
  return(err);
}

/* tnstat: weatherstation status */
char *
tnstat ( char *station) {
  int err;
  static char rbuf[NBUFF+1];

  snprintf(rbuf, NBUFF, "");
  if  ( ( err = strncmp( station, "ws2000", 5)) == 0) { 
    if ( ws2000station.status.is_present == 1) {
      snprintf(rbuf, NBUFF, readstat( "ws2000"));
    } else { 
     snprintf( rbuf, TBUFF, "No WS2000 weatherstation attached.\n"
       "Please check hardware, if you do not expect this correct.\n");
    }
  } else if  ( ( err = strncmp(station, "pcwsr", 5)) == 0 ) { 
      if ( pcwsrstation.status.is_present == 1) {
        snprintf( rbuf, NBUFF, readstat( "pcwsr"));
      } else {
        snprintf( rbuf, NBUFF, "No PCWSR weatherstation found.\n"
          "Please allow 5-10 minutes for PCWSR data to accumulate. "
          "Check hardware, if you do not expect this correct.\n");
      }
  } else if  ( ( err = strncmp(station, "onewire", 6)) == 0 ) { 
      if ( onewirestation.status.is_present == 1) {
        snprintf( rbuf, NBUFF, readstat( "onewire"));
      } else {
        snprintf( rbuf, NBUFF, "No ONEWIRE weatherstation found.\n"
          "Check hardware, if you do not expect this correct.\n");
      }
  }
  return(rbuf);
}


/* tnusage : print handling instructions for telnet access wthd */
char *
tnusage (int exitcode, char *error, char *addl) {
  static char s[SBUFF+1];

  if ( exitcode == 0 ) {  /* general help */
    snprintf(s, SBUFF, "wthd commands:\n"
             "\t\tshow\n"
             "\t\texec\n"
             "\t\tquit\n"
             "\t\treload\n"
             "\t\trestart\n"
             "\t\thelp\n"
             "type help <command> to get more help\n");
  } else if ( exitcode == 1 ) { /* help on show */
    snprintf( s, SBUFF, "wthd show commands:\n"
             "\tshow data <station>\t show last measurement values\n"
             "\tshow status <station>\t show sensor status\n"
             "\tshow config <station>\t show configuration parameters\n"
	     "where <station> is ws2000, pcwsr or onewire\n");
  } else if ( exitcode == 2 ) { /* help on exec */
    snprintf(s, SBUFF, "wthd exec commands:\n"
      "\texec ws2000 polldcftime\task ws2000 for current CDF time\n"
      "\texec ws2000 requestdataset\tretrieve first dataset record\n"
      "\texec ws2000 selectnextdataset\tset cursor to next dataset record\n"
      "\texec ws2000 set9sensor\tset ws2000 to receive data of 9 sensors\n"
      "\texec ws2000 set16sensor\tset ws2000 to receive data of 16 sensors\n"
      "\texec ws2000 pollstatus\tretrieve station and sensor status\n"
      "\texec ws2000 setintervaltime\tset measurement time interval\n"
      "only WS2000 executes commands\n"
     );
  } else {
    syslog(LOG_INFO,"tnusage: unknown exitcode: %d\n", exitcode);
  }
  return(s);
}

/* helpshow : print handling instructions for wthd data read */
char *
helpshow (int exitcode, char *error, char *addl) {
  static char s[TBUFF+1];
  snprintf(s, TBUFF, "To show weatherstation data, type:\n"
             "\t\tshow data <station>\n"
             "\t\tshow data <station> at <time>\n"
             "where\n\t<station> can be ws2000, pcwsr or onewire\n"
             "\t<time> is requested timestamp in format DD-MM-YYYY\n");
  return(s);
}


char *
tnhelp( char *args) {
  int err, ntok;
  int is_show, is_init, is_exec;
  int is_conf, is_data, is_stat;
  static char rbuf[TBUFF+1];
  char *sbuf;
  char *sep = "\\/:;=- ";
  char *token;

  if ( args == NULL ) {
    snprintf(rbuf,TBUFF, tnusage( 0, "", ""));
    return( rbuf);
  }

  ntok=0; 
  token = strtok_r(args, sep, &sbuf);

  do {
    ntok++;
    if ( ( err = strncmp(token, "show", 4) == 0 )) {
      is_show = 1;
    } else if ( ( err = strncmp(token, "reload", 4) == 0 )) {
      is_init = 1;
    } else if ( ( err = strncmp(token, "restart", 4) == 0 )) {
      is_init = 1;
    } else if ( ( err = strncmp(token, "exec", 4) == 0 )) {
      is_exec = 1;
    } else if ( ( err = strncmp(token, "exec", 4) == 0 )) {
      is_exec = 1;
    } else if ( ( err = strncmp(token, "config", 4) == 0 )) {
      is_conf = 1;
    } else if ( ( err = strncmp(token, "data", 4) == 0 )) {
      is_data = 1;
    } else if ( ( err = strncmp(token, "stat", 4) == 0 )) {
      is_stat = 1;
    }
  } while ( ( token = strtok_r( NULL, sep, &sbuf)) != NULL );

  if ( is_show == 1) {
    if ( is_data == 1) {
      snprintf(rbuf, TBUFF, "thhelp: provide help on SHOW DATA");
    } else if ( is_stat == 1) {
      snprintf(rbuf, TBUFF, "thhelp: provide help on SHOW STATUS");
    } else if ( is_conf == 1) {
      snprintf(rbuf, TBUFF, "thhelp: provide help on SHOW CONFIG");
    } else {
      snprintf(rbuf, TBUFF, "thhelp: provide general help on SHOW");
    }
  } else if ( is_init == 1 ) {
    snprintf(rbuf, TBUFF, "tnhelp: provide help on RESTART/RELOAD");
  } else if ( is_exec == 1) {
    snprintf(rbuf, TBUFF, tnusage( 2, "", ""));
  } else {  // catch all 
    snprintf(rbuf, TBUFF, tnusage( 0,"",""));
  }
  return(rbuf);
}


char *
execmd( char *args) {
  int err, ntok;
  int is_ws2000;
  int ncmd;
  char *rbuf;
  char *sbuf;
  char *sep = "\\/:;=- ";
  char *token;

  if ( ( rbuf = malloc(MAXMSGLEN+1)) == NULL )
    return NULL;

  if ( args == NULL ) {
    return(tnusage(2, "", ""));
  }

  ntok=0; 
  token = strtok_r(args, sep, &sbuf);
  do {
    ntok++;
    if ( ( err = strncmp(token, "ws2000", 4) == 0 )) {
      is_ws2000 = 1;
    } else if ( ( err = strncmp(token, "polldcftime", 4) == 0 )) {
      ncmd = 0;
    } else if ( ( err = strncmp(token, "requestdataset", 4) == 0 )) {
      ncmd = 1;
    } else if ( ( err = strncmp(token, "selectnextdataset", 4) == 0 )) {
      ncmd = 2;
    } else if ( ( err = strncmp(token, "set9sensor", 4) == 0 )) {
      ncmd = 3;
    } else if ( ( err = strncmp(token, "set16sensor", 5) == 0 )) {
      ncmd = 4;
    } else if ( ( err = strncmp(token, "pollstatus", 4) == 0 )) {
      ncmd = 5;
    } else if ( ( err = strncmp(token, "setintervaltime", 4) == 0 )) {
      ncmd = 6;
    } else if ( token == NULL ) {
      ncmd = 100;
    } else { /* no command in arguments */
      ncmd = 100;
    }
  } while ( ( token = strtok_r( NULL, sep, &sbuf)) != NULL );

  if ( is_ws2000 == 1) {
    if ( ntok > 1 ) { 
      snprintf(rbuf, MAXMSGLEN, "execmd: ws2000: execute cmd %d", ncmd);
      wsconf.command = ncmd;
      if ( ( err = wcmd()) == 0 ) {
        snprintf(rbuf, MAXMSGLEN, ws2000station.status.message); return(rbuf);
      } else { snprintf(rbuf, MAXMSGLEN, "error: no response of WS2000"); }
    } else if ( ncmd == 100) {
      return( tnusage( 2, "WS2000 called without command", ""));
    } else {
      return( tnusage( 2, "WS2000 command unknown", ""));
    }
  } else {  // catch all 
    return(tnusage( 2, "Only WS2000 supported", ""));
  }
  return(NULL);
}

char *
showcmd( char *args) {
  int err, ntok;
  int is_ws2000, is_pcwsr, is_onewire, is_unknown;
  int is_data, is_stat, is_conf;
  static char rbuf[NBUFF+1];;
  char *sbuf;
  char *sep = "\\/:;=- ";
  char *token;

  snprintf(rbuf, NBUFF, "");
  if ( args == NULL ) {
    snprintf(rbuf, NBUFF, tnusage(1,"",""));
    return(rbuf);
  }

  ntok=0; 
  token = strtok_r(args, sep, &sbuf);
  do {
    ntok++;
    if ( ( err = strncmp(token, "data", 4) == 0 )) {
      is_data = 1;
    } else if ( ( err = strncmp(token, "status", 4) == 0 )) {
      is_stat = 1;
    } else if ( ( err = strncmp(token, "config", 4) == 0 )) {
      is_conf = 1;
    } else if ( ( err = strncmp(token, "ws2000", 4) == 0 )) {
      is_ws2000 = 1;
    } else if ( ( err = strncmp(token, "pcwsr", 4) == 0 )) {
      is_pcwsr = 1;
    } else if ( ( err = strncmp(token, "onewire", 5) == 0 )) {
      is_onewire = 1;
    } else {
      is_unknown = 1;
    }
  } while ( ( token = strtok_r( NULL, sep, &sbuf)) != NULL );

  if ( is_data == 1) {
    if ( is_ws2000 == 1 ) { 
      snprintf(rbuf, NBUFF, readdb("ws2000"));
    } else if ( is_pcwsr == 1) {
      snprintf(rbuf, NBUFF, readdb("pcwsr"));
    } else if ( is_onewire == 1) {
      snprintf(rbuf, NBUFF, readdb("onewire"));
    } else {
      snprintf(rbuf, NBUFF, tnusage(1,"","")); 
    }
  } else if ( is_stat == 1) { 
    if ( is_ws2000 == 1 ) { 
      snprintf(rbuf, NBUFF, tnstat("ws2000"));
    } else if ( is_pcwsr == 1) {
      snprintf(rbuf, NBUFF, tnstat("pcwsr"));
    } else if ( is_onewire == 1) {
      printf("showcmd: is_onewire : %d\n", is_onewire);
      snprintf(rbuf, NBUFF, tnstat("onewire"));
    } else {
      snprintf(rbuf, MAXMSGLEN, tnusage(1,"","")); 
    }
  } else if ( is_conf == 1) { 
    if ( is_ws2000 == 1 ) { 
      snprintf(rbuf, NBUFF, echoconfig("ws2000"));
    } else if ( is_pcwsr == 1) {
      snprintf(rbuf, NBUFF, echoconfig("pcwsr"));
    } else if ( is_onewire == 1) {
      snprintf(rbuf, NBUFF, echoconfig("onewire"));
    } else {
      snprintf(rbuf, NBUFF, tnusage(1,"","")); 
    }
  } else {  // catch all 
    snprintf(rbuf, NBUFF, tnusage(1,"",""));
  }
  return(rbuf);
}

char *
initcmd( char *args) {
  int err;
  int is_reload, is_restart;
  char *rbuf;

  if ( ( rbuf = malloc(MAXMSGLEN+1)) == NULL )
    return NULL;

  if ( args == NULL ) {
    snprintf(rbuf,MAXMSGLEN, "initcmd: provide HELP (called w/o all args)");
  } else if ( ( err = strncmp(args, "reload", 4)) == 0 ) {
    is_reload = 1;
  } else if ( ( err = strncmp(args, "restart", 4)) == 0 ) {
    is_restart = 1;
  }

  
  if ( is_reload == 1) {
    snprintf(rbuf, MAXMSGLEN, "initcmd: reload"); 
  } else if ( is_restart == 1) { 
    snprintf(rbuf, MAXMSGLEN, "initcmd: restart"); 
  } else { 
    snprintf(rbuf, MAXMSGLEN, "initcmd: provide HELP"); 
  }
  return(rbuf);

}

