/* wthc.c

   client program to communicate w/ WS2000 weatherstation

   $Id: wthc.c,v 0.2 2001/03/01 06:30:54 volker Exp jahns $
   $Revision: 0.2 $
   

 */

#include "wth.h"

int main (int argc, char **argv) {
  int err;                        /* return value of functions, 
                                     useful for errorhandling */  
  int setno;                      /* number of datasets */

  /* used for commandline parsing */
  int c;
  extern int optind;              
  extern char *optarg;
  struct cmd cmd;                 /* command structure */

  /* used for storing data */
  struct sensor sens[MAXSENSORS]; /* structure contains sensor status, type 
                                     and data */

  openlog("wthc", LOG_PID , LOGFACILITY);

  /* initialize structure sens which holds *all* information on the
     sensors */
  err =  initsens(sens);
  syslog(LOG_DEBUG, "Initializing datastructure sens : %d\n", err);
                                                                    

  /* initialize struct cmd */
  cmd.netflg = -1;
  cmd.port   = WPORT; 
  
  /* parsing command line arguments */
  while (( c = getopt(argc, argv, "c:h:i:p:s")) != -1 ) {
      switch (c) {
	  case 'c':
              cmd.command = atoi(optarg);
	      break;
	  case 'h':
	      cmd.hostname = optarg;
              cmd.netflg = 1;
	      break;
	  case 'i':
              cmd.argcmd = atoi(optarg);
	      break;
	  case 'p':
              cmd.port = optarg;
	      break;
          case 's':
	      if ( cmd.netflg != -1 )
		  usage(1,"specify serial OR internet connection","");
	      cmd.netflg = 0;
	      break;
          case '?':
	      usage(1, "Commandline error", "");
	      break;
          default:
	      usage(0,"", "");
       }
   }
   

  /* check if intervall time has been set 
     in case cmd.command is request to set intervall time */
  if ( cmd.command == 6 ) {
      if ((cmd.argcmd < 1) || (cmd.argcmd >60))
	  usage(1,"interval time not been specified or out of range", "");
  }

  /* check on serial/internet connection is been chosen correctly */
  if ( cmd.netflg == -1 )
      usage(1, "specify serial OR internet connection","");

  /* sending command to weather station */
  err = wcmd(sens, &cmd, &setno);
  syslog(LOG_INFO, "wthc : return code wcmd :%d\n", err);
  switch (err) {
  case -2:
	printf("dataset error\n");
	break;
  case -1:
    printf("timeout: check wiring\n");
	break;
  case 0:
	break;
  }
  closelog();
  return(err);

  
} 



