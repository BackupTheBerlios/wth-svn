/*

  wthcmd.c
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
  int ocmd, err, telnetfd, connfd, maxfdp1, nready;
  int is_command, *ptr;
  struct sockaddr_in tnservaddr;
  char readline[MAXLINE], response[MAXLINE];
  char *rbuf;
  char *token, *cmdtok, *statok, *initok, *endtok;
  char *brkt, *sep = "\\/:;=- ";
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
        
        for ( token = strtok_r(readline, " ", &brkt); 
                token; token=strtok_r(NULL, sep, &brkt)) {
	  printf("wthcmd: token: \"%s\"\n", token);
	}
    
        if ( ( err = strncmp(readline, "help read", 9)) == 0 ) {
          snprintf(response, sizeof(response), helpread(0,"",""));
          Write(connfd, response, strlen(response));
        }
        else if ( ( err = strncmp(readline, "help", 1)) == 0 ) {
          snprintf(response, sizeof(response), tnusage(0,"",""));
          Write(connfd, response, strlen(response));
        }
        else if ( ( err = strncmp(readline, "read", 3)) == 0) {
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
        else {
          snprintf(response, sizeof(response), tnusage(0,"",""));
          Write(connfd, response, strlen(response));
        }
                        
        if ( is_command == 1 ) {
          /*
             at this place command handling code
          */
          snprintf(rbuf, sizeof(response), "wth called with command: %d\n", ocmd);
          snprintf(response, sizeof(response), rbuf);
          Write(connfd, response, strlen(response));
          is_command = 0;
        }
                        
        snprintf(response, sizeof(response), ">");
        Write(connfd, response, strlen(response));
      }
      Close(connfd);
    }
                
  }
  return(0);
}

/*
  cmdhandler
  interactive commands
  old version
*/

void *
old_cmd_hd( ) {
  int cfd;
  //char *rbuf;
  int waitmax = 0;
  int loopno  = 0;

  for ( ;; ) {
    printf("ws2000_cmdhandler awakening: %d\n", loopno);
    while ( waitmax < MAXWAIT ) {
      printf("ws2000_cmdhandler: waitmax: %d\n", waitmax);
      cfd = open(lckfile, O_RDWR | O_CREAT | O_EXCL, 0444);
      if ( cfd == -1 ) {
        printf("ws2000_cmdhandler: %s\n", strerror(errno));
        printf("ws2000_cmdhandler: lockfile already locked\n");
	sleep(5);
      } else {
	/* sending command to weather station */
        wsconf.command = 0;
        printf("ws2000_cmdhandler: sending command: %d\n", wsconf.command);
	printf("ws2000_cmdhandler: wcmd: %s\n",(char *)wcmd()); 
	//printf("ws2000_cmdhandler: %s", rbuf);
        close(cfd);
        unlink(lckfile);
        waitmax = 0;
        break;
      }
      waitmax++; 
    }
    loopno++;
    sleep(600);
  }
  return((void *) &success);
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

