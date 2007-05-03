#include "wthlib.h"


int main (int argc, char **argv) {
  int i;
  int cm;                         /* command sent to weather station */
  int argcm;                      /* command argument sent to weather station */
  int err;                        /* return value of functions, 
                                     useful for errorhandling */
  int setno;                      /* number of datasets */

  struct DCFstruct DCF;           /* DCF status and synchronous */
  struct sensor sens[MAXSENSORS]; /* structure contains sensor status, type 
                                     and data */
  struct cmd cmd;                 /* command structure */

  openlog(argv[0], LOG_PID , LOGFACILITY);

  /* initialize structure sens which holds *all* information on the
     sensors */
  err =  initsens(sens);
  syslog(LOG_DEBUG, "Initializing datastructure sens : %d\n", err); 

  /* parsing command line arguments */
  if ( (argc != 2) && (argc != 3) ) {
      printf("Usage: wstst <command> <command argument>, where\n");
      printf("<command> takes the values\n");
      printf("\t0\tPoll DCF Time\n");
      printf("\t1\tRequest dataset\n");
      printf("\t2\tSelect next dataset\n");
      printf("\t3\tActivate 9 temperature sensors\n");
      printf("\t4\tActivate 16 temperature sensors\n");
      printf("\t5\tRequest status\n");
      printf("\t6\tSet interval time to <commandargument>\n");
      printf("\t12\tRequest all available data recursively\n");

      exit(1);
  }

  if ( argc == 3 ) {  
  	  /* argcm is a commandargument byte sent to weatherstation */
          /* 
             the only command which currently needs this argument is
             6 : set intervall time
          */ 
          argcm = atoi(argv[2]);
  } 

  cm = atoi(argv[1]);

  cmd.command = cm;
  cmd.argcmd  = argcm;
  cmd.netflg  = 0;

  /* sending command to weather station */
  err = wcmd(sens, &cmd, &setno);  

  syslog(LOG_DEBUG, "main : return code wcmd :%d\n", err);
  closelog();
  return(0);

  
} 



