/* ws2000.c

   ws2000 handler in conjunction with wthnewd

   $Id$

   Copyright (C) 2001-2004,2007 Volker Jahns <volker@thalreit.de>

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
#include "wthnew.h"
#define MAXWAIT  10
#define DATAWAIT 300

char *lockfile = WS2000LOCK;
sqlite3 *ws2000db;

/*
  wloghandler
  logging WS2000 data to rrd and sqlite DB

*/
void *
wloghandler( ) {
  int lfd;
  int waitmax  = 0;

  for ( ;; ) {
    printf("wloghandler awakening\n");
    while ( waitmax < MAXWAIT ) {
      printf("wloghandler: waitmax: %d\n", waitmax);
      lfd = open( lockfile, O_RDWR | O_CREAT | O_EXCL, 0444);
      if ( lfd == -1 ) {
        printf("wloghandler: %s\n", strerror(errno));
        printf("wloghandler: lockfile already locked\n");
	sleep(5);
      } else {
	/* sending command to weather station */
        wsconf.command = 12;
        printf("wloghandler: sending command: %d\n", wsconf.command); 
	printf("wloghandler: wcmd: %s\n",(char *)wcmd()); 
        close( lfd);
        unlink( lockfile);
        waitmax = 0;
        break;
      }
      waitmax++; 
    }
    sleep(DATAWAIT);
  }
  return(0);
}



/*
  cmdhandler
  interactive commands
*/

void *
cmdhandler( ) {
  int cfd;
  //char *rbuf;
  int waitmax = 0;
  int loopno  = 0;

  for ( ;; ) {
    printf("ws2000_cmdhandler awakening: %d\n", loopno);
    while ( waitmax < MAXWAIT ) {
      printf("ws2000_cmdhandler: waitmax: %d\n", waitmax);
      cfd = open(lockfile, O_RDWR | O_CREAT | O_EXCL, 0444);
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
        unlink(lockfile);
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



/* wcmd
  
   execution of WS2000 weatherstation commands

*/
char *
wcmd ( ) {
  int ndat = 0;                   /* length of array containing the 
                                     data frame */
  int err;                        /* return value of functions */
  int command;
  int argcm;
  int dle_isset = FALSE;
  static unsigned char data[MAXBUFF]; /* data array to store the raw dataframe 
                                         and the message datagram */
  char *clk;                          /* display time in reasonable format */
  char *rbuf;                         /* return buffer */

  syslog(LOG_DEBUG, "wcmd: called for command request: %d\n",wsconf.command);  
  ws2000station.status.ndats = 0;

  /* first get DCF time if possible */
  command = wsconf.command; 
  argcm   = wsconf.argcmd; 
  wsconf.command = 0;
  if ( ( err = getcd( data, &ndat)) == -1) {
    return( (char *)mkmsg2("wcmd: error data reception\n"));
  }
  wsconf.command = command;

  ws2000station.status.DCFtime  = dcftime(data, ndat);
  if ( ws2000station.status.DCFtime == -1) {
    syslog( LOG_DEBUG, "wcmd: DCF not synchronized\n");
  }

  /* get status of weatherstation 
     needed to fill sens.status
  */
  command = wsconf.command; 
  argcm   = wsconf.argcmd; 
  wsconf.command = 5;
  if ( ( err = getcd( data, &ndat)) == -1) {
    return( (char *)mkmsg2( "wcmd: error data reception\n"));
  }
  wsconf.command = command;
  syslog(LOG_DEBUG, "wcmd : check status OK\n");


  /* status weatherstation */
  if ( ( rbuf = wstat(data, ndat)) == NULL) {
    return( ( char *)mkmsg2("wcmd: error in subroutine wstat\n"));
  }
  
  /* command 0 : poll DCF time */
  if (command == 0) {
    tzset();
	
    /* write command and retrieve data */
    if ( ( err = getcd( data, &ndat)) == -1) {
      return( ( char *)mkmsg2("wcmd: error data reception\n"));
    }

    /* calculate seconds since EPOCH if DCF synchronized */
    ws2000station.status.DCFtime  = dcftime(data, ndat);
    if ( ws2000station.status.DCFtime == -1) {
      return( ( char*)mkmsg2("DCF not synchronized\n"));
    }
    else {
      clk = ctime(&ws2000station.status.DCFtime);
      return( ( char *)mkmsg2("%s", clk));
    }
  }

  /* command 1 : Request Dataset */
  else if (command == 1)  {
    /* first get DCF time if possible */
    wsconf.command = 0;
    if ( ( err = getcd( data, &ndat)) == -1) {
      return( (char *)mkmsg2("wcmd: error data reception\n"));
    }
    wsconf.command = command;

    /* calculate seconds since EPOCH if DCF synchronized */
    ws2000station.status.DCFtime  = dcftime(data, ndat);

    /* write command and retrieve data */
    if ( ( err = getcd( data, &ndat)) == -1) {
      return( ( char *)mkmsg2("wcmd: error data reception\n"));
    }

    /* weather station response : no data available: <DLE> */
    if ( ( ndat == 1 ) && ( data[0] == DLE ) ) {
      return( ( char*)mkmsg2("no data available (<DLE> received)\n"));
    }
    /* fill data structure sens */
    else {
      /* get one dataset */
      err = datex(data, ndat);
      syslog(LOG_DEBUG, "wcmd : returncode datex : %d\n", err);
      ws2000station.status.ndats = ws2000station.status.ndats + 1;
    }
    return( ( char *)mkmsg2("wcmd: data available\n"));
    /* echo sensor data */
    /*
    if ( ws2000station.status.ndats > 0 )
      rbuf = pdata();
    */
  } 


  /* command 2 : Select next dataset */
  else if (command == 2) {
    /* write the command word to the weather station */ 
    /* extract message datagram */
    if ( ( err = getcd( data, &ndat)) == -1) {
      return( ( char *)mkmsg2("wcmd: error data reception\n"));
    }


    /* if DLE no data available */
    if ( ( ndat == 1 ) && ( data[0] == DLE ) ) {
      return( ( char *)mkmsg2("no data available(<DLE> received)\n"));
    } else if ( ( ndat == 1 ) && ( data[0] == ACK ) )  {
      /* if ACK next dataset is available */
      return( ( char *)mkmsg2("next dataset available (<ACK> received)\n"));
    }
    /* exit if unknown response */
    else {
      return( ( char *)mkmsg2("error next dataset : \"unknown response\"\n"));
    }
  }


  /* command 3 : Activate 8 temperature sensors */
  else if (command == 3) {

    /* write the command word to the weather station */ 
    /* extract message datagram */
    if ( ( err = getcd( data, &ndat)) == -1) {
      return( ( char *)mkmsg2("wcmd: error data reception\n"));
    }

    /* weather station response : <ACK> */
    if ( ( ndat == 1 ) && ( data[0] == ACK ) ) {
      return( ( char *)mkmsg2("set 9 temperature sensors (<ACK> received)\n"));
    }

    /* update status */
    wsconf.command = 5;
    if ( ( err = getcd( data, &ndat)) == -1) {
      return( ( char *)mkmsg2("wcmd: error data reception\n"));
    }
    if ( ( wstat(data, ndat)) == NULL) {
      return( ( char *)mkmsg2("wcmd: error in subroutine wstat\n"));
    }	  
  } 


  /* command 4 : Activate 16 temperature sensors */
  else if (command == 4) {

    /* write the command word to the weather station */ 
    /* extract message datagram */
    if ( ( err = getcd( data, &ndat)) == -1) {
      return( ( char *)mkmsg2("wcmd: error data reception\n"));
    }

    /* weather station response : <ACK> */
    if ( ( ndat == 1 ) && ( data[0] == ACK ) ) {
      return( ( char *)mkmsg2("set 16 temperature sensors(<ACK> received)\n"));
    }

    /* update status */
    wsconf.command = 5;
    if ( ( err = getcd( data, &ndat)) == -1) {
      return( ( char *)mkmsg2("wcmd: error data reception\n"));
    }
    if ( ( wstat(data, ndat)) == NULL) {
      return( ( char *)mkmsg2("wcmd: error in subroutine wstat\n"));
    }	  
  } 


  /* command 5 : Request status of weatherstation */
  else if ( command == 5 ) {
    if ( ( err = getcd( data, &ndat)) == -1) {
      return( (char *)mkmsg2( "wcmd: error data reception\n"));
    }
    
    /* status weatherstation */
    if ( ( rbuf = wstat(data, ndat)) == NULL) {
      return( ( char *)mkmsg2("wcmd: error in subroutine wstat\n"));
    } else {
      return( ( char *)rbuf);
    }
  } 


  /* command 6 : Set logging intervall of weatherstation */
  else if (command == 6) {
    /* write the command word to the weather station */ 
    /* extract message datagram */
    if ( ( err = getcd( data, &ndat)) == -1) {
      return( ( char *)mkmsg2("wcmd: error data reception\n"));
    }

    /* weather station response : <ACK> */
    if ( ( ndat == 1 ) && ( data[0] == ACK ) ) {
      return( ( char *)mkmsg2("set logging interval to %d [min] (<ACK> received)\n", wsconf.argcmd));
    }
  } 


  /* command 12 : 
     Recursively, request dataset and select next dataset, 
     recursive combination of command 1 and 2 
  */
  else if ( ( command == 12))  {      
    /* first get DCF time if possible */
    wsconf.command = 0;
    if ( ( err = getcd( data, &ndat)) == -1) {
      return( ( char *)mkmsg2("wcmd: error data reception\n"));
    }

    /* calculate seconds since EPOCH if DCF synchronized */
    ws2000station.status.DCFtime  = dcftime(data, ndat);
	  
    while ( dle_isset != TRUE) {
      /* 1st step: command #1: request dataset */
      wsconf.command = 1;
      if ( ( err = getcd( data, &ndat)) == -1) {
	return( ( char *)mkmsg2("wcmd: error data reception\n"));
      }

      /* extract dataset from raw dataframe */
      /* weather station response : no data available: <DLE> */
      if ( ( ndat == 1 ) && ( data[0] == DLE ) ) {
	syslog(LOG_INFO,
          "wcmd: command 12: no data available (<DLE> received)\n");
	dle_isset = TRUE;
      }
      /* do extraction */
      else {
	if ( ( err = datex(data, ndat)) == -1) {
	  return( ( char *)mkmsg2("wcmd: error extracting data frame"));
	}
	ws2000station.status.ndats++;
      }
 
      /* 2nd step: command #2: request next dataset */
      wsconf.command = 2;
      if ( ( err = getcd( data, &ndat)) == -1) {
	return( ( char *)mkmsg2("wcmd: error data reception\n"));
      }

      /* stop if DLE no data available, */
      if ( ( ndat == 1 ) && ( data[0] == DLE ) ) {
	syslog(LOG_INFO,
          "wcmd: command 12: no data available ( (<DLE> received)\n");
	dle_isset = TRUE; 
      }
      /* if ACK next dataset is available */
      else if ( ( ndat == 1 ) && ( data[0] == ACK ) ) {
	syslog(LOG_INFO, \
	       "wcmd: next dataset available (<ACK> received)!\n");
      }
      /* return if unknown response */
      /* no data is returned, maybe too strict? */
      else {
	return( ( char *)mkmsg2(
		     "wcmd: error request next dataset : unknown response\n"));
      }
    }
    return( ( char *)mkmsg2("wcmd: command 12: no data available ( ( <DLE> received)\n"));
  }//command 12

  else {
    return( ( char *)mkmsg2("unknown command\n"));
  }
  /* catch all: should never be reached */
  return( (char *)mkmsg2("called wcmd w/o command: this should not happen\n"));
}/* wcmd ends here */


/* demasq
  
   dataframe correction: STX, ETX and ENQ are masqued in the
   raw frame sent by the weather station. 

   function demasq demasquerades these bytes
   
   before calculating the checksum the dataframe 
   has to corrected for masqueraded STX, ETX and ENQ:

	   <STX> is masqueraded in raw frame as <ENQ><DC2>
	   <ETX> is masqueraded in raw frame as <ENQ><DC3>
	   <ENQ> is masquerared in raw frame as <ENQ><NAK>

   don't know what happens if checksum equals STX, ETX or ENQ
   it should be masqueraded in the dataframe

*/
int 
demasq(unsigned char *data, int *mdat) {
    int i;
    int j;

    syslog(LOG_DEBUG, "demasq : length dataframe : %d\n", *mdat);

    /* process all data but first and last byte */
    for ( i = 1; i < *mdat - 2; i++ ) {
        if ( data[i] == ENQ && data[i+1] == DC2 ) {
            syslog(LOG_DEBUG, 
		   "demasq : STX found: i %d: data[i] %x: data[i+1] %x\n",
		   i, data[i], data [i+1]);
            data[i] = STX;
            for ( j = i+1; j < *mdat - 1; j++ ) data[j] = data[j + 1]; 
            *mdat -=1;
        }
        if ( data[i] == ENQ && data[i+1] == DC3 ) {
            syslog(LOG_DEBUG, 
		   "demasq : ETX found: i %d: data[i] %x: data[i+1] %x\n",
		   i, data[i], data [i+1]);
            data[i] = ETX;
            for ( j = i+1; j < *mdat - 1; j++ ) data[j] = data[j + 1]; 
            *mdat -= 1; 
        }
        if ( data[i] == ENQ && data[i+1] == NAK ) {
            syslog(LOG_DEBUG, 
		   "demasq : ENQ found: i %d: data[i] %x: data[i+1] %x\n",
		   i, data[i], data [i+1]);
            data[i] = ENQ;
            for ( j = i+1; j < *mdat - 1; j++ ) data[j] = data[j + 1]; 
            *mdat -= 1;
        }

    }
    syslog(LOG_DEBUG, "demasq : length dataframe after corr :%d\n", *mdat);
    return(0);
} 


/* getrd

   get raw data
*/
int getrd ( unsigned char *data, int *mdat) {
    int err;

    if ( wsconf.netflg == 0 ) /* read serial interface */ {
	  if ( ( err = getsrd( data, mdat)) == -1)
		return(-1);
    }
    else if ( wsconf.netflg == 1 ) /* read network interface */ {
	  if ( ( err = getnrd( data, mdat)) == -1)
		return(-1);
    } else {
      syslog(LOG_DEBUG, "getrd : netflag undefined : %d\n", wsconf.netflg);
      return(-1);
    }
    return(0);
}


/* getcd

   get corrected data
*/
int 
getcd ( unsigned char *data, int *mdat) {
    int i;
    int err;

    /* read answer of weather station from serial port */
    if ( ( err = getrd( data, mdat)) == -1)
	  return(-1);

    /* echo raw dataframe */
    err = echodata( data, *mdat);
    
    /* check data frame and do 
    	 data correction for masqueraded bytes */
    if ( ( err = chkframe( data, mdat)) == -1)
         return(-1);
    
    /* echo raw demasqueraded dataframe */
    err = echodata(data, *mdat);

    /* extract message: eliminiate first and last 2 bytes */
    for ( i = 0; i < *mdat - 4; i++ ) {
        data[i] = data[i+2];
    }
    *mdat = *mdat - 4;

    syslog(LOG_DEBUG, "getcd : Data length getcd : %d\n", *mdat);
    return(0);
}


/* chkframe

   dataframe format is

   <STX><length>[message]<checksum><ETX>
     1      1       n        1       1

   i.e. the data frame contains the data messsage 
     + 2 start bytes 
     + 2 end bytes
   message is also denoted as weatherstation response
	 
*/
int 
chkframe( unsigned char *data, int *mdat) {
    int i;
    int chksum;
    int ldat;
    unsigned char *nakfram = "\x02\x01\x15\xe8\x03";

    ldat = *mdat;

    /* check fist byte is STX */
    if ( data[0]      == STX ) 
	  syslog(LOG_DEBUG, "chkframe : STX OK\n");
    else {
	  syslog(LOG_DEBUG, "chkframe : STX NOK\n");
	  werrno = ECHKSUM;
	  return(-1);
	}

    /* check last byte is ETX */
    if ( data[ldat-1] == ETX ) 
	  syslog(LOG_DEBUG, "chkframe : ETX OK\n");
    else {
	  syslog(LOG_DEBUG, "chkframe : ETX NOK\n");
	  return(-1);
	}

    /* check length of dataframe :

       mdat is number of elements in array data
       calculate length by function wstrlen and compare both integers
    */
    if ( wstrlen( data) == ldat ) 
	  syslog(LOG_DEBUG, "chkframe : frame length OK\n");
    else {
	  syslog(LOG_DEBUG, "chkframe : frame length NOK\n");
	  return(-1);
    }
	
    /* to calculate checksum demasquerade <STX, <ETX> and <ENQ> first */
    demasq( data, &ldat);

    /* calculate checksum */
    chksum = 0;
    for ( i = 0; i < ldat - 2; i++ ) {
	    chksum = chksum + data[i];
    }
    chksum = ( 256 - chksum ) & 0xff;

    if ( chksum == data[ldat - 2] )
      syslog(LOG_DEBUG, "chkframe : checksum match OK\n");
    else {
      syslog(LOG_DEBUG, "chkframe : checksum match NOK\n");
      werrno = ECHKSUM;
      return(-1);
    }


    /* check length of message 

       actual length of message is returned in data[1], 2nd byte of dataframe  
       expected length of message is defined in structure Ckey c 

	   as response length for command 1 can have 3 different values
	   depending on state of weatherstation c.clen serves as an 
           upper bound	   
    */        
    syslog(LOG_DEBUG, "chkframe : response length: %d\n", data[1]);
	
    /* check if NAK dataframe has been received */
    if ( !strncmp( data, nakfram, *mdat) ) {
	  syslog(LOG_INFO, 
		 "chkframe : NAK received : faulty data reception\n");
	  werrno = ERCPT;
	  return(-1);
    }
    *mdat = ldat;
    return(0);
} 


/* wstat 

return status of weather station
commandword is 5

*/
char *
wstat(unsigned char *data, int mdat ) {
  int i, err;
  time_t statusset_date;
  char frame[MAXMSGLEN] = ""; 
  char sf[3] = "";
  //char *errmsg = 0;
  static char t[MAXBUFF] = "no status available";

  char *s;

  /* empty t */
  t[0] = '\0';

  /* time of statusset
     if DCF synchronized use time delivered by weatherstation
     if not, use localtime
  */
  if ( ws2000station.status.DCFtime != -1 ) {
    statusset_date = ws2000station.status.DCFtime;
  }
  else { 
    syslog(LOG_INFO,
	   "wstat : DCF not synchronized, using localtime for statusset\n");
    err = time(&statusset_date);
    syslog(LOG_DEBUG, 
	   "wstat : present time: %lu (seconds since EPOCH)\n", 
           (long int)statusset_date);
  }

  /* open sqlite db file */
  if ( ( err = sqlite3_open( ws2000station.config.dbfile, &ws2000db))) {
    s = mkmsg2("Failed to open database %s. Error: %s\n", 
	     ws2000station.config.dbfile, sqlite3_errmsg(ws2000db));
    strncat( t, s, strlen(s));
    return(t);
  }
  /* insert statusdata */
  for ( i = 1; i <= 18; i++) {
    if ( data[i-1] != 0 ) {
      statdb( statusset_date, i, data[i-1], ws2000db);
    }
  }
  /* cleanup and close */
  sqlite3_close( ws2000db);
  

  /* status of first 8 temperature/humidity sensors */
  for ( i = 1; i <= 8; i++) {
    ws2000station.sensor[i].status   = data[i-1];
  }
  ws2000station.sensor[9].status   = data[8];  // rain
  ws2000station.sensor[10].status  = data[9];  // wind
  ws2000station.sensor[11].status  = data[10]; // indoor
  /* status of temperature/humidity sensor 9 to 15 */
  for ( i = 12; i <= 18; i++) {
    ws2000station.sensor[i].status   = data[i-1];
  }

  for ( i = 0; i < 18; i++ ) {
    sprintf(sf, "%2d:",i);
    strcat(frame, sf);
  }

  syslog(LOG_DEBUG, "wstat : %s\n", frame);    
  strcpy(frame, "");

  for ( i = 1; i <= 18; i++ ) {
    sprintf(sf, "%2x:",ws2000station.sensor[i].status);
    strcat(frame, sf);
  }
  syslog(LOG_DEBUG, "wstat : %s\n", frame);   

	
  /* interval time */
  ws2000station.status.interval = data [18];
  /* DCF status and synchronicity */
  ws2000station.status.DCFstat = getbits( data[19],0,1);
  ws2000station.status.DCFsync = getbits( data[19],3,1); 

  /* HF / Battery status */
  ws2000station.status.HFstat    = getbits( data[19],1,1);
  ws2000station.status.Battstat  = getbits( data[19],4,1);

  /* number of sensors */
  if ( getbits(data[19],2,1) == 0 ) { 
    ws2000station.status.numsens = 18 - 7  ; 
  }
  else if ( getbits(data[19],2,1) == 1 ) {
    ws2000station.status.numsens = 18; 
    //rw->wstat.nsens = 42;
  }

  /* version number */
  ws2000station.status.version = data[20];

  /* syslog messages */
  syslog(LOG_DEBUG, "wstat : Intervall time [min] : %d\n", data[18]);
  syslog(LOG_DEBUG, "wstat : DCF Status   Bit 0 : %d\n",
	 getbits(data[19],0,1));
  syslog(LOG_DEBUG, "wstat : HF Status    Bit 1 : %d\n",
	 getbits(data[19],1,1));
  syslog(LOG_DEBUG, "wstat : 8/16 Sensor  Bit 2 : %d\n",
	 getbits(data[19],2,1));
  syslog(LOG_DEBUG, "wstat : DCF sync     Bit 3 : %d\n",
	 getbits(data[19],3,1));
  syslog(LOG_DEBUG, "wstat : Battery      Bit 4 : %d\n",
	 getbits(data[19],4,1));
    
  syslog(LOG_DEBUG, "wstat : bit 4 : %d\n", getbits(data[19],4,1));
  syslog(LOG_DEBUG, "wstat : bit 5 : %d\n", getbits(data[19],5,1));
  syslog(LOG_DEBUG, "wstat : bit 6 : %d\n", getbits(data[19],6,1));
  syslog(LOG_DEBUG, "wstat : bit 7 : %d\n", getbits(data[19],7,1));

  syslog(LOG_DEBUG, "wstat: DCF.stat : %d\n", ws2000station.status.DCFstat);
  syslog(LOG_DEBUG, "wstat: DCF.sync : %d\n", ws2000station.status.DCFsync);
  syslog(LOG_DEBUG, "wstat: number sensors : %d\n",
	 ws2000station.status.numsens);
  syslog(LOG_DEBUG, "wstat: version : %x\n", ws2000station.status.version);

  /* fill return buffer */
  s = mkmsg2(
	     "Status\nVersion number\t:\t%x\nInterval time\t:\t%d (min)\n",
	     ws2000station.status.version, 
	     ws2000station.status.interval);
  strncat( t, s, strlen(s));   

  if ( ws2000station.status.DCFstat == 1 ) {
    s = mkmsg2("DCF status\t:\t%d (DCF receiver present)\n", 
	       ws2000station.status.DCFstat);
  }
  else {
    s = mkmsg2("DCF status\t:\t%d (no DCF receiver found)\n", 
	       ws2000station.status.DCFstat);
  }

  strncat(t, s, strlen(s));
	
  if ( ws2000station.status.DCFsync == 1 ) {
    s = mkmsg2("DCF sync.\t:\t%d (DCF synchronized)\n", 
	       ws2000station.status.DCFsync);
  }
  else {
    s = mkmsg2("DCF sync.\t:\t%d (DCF NOT synchronized)\n", 
	       ws2000station.status.DCFsync);
  }
  strcat(t,s);

  if ( ws2000station.status.HFstat == 1 ) {
    s = mkmsg2("HF status\t:\t%d (with HF)\n", 
	       ws2000station.status.HFstat);
  }
  else {
    s = mkmsg2("HF status\t:\t%d (without HF)\n", 
	       ws2000station.status.HFstat);
  }
  strcat(t,s);

  s = mkmsg2("Battery status\t:\t%d\n", 
	     ws2000station.status.Battstat);
  strcat(t,s);
   
  s = mkmsg2("Sensor status\t:\t( %d sensors)\n", 
	     ws2000station.status.numsens);
  strcat(t,s);

  for ( i = 0; i < ws2000station.status.numsens; i++ ) {
    s = mkmsg2("%2d|", i);
    strcat(t,s);  
  }
  strcat(t,"\n");
    
  
  for ( i = 0; i < ws2000station.status.numsens; i++ ) {
    s= mkmsg2("%2x|", ws2000station.sensor[i].status);
    strcat(t,s);
  } 
  strcat(t,"\n");
  return (t);
}



/* dcftime 

   calculate time from weather station response to "poll DCF time"
   returns 
     seconds sinche EPOCH, if DCF is synchronized 
     -1, if DCF not synchronized.
   
*/
time_t dcftime(unsigned char *data, int ndat) {
    int err;
    int tmp1;
    int tmp2;   
	
    char *clk;

    struct tm t;
    time_t ltim, dtim;
	
	
    /* 1st byte : flag : B0-B2 : weeday; B4 : DCF ok */
    tmp1 = getbits(data[0], 2,3);
    tmp2 = getbits(data[0], 4,1);
    syslog(LOG_DEBUG, "dcftime : weekday %d; DCF ok : %d\n",tmp1, tmp2);

    t.tm_wday = tmp1;
    /* return -1 if DCF is not synchronized */
    if ( tmp2 == 0 ) 
	return(-1);

    /* 2nd byte : hour (BCD) : low nibble units, high nibble tens */
    tmp1 = getbits(data[1], 3,4);
    tmp2 = getbits(data[1], 7,4);
    syslog(LOG_DEBUG, "dcftime : hour : %d %d\n", tmp2, tmp1);

    t.tm_hour = tmp1 + 10*tmp2;

    /* 3rd byte : min (BCD) : low nibble units, high nibble tens */
    tmp1 = getbits(data[2], 3,4);
    tmp2 = getbits(data[2], 7,4);
    syslog(LOG_DEBUG, "dcftime : min : %d %d\n", tmp2, tmp1);

    t.tm_min = tmp1 + 10*tmp2;

    /* 4th byte : seconds : binary counter */
    t.tm_sec = data[3];
    syslog(LOG_DEBUG, "dcftime : Seconds : %d\n", data[3]);

    /* 5th byte : day (BCD) : low nibble units, high nibble tens */
    tmp1 = getbits(data[4], 3,4);
    tmp2 = getbits(data[4], 7,4);
    syslog(LOG_DEBUG, "dcftime : day : %d %d\n", tmp2, tmp1);

    t.tm_mday = tmp1 + 10*tmp2;

    /* 6th byte : month (BCD) : low nibble units, high nibble tens */
    tmp1 = getbits(data[5], 3,4);
    tmp2 = getbits(data[5], 7,4);
    syslog(LOG_DEBUG, "dcftime : month : %d %d\n", tmp2, tmp1);

    t.tm_mon = tmp1 + 10*tmp2 - 1;

    /* 7th byte : year (BCD) : low nibble units, high nibble tens */
    tmp1 = getbits(data[6], 3,4);
    tmp2 = getbits(data[6], 7,4);
    syslog(LOG_DEBUG, "dcftime : year : %d %d\n", tmp2, tmp1);

    t.tm_year = tmp1 + 10*tmp2 + 100;

    /* surprise, surprise */
    t.tm_isdst = -1;
	
    /* time conversion */
    tzset();

    err = time(&ltim);
    clk = ctime(&ltim);
    syslog(LOG_DEBUG, "dcftime : local clk : %s\n", clk);
    
    dtim = mktime(&t);
    clk = ctime(&dtim);
    syslog(LOG_DEBUG, "dcftime : dtim : %lu\n", (long unsigned int)dtim);
    syslog(LOG_DEBUG, "dcftime : DCF clk : %s\n", clk);

    return(dtim);
}

/*
  settime - setting time incase DCF time is synchronized

*/
int
settime ( ) {
  struct timezone tz;
  struct timeval tv;

  if ( gettimeofday( &tv, &tz) == -1 )
    return(-1);

  tv.tv_sec  = ws2000station.status.DCFtime;
  tv.tv_usec = 0;

  if ( settimeofday( &tv, &tz) == -1 )
    return(-1);

  return(0);
}



/* datex 

   data extraction
   stores the message datagram of the weather station to structure sens

*/
int 
datex(unsigned char *data, int ndat) {
  int i;
  int j;
  int err;
  int mbit;
  int nbit;
  int Hi, Lo;
  time_t dataset_date;
  long age;
  char *clk;
  char *errmsg = 0;
  float meas_value;
	
  syslog(LOG_DEBUG, "datex : ndat in datex : %d\n", ndat);

  /* block number */
  syslog(LOG_DEBUG, "datex : block NO (byte 1) : %d\n", data[0]);
  syslog(LOG_DEBUG, "datex : block NO (byte 2) : %d\n", data[1]);

  /* age of dataset is stored as minutes up to current time */
  syslog(LOG_DEBUG, "datex : age (byte 1) : %d\n", data[2]);
  syslog(LOG_DEBUG, "datex : age (byte 2) : %d\n", data[3]);
  age = ( data[2] & 0xff )  + ( data[3] << 8 & 0xff00 );


  /* time of dataset
     if DCF synchronized use time delivered by weatherstation
     if not, use localtime
  */
  if ( ws2000station.status.DCFtime != -1 ) {
    dataset_date = ws2000station.status.DCFtime;
  }
  else { 
    syslog(LOG_INFO,
	   "datex : DCF not synchronized, using localtime for dataset\n");
    err = time(&dataset_date);
    syslog(LOG_DEBUG, 
	   "datex : present time: %lu (seconds since EPOCH)\n", 
           (long int)dataset_date);
  }
  
  /* dataset time is echoed in EPOCH seconds, dataset age in minutes
     up to present time
  */
  dataset_date = ( dataset_date/60 - age) * 60;
  clk  = ctime(&dataset_date);


  syslog(LOG_DEBUG, "datex : ws2000station.status.ndats : %d\n",
	 ws2000station.status.ndats);
  syslog(LOG_DEBUG, "datex : measured at : %lu (seconds since EPOCH)\n",
	 (long int)dataset_date);
  syslog(LOG_DEBUG, "datex : measured at : %s\n", clk);
  syslog(LOG_DEBUG, "datex : units : %s\n", wsconf.units);

  
  /* open sqlite db file */
  err = sqlite3_open( ws2000station.config.dbfile, &ws2000db);
  printf("err: %d : sqlite_errmsg: %s\n", err, sqlite3_errmsg(ws2000db));
  if ( err) {
    fprintf( stderr, "Failed to open database %s. Error: %s\n", 
	     ws2000station.config.dbfile, sqlite3_errmsg(ws2000db));
    free( errmsg);
    return -1;
  } else {
    printf("sqlite3_open: no error: OK\n");
  }


  /* get data of the first 8 temperature/humidity sensors */
  for ( i = 1; i <= 8; i++) {
    if ( ws2000station.sensor[i].status != 0 ) {
    j = (i-1) % 2;
    /* even i */
    if ( ((i-1) % 2) == 0 ) {
      mbit=3; nbit=7;
      j = (5*(i-1) - j)/2 + 4;

      /* temperature */ 
      meas_value =  
        10 * getbits(data[j+1], mbit-1, 3) + 
        getbits(data[j], nbit, 4) +
        0.1* getbits(data[j], mbit, 4);
      if ( getbits(data[j+1], mbit, 1) == 1 ) {  
        meas_value = -meas_value; 
      }
      printf("ws2000station.sensor[%d].status: %d\n", i, ws2000station.sensor[i].status);
      printf("Temperature: i: %d dataset_date: %lu meas_value: %f\n", 
        i, (long int)dataset_date, meas_value);
      printf("j+1: %d mbit-1: %d j: %d nbit: %d\n", j+1, mbit-1, j, nbit);

      /* humidity 
	 the value is calculated from an 8bit Byte which is formed
	 by two Nibbles    
      */
      Lo = getbits(data[j+1], nbit, 4);
      Hi = getbits(data[j+2], mbit-1, 3) << 4;
      meas_value = Hi + Lo;
      printf("ws2000station.sensor[%d].status: %d\n", i, ws2000station.sensor[i].status);
      printf("Humidity: i: %d dataset_date: %lu meas_value: %f\n", 
        i, (long int)dataset_date, meas_value);
      printf("j+1: %d mbit-1: %d j: %d nbit: %d\n", j+1, mbit-1, j, nbit);


      /* Bit 3 of Hi Nibble is new flag */
      //rw->sens[2*i+1].mess[rw->wstat.ndats].sign  = 
      //getbits(data[j+2], mbit, 1);
    } /* odd i */ else if ( ((i-1) % 2) == 1) {
      mbit=7; nbit=3;
      j = (5*(i-1) - j)/2 + 4;

      /* temperature */
      meas_value =
        10  * getbits(data[j+1], mbit-1, 3) +
        getbits(data[j+1], nbit, 4) +
        0.1 * getbits(data[j], mbit, 4);
      if (  getbits(data[j+1], mbit, 1) == 1) {
        meas_value = - meas_value;
      }

      printf("ws2000station.sensor[%d].status: %d\n", i, ws2000station.sensor[i].status);
      printf("Temperature: i: %d dataset_date: %lu meas_value: %f\n", 
        i, (long int)dataset_date, meas_value);
      printf("j+1: %d mbit-1: %d j: %d nbit: %d\n", j+1, mbit-1, j, nbit);

      /* humidity 
	 the value is calculated from an 8bit Byte which is formed
	 by two Nibbles    
      */
      Lo = getbits(data[j+2], nbit, 4);
      Hi = getbits(data[j+2], mbit-1, 3) << 4;
      meas_value = Hi + Lo;
      printf("ws2000station.sensor[%d].status: %d\n", i, ws2000station.sensor[i].status);
      printf("Humidity: i: %d dataset_date: %lu meas_value: %f\n", 
        i, (long int)dataset_date, meas_value);
      printf("j+1: %d mbit-1: %d j: %d nbit: %d\n", j+1, mbit-1, j, nbit);
	/* Bit 3 of Hi Nibble is new flag */
	//rw->sens[2*i+1].mess[rw->wstat.ndats].sign = 
      //getbits(data[j+2], mbit, 1);
    }
    } else {
      printf("Sensor  #%d: Temperature/Humiditysensor not found\n", i);
    }
  }

  /* Sensor #9: Rainsensor */
  if ( ws2000station.sensor[9].status != 0) {
    Hi = getbits(data[25], 6, 7) << 8 ;
    Lo = getbits(data[24], 7, 8);
    meas_value   = Hi + Lo;
    datadb( dataset_date, 17, meas_value, ws2000db);
    printf("Rainsensor:\t\tdataset_date: %lu meas_value: %f\n", 
      (long int)dataset_date, meas_value);
    // rain new flag
    //rw->sens[16].mess[rw->wstat.ndats].sign      = getbits(data[25], 7, 1);
  } else {
    printf("Sensor  #9: Rainsensor not found\n");
  }

  /* Sensor #10: Windsensor */
  if ( ws2000station.sensor[10].status != 0) {
  /*wind speed */
  meas_value = 
    100 * getbits(data[27], 6, 3) +
    10  * getbits(data[27], 3, 4) +
    getbits(data[26], 6, 3) +
    0.1 * getbits(data[26], 3, 4);
  datadb( dataset_date, 18, meas_value, ws2000db);
  printf("Windsensor(Windspeed):\tdataset_date: %lu meas_value: %f\n", 
    (long int)dataset_date, meas_value);


  /* wind direction */
  meas_value =
    100 * getbits( data[29], 1, 2 ) +
    10  * getbits(data[28], 7, 4 ) +
    getbits(data[28], 3, 4 );
  datadb( dataset_date, 19, meas_value, ws2000db);
  printf("Windsensor(Winddirection):\tdataset_date: %lu meas_value: %f\n", 
    (long int)dataset_date, meas_value);

  /* mean deviation of wind direction */
  meas_value =
    getbits( data[29], 4, 2 );
  datadb( dataset_date, 20, meas_value, ws2000db);
  printf("Windsensor(Variation):\tdataset_date: %lu meas_value: %f\n", 
    (long int)dataset_date, meas_value);
  } else {
    printf("Sensor #10: Windsensor not found\n");
  } 

  /* Sensor #11: Indoorsensor */
  if ( ws2000station.sensor[11].status != 0) {
  /* barometric pressure */
  meas_value = 
    100 *  getbits(data[30], 7, 4) +
    10  *  getbits(data[30], 3, 4) +
    getbits(data[29], 7, 4) + 200;
  printf("Indoorsensor(Pressure):\t\tdataset_date: %lu meas_value: %f\n", 
    (long int)dataset_date, meas_value);
  datadb( dataset_date, 21, meas_value, ws2000db);

  /* indoor temperature */
  meas_value = 
    10  * getbits(data[32], 3, 4) + 
    getbits(data[31], 7, 4) +
    0.1 * getbits(data[31], 7, 4);
  printf("Indoorsensor(Temperature):\tdataset_date: %lu meas_value: %f\n", 
    (long int)dataset_date, meas_value);
  datadb( dataset_date, 22, meas_value, ws2000db);

  /* indoor humidity */
  Lo = getbits(data[32], 7, 4);
  Hi = getbits(data[33], 2, 3) << 4;
  meas_value = Hi + Lo;
  printf("Indoorsensor(Humidity):\t\tdataset_date: %lu meas_value: %f\n", 
    (long int)dataset_date, meas_value);
  datadb( dataset_date, 23, meas_value, ws2000db);

  } else {
    printf("Sensor #11: Indoorsensor not found\n");
  }
  /* cleanup and close */
  sqlite3_close( ws2000db);
  printf("WS2000 sqlite done\n");
	
  return(0);
};




/* wstrlen

   Kerninghan, Ritchie, The C Programming Language, p103

*/
int
wstrlen ( unsigned char *s) {
    unsigned char *p = s;

    while ( *p != ETX ) p++;

    return p + 1 - s;    
}


int
bitstat( int byte, char *s_reg  ) 
{
  int x;
  struct timezone tz;
  struct timeval  tv;

  gettimeofday(&tv, &tz);
  printf("%5s | %lu.%lu : #", s_reg, tv.tv_sec, tv.tv_usec);
  for( x = 7; x > -1; x-- )
    printf( "%i", (byte & 1 << x ) > 0 ? 1 : 0 );
  printf( "b\n" );
  return(0);
}


/*  initws2000

  opens serial port for communication
  
  serial port settings:
  WS2000 (and similar stations)
     9600, 8, Even Parity,  2Stop
     lower RTS and raise DTR voltage

  PCWSR (PC Weatherstation receiver by ELV)
     19200 BAUD, 8, Odd Parity, 2 Stopp

*/
int 
initws2000 (int *pfd, struct termios *newtio,
	    struct termios *oldtio) 
{
  int i, itio;

  /* open the device to be non-blocking (read will return immediatly) */

  *pfd = open(ws2000station.config.device, O_RDWR| O_NOCTTY| O_NDELAY| O_NONBLOCK );
  if ( *pfd <0) {
    werrno = errno;
    syslog(LOG_INFO, "initserial: error opening serial device : %s",
	   strerror(werrno));
    return(-1);
  }

  /* is this asynchronous? probably not */
  if ( fcntl(*pfd, F_SETFL, 0) == -1 ) {
    werrno = errno;
    syslog(LOG_INFO, "initserial: error fcntl: %s",
	   strerror(werrno));
    return(-1);	
  }

  /* save current port settings */
  if ( tcgetattr(*pfd,oldtio) == -1 ) {  
    werrno = errno;
    syslog(LOG_INFO, "initserial: error tcgetattr oldtio: %s",
	   strerror(werrno));
    return(-1);	
  }

  /* prefill port settings */
  if ( tcgetattr(*pfd,newtio) == -1 ) { 
    werrno = errno;
    syslog(LOG_INFO, "initserial: error tcgetattr newtio: %s",
	   strerror(werrno));
    return(-1);	
  }
  
  /* set communication link parameters */
  if ( cfsetispeed(newtio, B9600) == -1 ) {
    werrno = errno;
    syslog(LOG_INFO, "initserial: error cfsetispeed: %s",
	   strerror(werrno));
    return(-1);	
  }
  
  if ( cfsetospeed(newtio, B9600) == -1 ) {
    werrno = errno;
    syslog(LOG_INFO, "initserial: error cfsetospeed: %s",
	   strerror(werrno));
    return(-1);	
  }


  newtio->c_iflag = 0;
  newtio->c_oflag = 0;
  newtio->c_cflag = 0;
  newtio->c_lflag = 0;

  for ( i = 0; i < NCCS; i++) {
    newtio->c_cc[i] = 0;
  }
  
  
  /* newtio->c_iflag &= (IXON | IXOFF | IXANY);  */  
  newtio->c_cflag |= PARENB;
  newtio->c_cflag &= ~PARODD;
  newtio->c_cflag |= CLOCAL;
  newtio->c_cflag |= CREAD;
  newtio->c_cflag |= CS8;
  newtio->c_cflag |= CSTOPB;
  newtio->c_cflag |= CSIZE; 
  
  /* newtio->c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); */
  newtio->c_cc[VMIN]=0;
  newtio->c_cc[VTIME]=10;

  if ( tcsetattr(*pfd,TCSANOW,newtio) == -1 ) {
    werrno = errno;
    syslog(LOG_INFO, "initserial: error tcsetattr: %s",
	   strerror(werrno));
    return(-1);	
  }

  /* lower DTR and RTS on serial line */
  if ( ioctl(*pfd, TIOCMGET, &itio) == -1 ) {
    werrno = errno;
    syslog(LOG_INFO, "initserial: error ioctl: %s",
	   strerror(werrno));
    return(-1);	
  }
  itio &= ~TIOCM_DTR;
  itio &= ~TIOCM_RTS;
  if ( ioctl(*pfd, TIOCMSET, &itio) == -1 ) {
    errno = errno;
    syslog(LOG_INFO, "initserial: error ioctl: %s",
	   strerror(werrno));
    return(-1);	
  }

  /* raise DTR */
  itio |= TIOCM_DTR;
  if ( ioctl(*pfd, TIOCMSET, &itio) == -1 ) {
    werrno = errno;
    syslog(LOG_INFO, "initserial: error ioctl: %s",
	   strerror(werrno));
    return(-1);	
  }
  return(0);
}

/*
   closeserial

   restore the old port settings
   lower DTR on serial line

*/
int closeserial( int fd, struct termios *oldtio) {
    int tset;

    /* lower DTR on serial line */
    if ( ioctl(fd, TIOCMGET, &tset) == -1 ) {
	  werrno = errno;
	  syslog(LOG_INFO, "closeserial: error ioctl: %s",
			 strerror(werrno));
	  return(-1);	
	}
    tset &= ~TIOCM_DTR;
    if ( ioctl(fd, TIOCMSET, &tset) == -1 ) {
      werrno = errno;
      syslog(LOG_INFO, "closeserial: error ioctl: %s",
	     strerror(werrno));
      return(-1);	
    }

    /* restore old port settings */
    /* takes approx 3 sec to reopen fd under FreeBSD
       if tcsetattr is called to restore */
    if ( tcsetattr(fd,TCSANOW,oldtio) == -1 ) {
	  werrno = errno;
	  syslog(LOG_INFO, "closeserial: error ioctl: %s",
			 strerror(werrno));
	  return(-1);	
	}
	
    /* close serial port */
    if ( close(fd) == -1 ) {
	  werrno = errno;
	  syslog(LOG_INFO, "closeserial: error close: %s",
			 strerror(werrno));
	  return(-1);	
    }
    return(0);
}


/*
  readdata

  reading data from serial port
  signal SIGIO is sent by the weather station when data are available
  huh - the weather station sends its data frame in chunks of several bytes   

*/
int 
readdata (int fd, unsigned char *data, int *ndat) 
{
  int i,n;
  int err;
  static unsigned char sbuf[MAXBUFF] = "no data available";
  int maxfd;
  int loop = 1;
  fd_set readfs;
  struct timeval tv;

  *ndat = 0;
	
  err = -1;
  maxfd = fd +1;
 
  syslog(LOG_INFO,"readdata: begin of exec"); 
  /* loop until input is available */
  while ( loop) {
	
	FD_ZERO(&readfs);
	FD_SET(fd, &readfs);

	tv.tv_sec  = wsconf.timeout;
	tv.tv_usec = 0;
	
	n = select(maxfd, &readfs, NULL, NULL, &tv);
	switch (n) {
	case -1:
          if ( errno == EBADF ) 
	    syslog(LOG_INFO, "readdata: select error: EBADF");
          else if ( errno == EINTR ) 
	    syslog(LOG_INFO, "readdata: select error: EINTR");
          else if ( errno == EINVAL ) 
	    syslog(LOG_INFO, "readdata: select error: EINVAL");
          else
	    syslog(LOG_INFO, "readdata: select error");
          err = -1; loop = 0;
	  break;
	case 0:
	  syslog(LOG_INFO, "readdata: select: Timeout\n");
          err = -1; loop = 0;
          break;
	default:
	  /* loop until err = 0 */
	  while ( ( sbuf[err-1] != 3) && ( err != 0)) {
	    err = read(fd, sbuf, MAXBUFF);
	    for ( i = 0; i < err; i++) {
              data[*ndat + i] = sbuf[i];
	    }
            *ndat += err;
	    loop=0;
	  }
	  break;
	}
  }
  return(err);
}


/*  c
    
    returns 
    communication string of command 
    expected response message length 

    cmd : description                     : response length (bytes)
    0   : Poll DCF Time                   : 7
    1   : Request Dataset                 : 35/61 (violation of protocol spec)
    2   : Select Next Dataset             : 1
    3   : Activate 9 temperature sensors  : 1
    4   : Activate 16 temperature sensors : 1
    5   : Request status                  : 21
    6   : Set interval time               : 1
    shouldn't this go into a configuration file?
 */
ws2000key_t *c(int n){
  static ws2000key_t p[] = {
	{"\x01\x30\xcf\x04", 7, "Poll DCF time"},
	{"\x01\x31\xce\x04", 61, "Request dataset"},
	{"\x01\x32\xcd\x04", 1, "Select next dataset"},
	{"\x01\x33\xcc\x04", 1, "Activate 9 temperature sensors"},
	{"\x01\x34\xcb\x04", 1, "Activate 16 temperature sensors"},
	{"\x01\x35\xca\x04", 21, "Request status"},
	{"\x01\x36\x53\xc9\x04", 1, "Set interval time"},
	{"", 0, "Unused"},
	{"", 0, "Unused"},
	{"", 0, "Unused"},
	{"", 0, "Unused"},
	{"", 0, "Unused"},
	{"", 0, "Recursive data request"},
  };
  return p+n;
  
};



/* getsrd 

   get raw data from serial interface
*/
int getsrd ( unsigned char *data, int *mdat) {
    int fd;                         /* filedescriptor serial port */
    char lword[5];                  /* temporary array to hold commandword */
    struct termios newtio,oldtio;   /* termios structures to set 
                                       comm parameters */

    /* initialize serial port */    
    if ( initws2000(&fd, &newtio, &oldtio) == -1 )
      return(-1);

    /* read ETX from serial port
 
    when the weather station is activated by setting DTR it answers
    with an ETX char to signalize it's OK 
    */
    if ( readdata(fd, data, mdat) == -1 ) {
      printf("readdata problem\n");
      closeserial(fd, &oldtio);
      return(-1);
    }

    if ( strcpy(lword, c(wsconf.command)->word) == NULL )
      return(-1); 
	
    /* modify command to set intervall time */
    if ( wsconf.command == 6) {
      lword[2] = wsconf.argcmd;
      lword[3] = lword[3] - wsconf.argcmd;
    }

    /* write to command to serial line */
    if ( write(fd, lword, sizeof(lword)) == -1 ) {
      werrno = errno;
      syslog(LOG_INFO, "getsrd: write failed: %s",
	     strerror(werrno));
      return(-1);
    }

    syslog(LOG_DEBUG, "getsrd : command written : %d\n", wsconf.command);

    /* read weather station resonse from serial port */
    if ( readdata(fd, data, mdat) == -1) {
      closeserial(fd, &oldtio);
      return(-1);
    }
    
    /* echo raw dataframe */
    if ( echodata( data, *mdat) == -1) {
    closeserial(fd, &oldtio);
      return(-1);
    }
    
    closeserial(fd, &oldtio);
    
    syslog(LOG_DEBUG, "getsrd : Data length getsrd : %d\n", *mdat);
    return(0);
}
