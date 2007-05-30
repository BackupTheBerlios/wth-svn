/*

  wthnewd.c

  server to read WS2000 weatherstation and pcwsr weathersensor receiver

  The principle is to fork off a child which handles the serial data read,
  right now data are echoed to standard out

  Copyright (C) Jun 2002,2007 Volker Jahns, Volker.Jahns@thalreit.de

*/
#include "wthnew.h"


int
main ( int argc, char **argv ) 
{
  char *rbuf;
  pid_t pid_pcwsr_loghandler, 
        pid_ws2000_loghandler,
        pid_ws2000_cmdhandler;

  char *ws2000lck = WS2000LOCK;

  initdata();
  openlog("wthnewd", LOG_PID , wsconf.logfacility);
  syslog(LOG_INFO, "wthnewd: %s\n", VERSION);
  unlink( ws2000lck);
  tzset(); /* setting timezone */

  /* fork off PCWSR child */
  pid_pcwsr_loghandler = fork();
  if ( pid_pcwsr_loghandler == 0 ) { /* child code */ 
    printf("pcwsr_loghandler: child code starts\n");
    /*
      if ( ( rbuf = (char *)echoconfig(wsconf)) == NULL) {
      perror("Error echo config parameters: exit!");
      exit(-1);
      }
      printf("%s\n", rbuf);
    */
    pcwsr_loghandler();
  }

  /* fork off WS2000 handler */
  /* actually two handlers are needed for the WS2000 PC interface :
    1st handler: retieves data and status of PC interface at regular times
    2nd handler: interactively reads commands
                 send them to the PC interface
                 reads the interface answer
                 send back the result
    at present time only a stub to execute command 0 has been implemented
  */
  /*
    logging WS2000 data to rrd and Sqlite DB
   */
  pid_ws2000_loghandler = fork();
  if ( pid_ws2000_loghandler == 0 ) { /* child code */ 
    printf("WS2000_loghandler : child code starts\n");
    if ( ( rbuf = (char *)echoconfig(wsconf)) == NULL) {
      perror("Error echo config parameters");
    }
    printf("%s\n", rbuf);

    ws2000_loghandler();
  }

  /*
    handling interactive commands to WS2000
  */
  pid_ws2000_cmdhandler = fork();
  if ( pid_ws2000_cmdhandler == 0 ) { /* child code */ 
    printf("WS2000_cmdhandler : child code starts\n");
    ws2000_cmdhandler();
  }


  if ( waitpid(pid_pcwsr_loghandler, NULL, 0) < 0 ) { /* parent is waiting! */
    perror("Waitpid error: pcwsr_loghandler()!");
    exit(1);
  }
  if ( waitpid(pid_ws2000_loghandler, NULL, 0) < 0 ) { /* parent is waiting! */
    perror("Waitpid error: ws2000_loghandler()!");
    exit(1);
  }
  if ( waitpid(pid_ws2000_cmdhandler, NULL, 0) < 0 ) { /* parent is waiting! */
    perror("Waitpid error: ws2000_cmdhandler()!");
    exit(1);
  }




  exit(0);
}

