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



void chomp(char *s) {
while(*s && *s != '\n' && *s != '\r') s++;

*s = 0;
}



/*
  cmd_hd
  interactive commands
  old version
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

