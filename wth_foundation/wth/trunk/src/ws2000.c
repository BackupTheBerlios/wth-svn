/* ws2000.c

   ws2000 handler implemented as POSIX thread

   $Id: ws2000.c 348 2012-10-11 13:43:22Z vjahns $
   $Revision$

   Copyright (C) 2001-2004,2007,2008 Volker Jahns <volker@thalreit.de>

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

#define MAXWAIT         10
#define DATAWAIT        300
#define MAXBUFF         131072
#define MAXSENSORS      24
#define WS2000LOCK      "/tmp/LCK...wth"

char *lockfile = WS2000LOCK;

/* sensor parameter assignment */
enum {
  SENSOR1      = 1,
  SENSOR2      = 2,
  SENSOR3      = 3,
  SENSOR4      = 4,
  SENSOR5      = 5,
  SENSOR6      = 6,
  SENSOR7      = 7,
  SENSOR8      = 8,
  RAINSENSOR   = 9,
  WINDSENSOR   = 10,
  INDOORSENSOR = 11,
  SENSOR9      = 12,
  SENSOR10     = 13,
  SENSOR11     = 14,
  SENSOR12     = 15,
  SENSOR13     = 16,
  SENSOR14     = 17,
  SENSOR15     = 18
};
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
	  syslog(LOG_ALERT, "closeserial: error ioctl: %s",
			 strerror(werrno));
	  return(-1);	
	}
    tset &= ~TIOCM_DTR;
    if ( ioctl(fd, TIOCMSET, &tset) == -1 ) {
      werrno = errno;
      syslog(LOG_ALERT, "closeserial: error ioctl: %s",
	     strerror(werrno));
      return(-1);	
    }

    /* restore old port settings */
    /* takes approx 3 sec to reopen fd under FreeBSD
       if tcsetattr is called to restore */
    if ( tcsetattr(fd,TCSANOW,oldtio) == -1 ) {
	  werrno = errno;
	  syslog(LOG_ALERT, "closeserial: error ioctl: %s",
			 strerror(werrno));
	  return(-1);	
	}
	
    /* close serial port */
    if ( close(fd) == -1 ) {
	  werrno = errno;
	  syslog(LOG_ALERT, "closeserial: error close: %s",
			 strerror(werrno));
	  return(-1);	
    }
    return(0);
}


/*
  readdata

  reading data from serial port
  huh - the weather station sends its data frame in 
        chunks of several bytes   

*/
int 
readdata (int fd, unsigned char *data, int *ndat) 
{
  int i;
  int n = 0;
  int err = -1;
  static unsigned char sbuf[MAXBUFF+1];
  int maxfd;
  int loop = 1;
  fd_set readfs;
  struct timeval tv;

  *ndat = 0;
	
  maxfd = fd +1;
  memset( sbuf, 0 , MAXBUFF);
 
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
	    syslog(LOG_CRIT, "readdata: select error: EBADF");
          else if ( errno == EINTR ) 
	    syslog(LOG_CRIT, "readdata: select error: EINTR");
          else if ( errno == EINVAL ) 
	    syslog(LOG_CRIT, "readdata: select error: EINVAL");
          else
	    syslog(LOG_CRIT, "readdata: select error");
          err = -1; loop = 0;
	  break;
	case 0:
	  syslog(LOG_CRIT, "readdata: select: Timeout\n");
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


/* wstrlen

   Kerninghan, Ritchie, The C Programming Language, p103

*/
int
wstrlen ( unsigned char *s) {
    unsigned char *p = s;
    while ( *p != ETX ) p++;
    return p + 1 - s;    
}


char *
bitstat( int byte  ) 
{
  int x;
  static char bbuf[MAXMSGLEN+1];

  for( x = 7; x > -1; x-- )
    snprintf( bbuf, MAXMSGLEN, "%i", (byte & 1 << x ) > 0 ? 1 : 0 );

  snprintf( bbuf,MAXMSGLEN, "b\n" );
  return(bbuf);
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
    syslog(LOG_ALERT, "initserial: error opening serial device : %s",
	   strerror(werrno));
    return(-1);
  }

  /* is this asynchronous? probably not */
  if ( fcntl(*pfd, F_SETFL, 0) == -1 ) {
    werrno = errno;
    syslog(LOG_ALERT, "initserial: error fcntl: %s",
	   strerror(werrno));
    return(-1);	
  }

  /* save current port settings */
  if ( tcgetattr(*pfd,oldtio) == -1 ) {  
    werrno = errno;
    syslog(LOG_ALERT, "initserial: error tcgetattr oldtio: %s",
	   strerror(werrno));
    return(-1);	
  }

  /* prefill port settings */
  if ( tcgetattr(*pfd,newtio) == -1 ) { 
    werrno = errno;
    syslog(LOG_ALERT, "initserial: error tcgetattr newtio: %s",
	   strerror(werrno));
    return(-1);	
  }
  
  /* set communication link parameters */
  if ( cfsetispeed(newtio, B9600) == -1 ) {
    werrno = errno;
    syslog(LOG_ALERT, "initserial: error cfsetispeed: %s",
	   strerror(werrno));
    return(-1);	
  }
  
  if ( cfsetospeed(newtio, B9600) == -1 ) {
    werrno = errno;
    syslog(LOG_ALERT, "initserial: error cfsetospeed: %s",
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
    syslog(LOG_ALERT, "initserial: error tcsetattr: %s",
	   strerror(werrno));
    return(-1);	
  }

  /* lower DTR and RTS on serial line */
  if ( ioctl(*pfd, TIOCMGET, &itio) == -1 ) {
    werrno = errno;
    syslog(LOG_ALERT, "initserial: error ioctl: %s",
	   strerror(werrno));
    return(-1);	
  }
  itio &= ~TIOCM_DTR;
  itio &= ~TIOCM_RTS;
  if ( ioctl(*pfd, TIOCMSET, &itio) == -1 ) {
    errno = errno;
    syslog(LOG_ALERT, "initserial: error ioctl: %s",
	   strerror(werrno));
    return(-1);	
  }

  /* raise DTR */
  itio |= TIOCM_DTR;
  if ( ioctl(*pfd, TIOCMSET, &itio) == -1 ) {
    werrno = errno;
    syslog(LOG_ALERT, "initserial: error ioctl: %s",
	   strerror(werrno));
    return(-1);	
  }
  return(0);
}


/* dcftime 

   calculate time from weather station response to "poll DCF time"
   returns 
     seconds sinche EPOCH, if DCF is synchronized 
     -1, if DCF not synchronized.
   
*/
time_t 
dcftime(unsigned char *data, int ndat) {
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
    syslog(LOG_DEBUG, "dcftime : year : %d %d", tmp2, tmp1);

    t.tm_year = tmp1 + 10*tmp2 + 100;
    t.tm_isdst = -1;
    tzset();

    err = time(&ltim);
    clk = ctime(&ltim);
    syslog(LOG_DEBUG, "dcftime : local clk : %s", clk);
    
    dtim = mktime(&t);
    clk = ctime(&dtim);
    syslog(LOG_DEBUG, "dcftime : dtim : %lu", (long unsigned int)dtim);
    syslog(LOG_DEBUG, "dcftime : DCF clk : %s", clk);

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



/* getrd 

   get raw data from serial interface
*/
int getrd ( unsigned char *data, int *mdat) {
    int fd;                        /* filedescriptor serial port */
    static char lword[6];          /* temporary array to hold commandword */
    struct termios newtio,oldtio;  /* termios structures to set 
                                      comm parameters */

    memset( lword, 0, 5);
    /* initialize serial port */    
    if ( initws2000(&fd, &newtio, &oldtio) == -1 )
      return(-1);

    /* read ETX from serial port
 
    when the weather station is activated by setting DTR it answers
    with an ETX char as OK response
    */
    if ( readdata(fd, data, mdat) == -1 ) {
      syslog(LOG_ALERT, "getsrd: error: readdata problem\n");
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
      syslog(LOG_ALERT, "getsrd: write failed: %s",
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
    /*
    if ( echodata( data, *mdat) == -1) {
      closeserial(fd, &oldtio);
      return(-1);
    }
    */
    closeserial(fd, &oldtio);
    
    syslog(LOG_DEBUG, "getsrd : Data length getsrd : %d\n", *mdat);
    return(0);
}



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
    char nakfram[TBUFF] = "\x02\x01\x15\xe8\x03";

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
    if ( !strncmp( (char *)data, nakfram, strlen(nakfram)) ) {
	  syslog(LOG_WARNING, 
		 "chkframe : NAK received : faulty data reception\n");
	  werrno = ERCPT;
	  return(-1);
    }
    *mdat = ldat;
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
    //err = echodata(data, *mdat);

    /* extract message: eliminiate first and last 2 bytes */
    for ( i = 0; i < *mdat - 4; i++ ) {
        data[i] = data[i+2];
    }
    *mdat = *mdat - 4;

    syslog(LOG_DEBUG, "getcd : Data length getcd : %d\n", *mdat);

    return(0);
}




/* wstat 

return status of weather station
commandword is 5

*/
char *
wstat(unsigned char *data, int mdat ) {
  int i, err;
  int sdata[MAXSENSORS];
  time_t statusset_date;
  //char frame[MAXMSGLEN+1] = ""; 
  static char t[NBUFF+1];
  char s[TBUFF+1];

  /* time of statusset
     if DCF synchronized use time delivered by weatherstation
     if not, use localtime
  */
  if ( ws2000station.status.DCFtime != -1 ) {
    syslog(LOG_DEBUG, "wstat: DCF synchronized: statusset_date: %lu", (long int)ws2000station.status.DCFtime);
    statusset_date = ws2000station.status.DCFtime;
  } else { 
    syslog(LOG_INFO,
	   "wstat : DCF not synchronized, using localtime for statusset\n");
    err = time(&statusset_date);
    syslog(LOG_DEBUG, 
	   "wstat : statuset_date: %lu (seconds since EPOCH)\n", 
           (long int)statusset_date);
  }

  /* insert statusdata */
  for ( i = 1; i <= 18; i++) { sdata[i] = data[i-1]; }

  err = stat_ws2000db( sdata, statusset_date, ws2000station.config.dbtype);

  if ( err) {
    syslog(LOG_ALERT,
      "wstat: error: writing sensor status to database");
  }	
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
  snprintf( t, TBUFF,
	     "Status\nVersion number\t:\t%x\nInterval time\t:\t%d (min)\n",
	     ws2000station.status.version, 
	     (int)ws2000station.status.interval);

  if ( ws2000station.status.DCFstat == 1 ) {

    snprintf( s, TBUFF, "DCF status\t:\t%d (DCF receiver present)\n", 
	       ws2000station.status.DCFstat);
  }
  else {
    snprintf( s, TBUFF, "DCF status\t:\t%d (no DCF receiver found)\n", 
	       ws2000station.status.DCFstat);
  }

  strncat(t, s, strlen(s));
	
  if ( ws2000station.status.DCFsync == 1 ) {
    snprintf( s, TBUFF, "DCF sync.\t:\t%d (DCF synchronized)\n", 
	       ws2000station.status.DCFsync);
  }
  else {
    snprintf( s, TBUFF, "DCF sync.\t:\t%d (DCF NOT synchronized)\n", 
	       ws2000station.status.DCFsync);
  }
  strcat(t,s);

  if ( ws2000station.status.HFstat == 1 ) {
    snprintf( s, TBUFF, "HF status\t:\t%d (with HF)\n", 
	       ws2000station.status.HFstat);
  }
  else {
    snprintf (s , TBUFF, "HF status\t:\t%d (without HF)\n", 
	       ws2000station.status.HFstat);
  }
  strcat(t,s);

  snprintf(s, TBUFF, "Battery status\t:\t%d\n", 
	     ws2000station.status.Battstat);
  strcat(t,s);
   
  snprintf(s, TBUFF, "Sensor status\t:\t( %d sensors)\n", 
	     ws2000station.status.numsens);
  strcat(t,s);

  for ( i = 1; i <= ws2000station.status.numsens; i++ ) {
    snprintf(s, TBUFF,"%2d|", i);
    strcat(t,s);  
  }
  strcat(t,"\n");
  
  return (t);
}

ws2000mval_t
get_ws2000val( unsigned char *data, int sensor_no) {
  int j;
  int mbit, nbit;
  int Hi, Lo;
  ws2000mval_t ws2000val;

  j = (sensor_no-1) % 2;
  /* even sensor_no */
  if ( ((sensor_no-1) % 2) == 0 ) {
    mbit=3; nbit=7;
    j = (5*(sensor_no-1) - j)/2 + 4;

    /* temperature */
    ws2000val.temperature =
      10 * getbits(data[j+1], mbit-1, 3) + getbits(data[j], nbit, 4) +
      0.1* getbits(data[j], mbit, 4);
    if ( getbits(data[j+1], mbit, 1) == 1 ) {
      ws2000val.temperature = -ws2000val.temperature;
    }
    /* humidity 
       the value is calculated from an 8bit Byte which is formed
       by two Nibbles    
    */
    Lo = getbits(data[j+1], nbit, 4);
    Hi = getbits(data[j+2], mbit-1, 3) << 4;
    ws2000val.humidity = Hi + Lo;

    /* bit 3 of Hi nibble is new flag */
    ws2000val.new = getbits(data[j+2], mbit, 1);

    syslog(LOG_DEBUG, "get_ws2000val: sensor #%d humidity: Hi: %x(h) Lo: %x(h)\n",
           sensor_no, Hi, Lo);
  } /* odd i */ else if ( ((sensor_no-1) % 2) == 1) {
    mbit=7; nbit=3;
    j = (5*(sensor_no-1) - j)/2 + 4;

    /* temperature */
    ws2000val.temperature =
      10  * getbits(data[j+1], mbit-1, 3) + getbits(data[j+1], nbit, 4) +
      0.1 * getbits(data[j], mbit, 4);
    if (  getbits(data[j+1], mbit, 1) == 1) {
      ws2000val.temperature = -ws2000val.temperature;
    }
    /* humidity 
       the value is calculated from an 8bit Byte which is formed
       by two Nibbles    
    */
    Lo = getbits(data[j+2], nbit, 4);
    Hi = getbits(data[j+2], mbit-1, 3) << 4;
    ws2000val.humidity = Hi + Lo;
    syslog(LOG_DEBUG, "get_ws2000val: sensor #%d humidity: Hi: %x(h) Lo: %x(h)\n",
           sensor_no, Hi, Lo);
    /* Bit 3 of Hi Nibble is new flag */
    ws2000val.new =
      getbits(data[j+2], mbit, 1);
  }
  return(ws2000val);
}

/* datexn

   data extraction - cleaned version
   stores the message datagram of the weather station to structure sens

*/
int 
datexn ( unsigned char *data, int ndat) {
  int err; 
  int Hi, Lo, new;
  time_t dataset_date;
  long age;
  char *clk;
  char tstrg[TBUFF+1];
  float meas_value[3];
  ws2000mval_t ws2000mval;
  float rainfall;
  float windspeed, winddirection, winddeviation;
  float temperature, humidity, pressure;
	

  /* block number */
  syslog(LOG_DEBUG, "datexn: block NO (byte 1) : %d\n", data[0]);
  syslog(LOG_DEBUG, "datexn: block NO (byte 2) : %d\n", data[1]);

  /* age of dataset is stored as minutes up to current time */
  syslog(LOG_DEBUG, "datexn: age (byte 1) : %d\n", data[2]);
  syslog(LOG_DEBUG, "datexn: age (byte 2) : %d\n", data[3]);
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
	   "datexn: DCF not synchronized, using localtime for dataset\n");
    err = time(&dataset_date);
    syslog(LOG_DEBUG, 
	   "datexn: present time: %lu (seconds since EPOCH)\n", 
           (long int)dataset_date);
  }
  
  /* dataset time is echoed in EPOCH seconds, dataset age in minutes
     up to present time
  */
  dataset_date = ( dataset_date/60 - age) * 60;
  clk  = ctime(&dataset_date);
  snprintf(tstrg,MAXMSGLEN, "%lu", (long)dataset_date);

  syslog(LOG_DEBUG, "datexn: ws2000station.status.ndats : %d\n",
	 ws2000station.status.ndats);
  syslog(LOG_DEBUG, "datexn: measured at : %lu (seconds since EPOCH)\n",
	 (long int)dataset_date);
  syslog(LOG_DEBUG, "datexn: measured at : %s\n", clk);
  syslog(LOG_DEBUG, "datexn: units : %s\n", wsconf.units);


  /* SENSOR1  Temperature/Humidity */
  err = is_ws2000sens( SENSOR1, ws2000station.config.dbtype);
  if ( err != 0 ) {
    ws2000mval = get_ws2000val( data, SENSOR1);
    syslog(LOG_DEBUG,
      "datexn: sensor #%d dataset_date: %lu "
      "temperature: %f humidity: %f new: %d\n",
      SENSOR1, 
      (long int)dataset_date, 
      ws2000mval.temperature,
      ws2000mval.humidity,
      ws2000mval.new);
    err = new_ws2000db( SENSOR1, ws2000mval.new, ws2000station.config.dbtype);
    if ( err) {
      syslog(LOG_ALERT,"datexn: error: SENSOR1 new flag configuration problem");
    }
    err = measval_hd( "sensor1",
                      "temperature",
                      WS2000,
                      ws2000station.config.dbtype,
                      dataset_date, 
                      ws2000mval.temperature);
    if ( err != 0 ) {
      syslog(LOG_ALERT,"datexn: error: SENSOR1 temperature : check configuration");
    }
    err = measval_hd( "sensor1",
                      "humidity",
                      WS2000,
                      ws2000station.config.dbtype,
                      dataset_date, 
                      ws2000mval.humidity);
    if ( err != 0 ) {
      syslog(LOG_ALERT,"datexn: error: SENSOR1 humidity : check configuration");
    }
  } else {
    syslog(LOG_DEBUG,"datexn: SENSOR1 temperature/humidity not found");
  }


  /* SENSOR2  Temperature/Humidity */
  err = is_ws2000sens( SENSOR2, ws2000station.config.dbtype);
  if ( err != 0 ) {
    ws2000mval = get_ws2000val( data, SENSOR2);
    syslog(LOG_DEBUG,
      "datexn: sensor #%d dataset_date: %lu "
      "temperature: %f humidity: %f new: %d\n",
      SENSOR2, 
      (long int)dataset_date, 
      ws2000mval.temperature,
      ws2000mval.humidity,
      ws2000mval.new);
    err = new_ws2000db( SENSOR2, ws2000mval.new, ws2000station.config.dbtype);
    if ( err) {
      syslog(LOG_ALERT,"datexn: error: SENSOR2 new flag configuration problem");
    }
    err = measval_hd( "sensor2",
                      "temperature",
                      WS2000,
                      ws2000station.config.dbtype,
                      dataset_date, 
                      ws2000mval.temperature);
    if ( err != 0 ) {
      syslog(LOG_ALERT,"datexn: error: SENSOR2 temperature : check configuration");
    }
    err = measval_hd( "sensor2",
                      "humidity",
                      WS2000,
                      ws2000station.config.dbtype,
                      dataset_date, 
                      ws2000mval.humidity);
    if ( err != 0 ) {
      syslog(LOG_ALERT,"datexn: error: SENSOR2 humidity : check configuration");
    }
  } else {
    syslog(LOG_DEBUG,"datexn: SENSOR2 temperature/humidity not found");
  }


  /* SENSOR3  Temperature/Humidity */
  err = is_ws2000sens( SENSOR3, ws2000station.config.dbtype);
  if ( err != 0 ) {
    ws2000mval = get_ws2000val( data, SENSOR3);
    syslog(LOG_DEBUG,
      "datexn: sensor #%d dataset_date: %lu "
      "temperature: %f humidity: %f new: %d\n",
      SENSOR3, 
      (long int)dataset_date, 
      ws2000mval.temperature,
      ws2000mval.humidity,
      ws2000mval.new);
    err = new_ws2000db( SENSOR3, ws2000mval.new, ws2000station.config.dbtype);
    if ( err) {
      syslog(LOG_ALERT,"datexn: error: SENSOR3 new flag configuration problem");
    }
    err = measval_hd( "sensor3",
                      "temperature",
                      WS2000,
                      ws2000station.config.dbtype,
                      dataset_date, 
                      ws2000mval.temperature);
    if ( err != 0 ) {
      syslog(LOG_ALERT,"datexn: error: SENSOR3 temperature : check configuration");
    }
    err = measval_hd( "sensor3",
                      "humidity",
                      WS2000,
                      ws2000station.config.dbtype,
                      dataset_date, 
                      ws2000mval.humidity);
    if ( err != 0 ) {
      syslog(LOG_ALERT,"datexn: error: SENSOR3 humidity : check configuration");
    }
  } else {
    syslog(LOG_DEBUG,"datexn: SENSOR3 temperature/humidity not found");
  }

  /* SENSOR4  Temperature/Humidity */
  err = is_ws2000sens( SENSOR4, ws2000station.config.dbtype);
  if ( err != 0 ) {
    ws2000mval = get_ws2000val( data, SENSOR4);
    syslog(LOG_DEBUG,
      "datexn: sensor #%d dataset_date: %lu "
      "temperature: %f humidity: %f new: %d\n",
      SENSOR4, 
      (long int)dataset_date, 
      ws2000mval.temperature,
      ws2000mval.humidity,
      ws2000mval.new);
    err = new_ws2000db( SENSOR4, ws2000mval.new, ws2000station.config.dbtype);
    if ( err) {
      syslog(LOG_ALERT,"datexn: error: SENSOR4 new flag configuration problem");
    }
    err = measval_hd( "sensor4",
                      "temperature",
                      WS2000,
                      ws2000station.config.dbtype,
                      dataset_date, 
                      ws2000mval.temperature);
    if ( err != 0 ) {
      syslog(LOG_ALERT,"datexn: error: SENSOR4 temperature : check configuration");
    }
    err = measval_hd( "sensor4",
                      "humidity",
                      WS2000,
                      ws2000station.config.dbtype,
                      dataset_date, 
                      ws2000mval.humidity);
    if ( err != 0 ) {
      syslog(LOG_ALERT,"datexn: error: SENSOR4 humidity : check configuration");
    }
  } else {
    syslog(LOG_DEBUG,"datexn: SENSOR4 temperature/humidity not found");
  }

  /* SENSOR5  Temperature/Humidity */
  err = is_ws2000sens( SENSOR5, ws2000station.config.dbtype);
  if ( err != 0 ) {
    ws2000mval = get_ws2000val( data, SENSOR5);
    syslog(LOG_DEBUG,
      "datexn: sensor #%d dataset_date: %lu "
      "temperature: %f humidity: %f new: %d\n",
      SENSOR5, 
      (long int)dataset_date, 
      ws2000mval.temperature,
      ws2000mval.humidity,
      ws2000mval.new);
    err = new_ws2000db( SENSOR5, ws2000mval.new, ws2000station.config.dbtype);
    if ( err) {
      syslog(LOG_ALERT,"datexn: error: SENSOR5 new flag configuration problem");
    }
    err = measval_hd( "sensor5",
                      "temperature",
                      WS2000,
                      ws2000station.config.dbtype,
                      dataset_date, 
                      ws2000mval.temperature);
    if ( err != 0 ) {
      syslog(LOG_ALERT,"datexn: error: SENSOR5 temperature : check configuration");
    }
    err = measval_hd( "sensor5",
                      "humidity",
                      WS2000,
                      ws2000station.config.dbtype,
                      dataset_date, 
                      ws2000mval.humidity);
    if ( err != 0 ) {
      syslog(LOG_ALERT,"datexn: error: SENSOR5 humidity : check configuration");
    }
  } else {
    syslog(LOG_DEBUG,"datexn: SENSOR5 temperature/humidity not found");
  }

  /* SENSOR6  Temperature/Humidity */
  err = is_ws2000sens( SENSOR6, ws2000station.config.dbtype);
  if ( err != 0 ) {
    ws2000mval = get_ws2000val( data, SENSOR6);
    syslog(LOG_DEBUG,
      "datexn: sensor #%d dataset_date: %lu "
      "temperature: %f humidity: %f new: %d\n",
      SENSOR6, 
      (long int)dataset_date, 
      ws2000mval.temperature,
      ws2000mval.humidity,
      ws2000mval.new);
    err = new_ws2000db( SENSOR6, ws2000mval.new, ws2000station.config.dbtype);
    if ( err) {
      syslog(LOG_ALERT,"datexn: error: SENSOR6 new flag configuration problem");
    }
    err = measval_hd( "sensor6",
                      "temperature",
                      WS2000,
                      ws2000station.config.dbtype,
                      dataset_date, 
                      ws2000mval.temperature);
    if ( err != 0 ) {
      syslog(LOG_ALERT,"datexn: error: SENSOR6 temperature : check configuration");
    }
    err = measval_hd( "sensor6",
                      "humidity",
                      WS2000,
                      ws2000station.config.dbtype,
                      dataset_date, 
                      ws2000mval.humidity);
    if ( err != 0 ) {
      syslog(LOG_ALERT,"datexn: error: SENSOR6 humidity : check configuration");
    }
  } else {
    syslog(LOG_DEBUG,"datexn: SENSOR6 temperature/humidity not found");
  }

  /* SENSOR7  Temperature/Humidity */
  err = is_ws2000sens( SENSOR7, ws2000station.config.dbtype);
  if ( err != 0 ) {
    ws2000mval = get_ws2000val( data, SENSOR7);
    syslog(LOG_DEBUG,
      "datexn: sensor #%d dataset_date: %lu "
      "temperature: %f humidity: %f new: %d\n",
      SENSOR7, 
      (long int)dataset_date, 
      ws2000mval.temperature,
      ws2000mval.humidity,
      ws2000mval.new);
    err = new_ws2000db( SENSOR7, ws2000mval.new, ws2000station.config.dbtype);
    if ( err) {
      syslog(LOG_ALERT,"datexn: error: SENSOR7 new flag configuration problem");
    }
    err = measval_hd( "sensor7",
                      "temperature",
                      WS2000,
                      ws2000station.config.dbtype,
                      dataset_date, 
                      ws2000mval.temperature);
    if ( err != 0 ) {
      syslog(LOG_ALERT,"datexn: error: SENSOR7 temperature : check configuration");
    }
    err = measval_hd( "sensor7",
                      "humidity",
                      WS2000,
                      ws2000station.config.dbtype,
                      dataset_date, 
                      ws2000mval.humidity);
    if ( err != 0 ) {
      syslog(LOG_ALERT,"datexn: error: SENSOR7 humidity : check configuration");
    }
  } else {
    syslog(LOG_DEBUG,"datexn: SENSOR7 temperature/humidity not found");
  }

  /* SENSOR8  Temperature/Humidity */
  err = is_ws2000sens( SENSOR8, ws2000station.config.dbtype);
  if ( err != 0 ) {
    ws2000mval = get_ws2000val( data, SENSOR8);
    syslog(LOG_DEBUG,
      "datexn: sensor #%d dataset_date: %lu "
      "temperature: %f humidity: %f new: %d\n",
      SENSOR8, 
      (long int)dataset_date, 
      ws2000mval.temperature,
      ws2000mval.humidity,
      ws2000mval.new);
    err = new_ws2000db( SENSOR8, ws2000mval.new, ws2000station.config.dbtype);
    if ( err) {
      syslog(LOG_ALERT,"datexn: error: SENSOR8 new flag configuration problem");
    }
    err = measval_hd( "sensor8",
                      "temperature",
                      WS2000,
                      ws2000station.config.dbtype,
                      dataset_date, 
                      ws2000mval.temperature);
    if ( err != 0 ) {
      syslog(LOG_ALERT,"datexn: error: SENSOR8 temperature : check configuration");
    }
    err = measval_hd( "sensor8",
                      "humidity",
                      WS2000,
                      ws2000station.config.dbtype,
                      dataset_date, 
                      ws2000mval.humidity);
    if ( err != 0 ) {
      syslog(LOG_ALERT,"datexn: error: SENSOR8 humidity : check configuration");
    }
  } else {
    syslog(LOG_DEBUG,"datexn: SENSOR8 temperature/humidity not found");
  }

  /* Sensor #9: RAINSENSOR */
  err = is_ws2000sens( 9, ws2000station.config.dbtype);
  if ( err != 0 )  {
    syslog(LOG_DEBUG, "datexn: sensor #9 RAINSENSOR found\n");
    Hi = getbits(data[25], 6, 7) << 8 ;
    Lo = getbits(data[24], 7, 8);
    rainfall   = Hi + Lo;
    new =  getbits(data[25], 7, 1); /* rainsensor new flag */
    err =new_ws2000db( RAINSENSOR, new, ws2000station.config.dbtype);
    if ( err ) {
      syslog(LOG_ALERT,"datexn: error: sensor #9 RAINSENSOR new flag configuration problem");
    }
    err = measval_hd( "rainsensor",
                      "rainfall",
                      WS2000,
                      ws2000station.config.dbtype,
                      dataset_date, 
                      rainfall);
    if ( err != 0 ) {
      syslog(LOG_ALERT,"datexn: error: RAINSENSOR rainfall: check configuration");
    }
    syslog(LOG_DEBUG,
	   "datex: sensor #9 rain:\t\tdataset_date: %lu "
           "meas_value: %f new: %d\n", 
	   (long int)dataset_date, meas_value[0], new);
  } else {
    syslog(LOG_DEBUG,"datexn: sensor #9 RAINSENSOR not found\n");
  }

  /* Sensor #10: WINDSENSOR */
  err =  is_ws2000sens( WINDSENSOR, ws2000station.config.dbtype);
  if ( err != 0 )  {
   syslog(LOG_DEBUG, "datexn: sensor #10 WINDSENSOR found\n");


    /* windspeed  raw data is km/h */
    windspeed = 
      100 * getbits(data[27], 6, 3) +
      10  * getbits(data[27], 3, 4) +
      getbits(data[26], 6, 3) +
      0.1 * getbits(data[26], 3, 4);
    windspeed = windspeed / 3.6; /* km/h to m/s */

    err = measval_hd( "windsensor",
                      "windspeed",
                      WS2000,
                      ws2000station.config.dbtype,
                      dataset_date, 
                      windspeed);
    if ( err != 0 ) {
      syslog(LOG_ALERT,"datexn: error: RAINSENSOR rainfall: check configuration");
    }

    syslog(LOG_DEBUG,
     "datex: sensor #10 WINDSENSOR :\tdataset_date: %lu windspeed: %f new: %d\n", 
     (long int)dataset_date, windspeed, new);


    /* wind new flag */
    new = getbits ( data[27], 7, 1);
    err = new_ws2000db( WINDSENSOR, new, ws2000station.config.dbtype);
    if ( err ) {
      syslog(LOG_ALERT,"datexn: error: sensor #10 WINDSENSOR cannot write new flag\n");
    }


    /* wind direction */
    winddirection =
      100 * getbits( data[29], 1, 2 ) +
      10 * getbits(data[28], 7, 4 ) +
      getbits(data[28], 3, 4 );

    err = measval_hd( "windsensor",
                      "winddirection",
                      WS2000,
                      ws2000station.config.dbtype,
                      dataset_date, 
                      winddirection);
    if ( err != 0 ) {
      syslog(LOG_ALERT,"datexn: error: WINDSENSOR winddirection: check configuration");
    }

    syslog(LOG_DEBUG,
      "datex: sensor #10 WINDSENSOR:\tdataset_date: %lu winddirection: %f\n", 
      (long int)dataset_date, winddirection);


    /* mean deviation of wind direction */
    winddeviation =
      getbits( data[29], 4, 2 );

    err = measval_hd( "windsensor",
                      "winddeviation",
                      WS2000,
                      ws2000station.config.dbtype,
                      dataset_date, 
                      winddeviation);

    if ( err != 0 ) {
      syslog(LOG_ALERT,"datexn: error: WINDSENSOR winddeviation: check configuration");
    }

    syslog(LOG_DEBUG,
      "datexn: sensor #10 WINDSENSOR:\tdataset_date: %lu winddeviation: %f\n", 
      (long int)dataset_date, winddeviation);
  } else {
    syslog(LOG_DEBUG,"datex: sensor #10 windsensor not found\n");
  } 


  /* Sensor #11: INDOORSENSOR */
  err = is_ws2000sens( INDOORSENSOR, ws2000station.config.dbtype);
  if ( err != 0 )  {
    syslog(LOG_DEBUG, "datexn: sensor #11 INDOORSENSOR found\n");


    /* barometric pressure */
    pressure = 
      100 *  getbits(data[30], 7, 4) +
      10  *  getbits(data[30], 3, 4) +
      getbits(data[29], 7, 4) + 200;

    err = measval_hd( "indoorsensor",
                      "pressure",
                      WS2000,
                      ws2000station.config.dbtype,
                      dataset_date, 
                      pressure);
    if ( err != 0 ) {
      syslog(LOG_ALERT,"datexn: error: INDOORSENSOR pressure: check configuration");
    }

    syslog(LOG_DEBUG,
      "datexn: sensor #11 INDOORSENSOR:\tdataset_date: %lu pressure: %f\n", 
      (long int)dataset_date, pressure);


    /* indoor temperature */
    temperature = 
      10  * getbits(data[32], 3, 4) + 
      getbits(data[31], 7, 4) +
      0.1 * getbits(data[31], 7, 4);

    err = measval_hd( "indoorsensor",
                      "temperature",
                      WS2000,
                      ws2000station.config.dbtype,
                      dataset_date, 
                      temperature);
    if ( err != 0 ) {
      syslog(LOG_ALERT,"datexn: error: INDOORSENSOR temperature: check configuration");
    }

    syslog(LOG_DEBUG,
      "datexn: sensor #11 INDOORSENSOR:\tdataset_date: %lu temperature: %f\n", 
      (long int)dataset_date, temperature);

    /* indoor humidity */
    Lo = getbits(data[32], 7, 4);
    Hi = getbits(data[33], 2, 3) << 4;
    humidity = Hi + Lo;

    err = measval_hd( "indoorsensor",
                      "humidity",
                      WS2000,
                      ws2000station.config.dbtype,
                      dataset_date, 
                      humidity);
    if ( err != 0 ) {
      syslog(LOG_ALERT,"datexn: error: INDOORSENSOR humidity: check configuration");
    }

    syslog(LOG_DEBUG,
      "datexn: sensor #11 INDOORSENSOR:\tdataset_date: %lu humidity: %f ",
      (long int)dataset_date, humidity);
    syslog(LOG_DEBUG,
      "datexn: sensor #11 INDOORSENSOR: humidity: Hi: %x(h) Lo: %x(h)", Hi, Lo);


    /* indoor sensor new flag */
    new = getbits( data[33], 3, 1);

    syslog(LOG_DEBUG,
      "datexn: sensor #11 INDOORSENSOR:\tdataset_date: %lu new: %d\n",
      (long int)dataset_date, new);

    err = new_ws2000db( INDOORSENSOR, new, ws2000station.config.dbtype); 
    if ( err) {
      syslog(LOG_ALERT,"datexn: error: sensor #11 INDOORSENSOR cannot write database");
    }
  } else {
    syslog(LOG_DEBUG,"datexn: sensor #11 indoorsensor not found\n");
  }

  return(err);
};



/* 
  wcmd  
  execution of WS2000 weatherstation commands

*/
int
wcmd ( ) {
  int ndat = 0;                   /* length of array containing the 
                                     data frame */
  int err;                        /* return value of functions */
  int command;
  int argcm;
  int dle_isset = FALSE;
  static unsigned char data[MAXBUFF+1]; /* data array store the raw dataframe 
                                           and the message datagram */
  char *clk;                            /* display time in reasonable format */
  char *rbuf;                           /* return buffer */

  time_t pstatime;
  struct tm *pstatm;
  struct rusage pstat;

  time(&pstatime); pstatm = gmtime(&pstatime);
  err = getrusage( RUSAGE_SELF, &pstat);
  syslog(LOG_DEBUG, "wcmd_i: memory check: %lu : "
         "maxrss: %ld : ixrss: %ld idrss: %ld isrss : %ld\n",
         (long int)pstatime, pstat.ru_maxrss,
         pstat.ru_ixrss, pstat.ru_idrss,pstat.ru_isrss);

  syslog(LOG_DEBUG, "wcmd: called for command request: %d\n",wsconf.command);  
  ws2000station.status.ndats = 0;

  /* first get DCF time if possible */
  command = wsconf.command; 
  argcm   = wsconf.argcmd; 
  wsconf.command = 0;

  
  if ( ( err = getcd( data, &ndat)) == -1) {
    syslog(LOG_CRIT, "wcmd: error data reception");
    ws2000station.status.is_present = -1;
    return(1);
  }
  wsconf.command = command;

  ws2000station.status.is_present = 1;
  ws2000station.status.DCFtime  = dcftime(data, ndat);
   if ( ws2000station.status.DCFtime == -1) {
    snprintf(ws2000station.status.message, MAXMSGLEN, "DCF not synchronized\n");
    syslog( LOG_INFO, "wcmd: DCF not synchronized\n");
  }

  /* get status of weatherstation 
     needed to fill sens.status
  */
  command = wsconf.command; 
  argcm   = wsconf.argcmd; 
  wsconf.command = 5;
  if ( ( err = getcd( data, &ndat)) == -1) {
    syslog(LOG_CRIT, "wcmd: error data reception");
    return(err);
  }
  wsconf.command = command;
  syslog(LOG_DEBUG, "wcmd : check status OK\n");


  /* status weatherstation */
  if ( ( rbuf = wstat(data, ndat)) == NULL) {
    syslog(LOG_CRIT, "wcmd: error in subroutine wstat");
    return(1);
  }

  /* command 0 : poll DCF time */
  if (command == 0) {
    tzset();
	
    /* write command and retrieve data */
    if ( ( err = getcd( data, &ndat)) == -1) {
      syslog(LOG_CRIT, "wcmd: error data reception");
      return(1);
    }

    /* calculate seconds since EPOCH if DCF synchronized */
    ws2000station.status.DCFtime  = dcftime(data, ndat);
    if ( ws2000station.status.DCFtime == -1) {
      syslog(LOG_NOTICE, "DCF not synchronized");
      snprintf(ws2000station.status.message, MAXMSGLEN, "DCF not synchronized\n");
      return(0);
    }
    else {
      clk = ctime(&ws2000station.status.DCFtime);
      syslog(LOG_INFO, "%s", clk);
      snprintf(ws2000station.status.message, MAXMSGLEN, "%s", clk);
      return(0);
    }
  }

  /* command 1 : Request Dataset */
  else if (command == 1)  {
    /* first get DCF time if possible */
    wsconf.command = 0;
    if ( ( err = getcd( data, &ndat)) == -1) {
      syslog(LOG_CRIT, "wcmd: error data reception");
      snprintf( ws2000station.status.message, MAXMSGLEN, 
        "error: data reception\n");
      return(1);
    }
    wsconf.command = command;

    /* calculate seconds since EPOCH if DCF synchronized */
    ws2000station.status.DCFtime  = dcftime(data, ndat);

    /* write command and retrieve data */
    if ( ( err = getcd( data, &ndat)) == -1) {
      syslog(LOG_CRIT, "wcmd: error data reception");
      snprintf( ws2000station.status.message, MAXMSGLEN, 
        "error: data reception\n");
      return(1);
    }

    /* weather station response : no data available: <DLE> */
    if ( ( ndat == 1 ) && ( data[0] == DLE ) ) {
      syslog(LOG_INFO, "no data available (<DLE> received)");
      snprintf( ws2000station.status.message, MAXMSGLEN, 
        "no data available\n");
      return(0);
    }
    /* fill data structure sens */
    else {
      /* get one dataset */
      err = datexn(data, ndat);
      syslog(LOG_DEBUG, "wcmd : returncode datexn: %d\n", err);
      ws2000station.status.ndats = ws2000station.status.ndats + 1;
    }
    return(err);
  } 


  /* command 2 : Select next dataset */
  else if (command == 2) {
    /* write the command word to the weather station */ 
    /* extract message datagram */
    if ( ( err = getcd( data, &ndat)) == -1) {
      syslog(LOG_CRIT, "wcmd: error data reception");
      return(1);
    }

    /* if DLE no data available */
    if ( ( ndat == 1 ) && ( data[0] == DLE ) ) {
      syslog(LOG_INFO, "no data available(<DLE> received)");
      return(0);
    } else if ( ( ndat == 1 ) && ( data[0] == ACK ) )  {
      /* if ACK next dataset is available */
      syslog(LOG_INFO, "next dataset available (<ACK> received)");
      return(0);
    }
    /* exit if unknown response */
    else {
      syslog(LOG_CRIT, "error next dataset : \"unknown response\"");
      return(1);
    }
  }


  /* command 3 : Activate 8 temperature sensors */
  else if (command == 3) {

    /* write the command word to the weather station */ 
    /* extract message datagram */
    if ( ( err = getcd( data, &ndat)) == -1) {
      syslog(LOG_CRIT, "wcmd: error data reception");
      return(1);
    }

    /* weather station response : <ACK> */
    if ( ( ndat == 1 ) && ( data[0] == ACK ) ) {
      syslog(LOG_INFO, "set 9 temperature sensors (<ACK> received)");
      return(0);
    }

    /* update status */
    wsconf.command = 5;
    if ( ( err = getcd( data, &ndat)) == -1) {
      syslog(LOG_CRIT, "wcmd: error data reception");
      return(1);
    }
    if ( ( rbuf = wstat(data, ndat)) == NULL) {
      syslog(LOG_CRIT, "wcmd: error in subroutine wstat");
      return(1);
    }	  
  } 


  /* command 4 : Activate 16 temperature sensors */
  else if (command == 4) {

    /* write the command word to the weather station */ 
    /* extract message datagram */
    if ( ( err = getcd( data, &ndat)) == -1) {
      syslog(LOG_CRIT, "wcmd: error data reception");
      return(1);
    }

    /* weather station response : <ACK> */
    if ( ( ndat == 1 ) && ( data[0] == ACK ) ) {
      syslog(LOG_INFO, "set 16 temperature sensors(<ACK> received)");
      return(0);
    }

    /* update status */
    wsconf.command = 5;
    if ( ( err = getcd( data, &ndat)) == -1) {
      syslog(LOG_CRIT, "wcmd: error data reception");
      return(1);
    }
    if ( ( rbuf = wstat(data, ndat)) == NULL) {
      syslog(LOG_CRIT, "wcmd: error in subroutine wstat");
      return(1);
    }	  
  } 


  /* command 5 : Request status of weatherstation */
  else if ( command == 5 ) {
    if ( ( err = getcd( data, &ndat)) == -1) {
      syslog(LOG_CRIT, "wcmd: error data reception");
      return(1);
    }
    
    /* status weatherstation */
    if ( ( rbuf = wstat(data, ndat)) == NULL) {
      syslog(LOG_CRIT, "wcmd: error in subroutine wstat");
      return(1);
    } else {
      syslog(LOG_DEBUG, "wcmd: rbuf: %s", rbuf);
      return(0);
    }
  } 


  /* command 6 : Set logging intervall of weatherstation */
  else if (command == 6) {
    /* write the command word to the weather station */ 
    /* extract message datagram */
    if ( ( err = getcd( data, &ndat)) == -1) {
      syslog(LOG_CRIT, "wcmd: error data reception\n");
      return(1);
    }

    /* weather station response : <ACK> */
    if ( ( ndat == 1 ) && ( data[0] == ACK ) ) {
      syslog(LOG_NOTICE, 
	     "set logging interval to %d [min] (<ACK> received)\n", 
	     wsconf.argcmd);
      return(0);
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
      syslog(LOG_CRIT, "wcmd: error data reception");
      return(1);
    }

    /* calculate seconds since EPOCH if DCF synchronized */
    ws2000station.status.DCFtime  = dcftime(data, ndat);
	  
    while ( dle_isset != TRUE) {
      /* 1st step: command #1: request dataset */
      wsconf.command = 1;
      if ( ( err = getcd( data, &ndat)) == -1) {
        syslog(LOG_CRIT,"wcmd: error data reception");
	return(1);
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
	if ( ( err = datexn(data, ndat)) == -1) {
	  syslog(LOG_CRIT, "wcmd: error extracting data frame");
	  return(err);
	}
	ws2000station.status.ndats++;
      }
 
      /* 2nd step: command #2: request next dataset */
      wsconf.command = 2;
      if ( ( err = getcd( data, &ndat)) == -1) {
	syslog(LOG_CRIT,"wcmd: error data reception");
	return(err);
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
        syslog(LOG_CRIT, 
	       "wcmd: error request next dataset : unknown response");
	return(1);
      }
    }
    return(0);
  }//command 12

  else {
    syslog(LOG_CRIT,"wcmd: unknown command"); 
    return(1);
  }
  /* catch all: should never be reached */
  syslog(LOG_CRIT,"wcmd: called w/o command: should not happen."); 
  return(1);
} /* wcmd ends here */



/*
  WS2000 weatherstation handler
  
  writing WS2000 data to database

*/
void *
ws2000_hd( void *arg) {
  int lfd, err;
  int waitmax  = 0;

  syslog(LOG_NOTICE,"ws2000_hd start exceuting\n");


  if ( ws2000station.config.dbtype == SQLITE) {
    if ( ( err = sqlite3_open( ws2000station.config.sqlite_dbfile, &ws2000db))) {
      syslog(LOG_ALERT, "ws2000_hd: error: failed to open database %s.",
        ws2000station.config.sqlite_dbfile);
      return( ( void *) &failure);
    }
  } else {
    syslog(LOG_ALERT, "ws2000_hd: error: no database type defined");
    return( ( void *) &failure);
  }


  for ( ;; ) {
    while ( waitmax < MAXWAIT ) {
      syslog(LOG_DEBUG,"ws2000_hd: waitmax: %d\n", waitmax);
      lfd = open( lockfile, O_RDWR | O_CREAT | O_EXCL, 0444);
      if ( lfd == -1 ) {
        syslog(LOG_CRIT,"ws2000_hd: %s\n", strerror(errno));
        syslog(LOG_CRIT, "ws2000_hd: lockfile \"%s\" already locked\n",
               lockfile);
	sleep(5);
      } else {
	/* sending command to weather station */
        wsconf.command = 12;
        syslog(LOG_DEBUG,"ws2000_hd: sending command: %d\n", wsconf.command); 
	syslog(LOG_INFO,"ws2000_hd: wcmd return code: %d\n", wcmd()); 
        close( lfd);
        unlink( lockfile);
        waitmax = 0;
        break;
      }
      waitmax++; 
    }
    sleep(DATAWAIT);
  }
  /* database cleanup and close */
  if ( ws2000station.config.dbtype == SQLITE) {
    sqlite3_close( ws2000db);
  } 
  return( ( void *) &success);
}



