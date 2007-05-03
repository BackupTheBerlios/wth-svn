/* wthc.c

   client program to communicate w/ WS2000 weatherstation

   $Id: wthc.c,v 1.1 2002/07/04 09:51:48 jahns Exp jahns $
   $Revision: 1.1 $
   
   Copyright (C) 2000-2001,2005,2007 Volker Jahns <volker@thalreit.de>
                                     Rob Burrowes <rob@cs.auckland.ac.nz>

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
#include <unistd.h>
#include "config.h"

int main (int argc, char **argv) {
  int oarg, stime = 0;
  char *rbuf;
  extern int optind;              
  extern char *optarg;
  struct cmd  *pcmd;
  struct wthio wio;

  initdata(&wio);
  pcmd = initcmd();
  if ( ( rbuf = readconfig(pcmd)) == NULL ) {
	perror("Error reading configuration: exit!");
	exit(-1);
  }
  
  openlog("wthc", LOG_PID , pcmd->logfacility);
  //setlogmask(LOG_UPTO(LOG_ERR));
  syslog(LOG_DEBUG, "wthc: %s\n", VERSION);
                                                                    
  /* parsing command line arguments */
  while (( oarg = getopt(argc, argv, "c:h:i:p:stu:v")) != -1 ) {
    switch (oarg) {
    case 'c':
      pcmd->command = atoi(optarg);
      break;
    case 'h':
      pcmd->hostname = optarg;
      pcmd->netflg = 1;
      break;
    case 'i':
      pcmd->argcmd = atoi(optarg);
      break;
    case 'p':
      pcmd->port = optarg;
      break;
    case 's':
      if ( pcmd->netflg != -1 )
	usage(1,"specify serial OR internet connection","");
      pcmd->netflg = 0;
      break;
    case 't':
      stime = 1;
      break;
    case 'v':
      pcmd->verbose = 1;
      printf("wthc version: %s\n", VERSION);
      printf("%s", rbuf);
      break;
    case 'u':
      pcmd->units = optarg;
      break;
    case '?':
      usage(1, "Commandline error", "");
      break;
    default:
      usage(0,"", "");
    }
  }

  /* save command for later use */
  oarg = pcmd->command;

  /* check if intervall time has been set 
     in case cmd.command is request to set intervall time */
  if ( pcmd->command == 6 ) {
      if ((pcmd->argcmd < 1) || (pcmd->argcmd >60))
	  usage(1,"interval time not been specified or out of range", "");
  }

  /* check on serial/internet connection is been chosen correctly */
  if ( pcmd->netflg == -1 )
      usage(1, "specify serial OR internet connection","");

  if ( pcmd->verbose == 1 ) {
	if ( ( rbuf = echoconfig(pcmd)) == NULL) {
	  perror("Error echo config parameters: exit!");
	  exit(-1);
	}
	printf("%s\n", rbuf);
  }
  
  /* sending command to weather station */  
  rbuf =  wcmd(pcmd, &wio); 
  printf("%s", rbuf);
  /* syslog(LOG_INFO, "wthc : wcmd : %s\n", c(o)->descr); */

  if ( stime == 1 ) {
      if ( settime(&wio) == -1 ) 
	perror("Can't set time");
  }

  switch (werrno) {
  case -2:
	printf("serial line error\n");
	break;
  case -3:
    printf("chksum error\n");
	break;
  case 0:
	break;
  }

  closelog();

  exit(0);

} 



