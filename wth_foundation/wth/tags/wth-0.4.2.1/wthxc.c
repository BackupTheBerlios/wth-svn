/* wthc.c

   client program to communicate w/ WS2000 weatherstation

   $Id: wthc.c,v 0.4 2001/09/14 15:38:00 jahns Exp jahns $
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
#include "version.h"

int main (int argc, char **argv) {
  int o;
  char *rbuf;
  extern int optind;              
  extern char *optarg;
  struct cmd cmd;                 /* command structure */
  struct wthio wio;               /* result */


  initdata(&wio);
  initcmd(&cmd);
  if ( ( rbuf = readconfig(&cmd)) == NULL ) {
	perror("Error reading configuration: exit!");
	exit(-1);
  }
  
  openlog("wthxc", LOG_PID , cmd.logfacility);
  syslog(LOG_INFO, "%s\n", wth_version);
                                                                    
  /* parsing command line arguments */
  while (( o = getopt(argc, argv, "c:h:xi:p:sv")) != -1 ) {
      switch (o) {
	  case 'c':
	    cmd.command = atoi(optarg);
	    break;
	  case 'h':
	    cmd.hostname = optarg;
	    cmd.netflg = 1;
	    break;
	  case 'x':
	    cmd.netflg = 2;
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
          case 'v':
	    cmd.verbose = 1;
	    printf("wthc: %s\n", wth_version);
	    printf("%s", rbuf);
	    break;
          case '?':
	    usage(1, "Commandline error", "");
	    break;
          default:
	    usage(0,"", "");
      }
  }

  /* save command for later use */
  o = cmd.command;

  /* check if intervall time has been set 
     in case cmd.command is request to set intervall time */
  if ( cmd.command == 6 ) {
      if ((cmd.argcmd < 1) || (cmd.argcmd >60))
	  usage(1,"interval time not been specified or out of range", "");
  }

  /* check on serial/internet connection is been chosen correctly */
  if ( cmd.netflg == -1 )
      usage(1, "specify serial OR internet connection","");

  if (cmd.verbose == 1 ) {
	if ( ( rbuf = echoconfig(&cmd)) == NULL) {
	  perror("Error echo config parameters: exit!");
	  exit(-1);
	}
	printf("%s\n", rbuf);
  }
  
  /* sending command to weather station */  
  rbuf =  wcmd(&cmd, &wio); 
  printf("%s", rbuf);
  /* syslog(LOG_INFO, "wthc : wcmd : %s\n", c(o)->descr); */
  
  switch (werrno) {
  case ESERIAL:
    printf("serial line error\n");
    break;
  case ECHKSUM:
    printf("data chksum error\n");
    break;
  case ERCPT:
    printf("faulty data reception\n");
    break;
  case 0:
    break;
  }
  closelog();
  
  return(0);  
} 




