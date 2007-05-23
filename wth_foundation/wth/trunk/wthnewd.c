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
  pid_t pid_pcwsr_handler, pid_ws2000_handler;

  initdata();
  openlog("wthnewd", LOG_PID , wsconf.logfacility);
  syslog(LOG_INFO, "wthnewd: %s\n", VERSION);
  tzset(); /* setting timezone */

  /* fork off PCWSR child */
  pid_pcwsr_handler = fork();
  if ( pid_pcwsr_handler == 0 ) { /* child code */ 
    printf("pcwsr child code starts\n");
    /*
      if ( ( rbuf = (char *)echoconfig(wsconf)) == NULL) {
      perror("Error echo config parameters: exit!");
      exit(-1);
      }
      printf("%s\n", rbuf);
    */
    pcwsr_handler();
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
  pid_ws2000_handler = fork();
  if ( pid_ws2000_handler == 0 ) { /* child code */ 
    printf("WS2000 child code starts\n");
    if ( ( rbuf = (char *)echoconfig(wsconf)) == NULL) {
      perror("Error echo config parameters");
    }
    printf("%s\n", rbuf);

    ws2000_handler();
  }


  if ( waitpid(pid_pcwsr_handler, NULL, 0) < 0 ) { /* parent is waiting! */
    perror("Waitpid error: pcwsr_handler()!");
    exit(1);
  }

  if ( waitpid(pid_ws2000_handler, NULL, 0) < 0 ) { /* parent is waiting! */
    perror("Waitpid error: ws2000_handler()!");
    exit(1);
  }

  exit(0);
}

