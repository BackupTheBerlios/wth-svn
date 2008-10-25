/*

  cmd.c
  wth command handler

  $Id$

  Copyright 2008, Volker Jahns, volker@thalreit.de

*/
#include "wth.h"
#define MAXWAIT  10
#define DATAWAIT 300

char *lckfile;

/* remove trailing eol character */
void chomp(char *s) {
  while(*s && *s != '\n' && *s != '\r') s++;
  *s = 0;
}

/*
  cmd_hd
  interactive commands
*/
void *
cmd_hd( ) {
  int err, telnetfd, connfd, maxfdp1, nready, ntok;
  struct sockaddr_in tnservaddr;
  char readline[MAXLINE], response[MAXLINE];
  char *rbuf, *sbuf;
  char *token;
  char *sep = "\\/:;=- ";
  fd_set rset;
  const int on = 1;

  /* create telnet TCP socket */
  telnetfd = Socket(AF_INET, SOCK_STREAM, 0);
  bzero(&tnservaddr, sizeof(tnservaddr));
  tnservaddr.sin_family      = AF_INET;
  tnservaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  tnservaddr.sin_port        = htons(atoi(wsconf.port));

  Setsockopt(telnetfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  Bind(telnetfd, (SA *) &tnservaddr, sizeof(tnservaddr));
  Listen(telnetfd, LISTENQ);

  FD_ZERO(&rset);
  maxfdp1 = telnetfd + 1;

  for ( ; ; ) {
    FD_SET(telnetfd, &rset);

    if ( ( nready = select(maxfdp1, &rset, NULL, NULL, NULL)) < 0 ) {
      if ( errno == EINTR )
        continue;
      else
        syslog(LOG_INFO, "select error");
    }
                
    if ( FD_ISSET(telnetfd, &rset)) {
      connfd = Accept(telnetfd, (SA *) NULL, NULL);
      snprintf(response, sizeof(response), "\n\n%s\n%s\n>",
               "Welcome to wth service","Type help for help");
      Write(connfd, response, strlen(response));
      for ( ; ; ) {
        snprintf(readline,1," ");
        Read(connfd, readline, MAXLINE);
        chomp(readline);
        rbuf = strdup(readline);

	printf("cmd_hd: inputstring: \"%s\"\n", rbuf);
	ntok=0; token = strtok_r(rbuf, sep, &sbuf);
	if ( ( err = strncmp( token,"help",4)) == 0) {
          snprintf(response, sizeof(response), "cmd_hd: %s\n", tnhelp( sbuf));
          Write(connfd, response, strlen(response));
	} else if ( ( err = strncmp( token, "exec",4)) == 0 ) {
	  printf("cmd_hd: %s\n\n", execmd( sbuf)); 
	} else if ( ( err = strncmp( token, "show",4)) == 0 ) {
          snprintf(response, sizeof(response), "cmd_hd: %s\n", showcmd(sbuf));
          Write(connfd, response, strlen(response));
	} else if ( ( err = strncmp( token, "reload",4)) == 0 ) {
	  printf("cmd_hd: %s\n\n", initcmd(token)); 
	} else if ( ( err = strncmp( token, "restart",4)) == 0 ) {
	  printf("cmd_hd: %s\n\n", initcmd(token));
	  initcmd(sbuf);
	} else if ( ( err = strncmp(readline, "quit", 1)) == 0) {
          break;
	} else if ( token == NULL) {
	  printf("cmd_hd: token NULL\n");
	  tnusage(0, "", ""); 
	} else {
	  printf("cmd_hd: provide HELP (unknown command)\n");
	}


        /*
        ntok=0;
        for ( token = strtok_r(rbuf, sep, &brkt); 
                token; token=strtok_r(NULL, sep, &brkt)) {
	  printf("wthcmd: ntok: %d token: \"%s\"\n", ntok, token);
          ntok++;
          if ( ( err = strncmp(token, "help", 4)) == 0) {
            token = strtok_r(NULL, sep, &brkt);
            printf("wthcmd: help token found: next token: %s\n", token);
            if ( ( err = strncmp(token, "read", 4)) == 0 ) {
              snprintf(response, sizeof(response), helpread(0,"",""));
              Write(connfd, response, strlen(response));
            } else {
              snprintf(response, sizeof(response), tnusage(0,"",""));
              Write(connfd, response, strlen(response));
            }
          }
	}
 
        if ( ( err = strncmp(readline, "read", 3)) == 0) {
          ocmd = atoi(readline);
          is_command = 1;
        }
        else if ( ( err = strncmp(readline, "status", 4)) == 0) {
          snprintf(response, sizeof(response) , tnstat("ws2000"));
          Write(connfd, response, strlen(response));
        }
        else if ( ( err = strncmp(readline, "1", 1)) == 0) {
          ocmd = atoi(readline);
          is_command = 1;
        }
        else if ( ( err = strncmp(readline, "q", 1)) == 0)
          break;
        else if ( ( err = strncmp(readline, "quit", 4)) == 0)
          break;

	*/

        //else {
        //  snprintf(response, sizeof(response), tnusage(0,"",""));
        //  Write(connfd, response, strlen(response));
        //}
        
        /*                
        if ( is_command == 1 ) {
          snprintf(rbuf, sizeof(response), "wth called with command: %d\n", ocmd);
          snprintf(response, sizeof(response), rbuf);
          Write(connfd, response, strlen(response));
          is_command = 0;
        }
        */
                
        snprintf(response, sizeof(response), ">");
        Write(connfd, response, strlen(response));
      }
      Close(connfd);
    }
                
  }
  return(0);
}


/* tnstat: weatherstation status */
char *
tnstat ( char *station) {
  int i, err;
  static char t[MAXBUFF] = "";
  char buf[MAXMSGLEN];
  char *s;
  struct tm *tm;
  time_t lastread, statread;

  printf("tnstat: station: %s\n", station);

  if  ( ( err = strncmp( station, "ws2000", 5)) == 0) { 
    if ( ws2000station.status.is_present == 1) {
      s = mkmsg2("WS2000 weatherstation status\n"
            "version\t\t:\t%x\nmeasure interval:\t%lu (min)\n",
            ws2000station.status.version,
            ws2000station.status.interval
                );
      strncat( t, s, strlen(s));

      if ( ws2000station.status.DCFstat == 1 ) {
        s = mkmsg2("DCF status\t:\t%d (DCF receiver present)\n",
               ws2000station.status.DCFstat);
      }
      else {
        s = mkmsg2("DCF status\t:\t%d (no DCF receiver found)\n",
               ws2000station.status.DCFstat);
      }
      strncat( t, s, strlen(s));
      if ( ws2000station.status.DCFsync == 1 ) {
        s = mkmsg2("DCF sync.\t:\t%d (DCF synchronized)\n",
               ws2000station.status.DCFsync);
      }
      else {
        s = mkmsg2("DCF sync.\t:\t%d (DCF NOT synchronized)\n",
               ws2000station.status.DCFsync);
      }
      strncat( t, s, strlen(s));
      if ( ws2000station.status.HFstat == 1 ) {
        s = mkmsg2("HF status\t:\t%d (with HF)\n",
               ws2000station.status.HFstat);
      }
      else {
        s = mkmsg2("HF status\t:\t%d (without HF)\n",
               ws2000station.status.HFstat);
      }
      strncat(t,s,strlen(s));
      s = mkmsg2("battery status\t:\t%d\n",
             ws2000station.status.Battstat);
      strncat(t,s,strlen(s));
  
      s = mkmsg2("sensor number\t:\t%d (sensors)\n",
             ws2000station.status.numsens);
      strncat(t,s,strlen(s));

      s = mkmsg2("sensorname\tsensorstatus\tlastseen\n"
            "------------\t------------\t----------\n"
            );
      strncat(t,s,strlen(s));
      time(&statread);
      lastread = statread - ws2000station.status.lastread;

      if ( lastread > 600 ) {
        ws2000station.status.lastread = statread;
        readstat( station);
      }
      for ( i = 1; i <=ws2000station.status.numsens; i++) {
        tm = gmtime(&ws2000station.sensor[i].lastseen);
        strftime(buf, sizeof(buf), "%b %e, %Y %H:%M:%S %Z", tm);
        s = mkmsg2("%12s\t%d\t\t%s\n",
          ws2000station.sensor[i].sensorname,
          ws2000station.sensor[i].status,
          buf
          );
        strncat(t,s,strlen(s));
      }
    } else { 
     s = mkmsg2("No WS2000 weatherstation attached."
                "Please check line, if this should not be so");
     strncat(t, s, strlen(s)); 
    }
  } else if ( ( ( err = strncmp(station, "pcwsr", 5)) == 0 ) 
           && ( pcwsrstation.status.is_present == 1)) {
      s = mkmsg2("Pcwsr weatherstation status\n");
      strncat( t, s, strlen(s));
  }
  return(t);
}



/* tnusage : print handling instructions for telnet access wthd */
char *
tnusage (int exitcode, char *error, char *addl) {
  char *s;

  s = mkmsg2("wthd commands:\n"
             "\t\tread\n"
             "\t\tshow\n"
             "\t\treload\n"
             "\t\trestart\n"
             "\t\texec\n"
             "\t\thelp\n"
             "type help <command> to get more help\n");
  return(s);
}

/* helpread : print handling instructions for wthd data read */
char *
helpread (int exitcode, char *error, char *addl) {
  char *s;

  s = mkmsg2("read wthd data:\n"
             "\t\tread <station>\n"
             "\t\tread <station> LAST\n"
             "\t\tread <station> start <timestart>\n"
             "\t\tread <station> start <timestart> end <timeend>\n"
             "where <station> can be ws2000, pcwsr or 1wire\n");
  return(s);
}


char *
tnhelp( char *args) {
  int err, ntok;
  int is_show, is_init, is_exec;
  int is_conf, is_data, is_stat;
  char *rbuf;
  char *sbuf;
  char *sep = "\\/:;=- ";
  char *token;

  if ( ( rbuf = malloc(MAXMSGLEN+1)) == NULL )
    return NULL;

  //printf("tnhelp: input string: \"%s\"\n", args);
  if ( args == NULL ) {
    snprintf(rbuf,MAXMSGLEN, "tnhelp: provide general HELP\n");
    return(rbuf);
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
      snprintf(rbuf, MAXMSGLEN, "thhelp: provide help on SHOW DATA");
    } else if ( is_stat == 1) {
      snprintf(rbuf, MAXMSGLEN, "thhelp: provide help on SHOW STATUS");
    } else if ( is_conf == 1) {
      snprintf(rbuf, MAXMSGLEN, "thhelp: provide help on SHOW CONFIG");
    } else {
      snprintf(rbuf, MAXMSGLEN, "thhelp: provide general help on SHOW");
    }
  } else if ( is_init == 1 ) {
    snprintf(rbuf, MAXMSGLEN, "tnhelp: provide help on RESTART/RELOAD");
  } else if ( is_exec == 1) {
    snprintf(rbuf, MAXMSGLEN, "tnhelp: provide help on EXEC");
  } else {  // catch all 
    snprintf(rbuf, MAXMSGLEN, "thhelp: provide general HELP");
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
    snprintf(rbuf,MAXMSGLEN, "execmd: provide HELP (called w/o all args)");
    return(rbuf);
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
    } else if ( ncmd == 100) {
      snprintf(rbuf, MAXMSGLEN, "execmd: provide HELP (ws2000 called w/o command)");
    } else {
      snprintf(rbuf, MAXMSGLEN, "execmd: provide HELP (ws2000 command unknown)");
    }
  } else {  // catch all 
    snprintf(rbuf, MAXMSGLEN, 
      "execmd: provide general HELP (only ws2000 supported)");
  }
  return(rbuf);
}

char *
showcmd( char *args) {
  int err, ntok;
  int is_ws2000, is_pcwsr, is_1wire;
  int is_data, is_stat, is_conf;
  char *rbuf;
  char *sbuf;
  char *sep = "\\/:;=- ";
  char *token;

  if ( ( rbuf = malloc(MAXMSGLEN+1)) == NULL )
    return NULL;

  if ( args == NULL ) {
    snprintf(rbuf,MAXMSGLEN, "showcmd: provide HELP (called w/o all args");
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
    } else if ( ( err = strncmp(token, "1wire", 5) == 0 )) {
      is_1wire = 1;
    }
  } while ( ( token = strtok_r( NULL, sep, &sbuf)) != NULL );

  if ( is_data == 1) {
    if ( is_ws2000 == 1 ) { 
      snprintf(rbuf, MAXMSGLEN, "showcmd: show data ws2000");
    } else if ( is_pcwsr == 1) {
      snprintf(rbuf, MAXMSGLEN, "showcmd: show data pcwsr");
    } else if ( is_1wire == 1) {
      snprintf(rbuf, MAXMSGLEN, "showcmd: show data 1wire)");
    } else {
      snprintf(rbuf, MAXMSGLEN, "showcmd: provide HELP (no weatherstation specified)"); 
    }
  } else if ( is_stat == 1) { 
    if ( is_ws2000 == 1 ) { 
      snprintf(rbuf, MAXMSGLEN, "showcmd: show status ws2000");
    } else if ( is_pcwsr == 1) {
      snprintf(rbuf, MAXMSGLEN, "showcmd: show status pcwsr");
    } else if ( is_1wire == 1) {
      snprintf(rbuf, MAXMSGLEN, "showcmd: show status 1wire)");
    } else {
      snprintf(rbuf, MAXMSGLEN, "showcmd: provide HELP (no weatherstation specified)"); 
    }
  } else if ( is_conf == 1) { 
    if ( is_ws2000 == 1 ) { 
      snprintf(rbuf, MAXMSGLEN, "showcmd: show config ws2000");
    } else if ( is_pcwsr == 1) {
      snprintf(rbuf, MAXMSGLEN, "showcmd: show config pcwsr");
    } else if ( is_1wire == 1) {
      snprintf(rbuf, MAXMSGLEN, "showcmd: show config 1wire)");
    } else {
      snprintf(rbuf, MAXMSGLEN, "showcmd: provide HELP (no weatherstation specified)"); 
    }
  } else {  // catch all 
    snprintf(rbuf, MAXMSGLEN, 
      "execmd: provide general HELP (only ws2000 supported)");
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

