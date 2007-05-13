/* libwth.c

   $Id: libwth.c,v 1.2 2005/10/14 14:21:53 jahns Exp jahns $
   $Revision: 1.2 $

   Copyright (C) 2000-2002,2005,2007 Volker Jahns <volker@thalreit.de>

   some code orginates from UNIX network programming (R.Stevens)
     http://www.kohala.com/start

   configuration file parsing taken partly and modified from thttpd 
     http://www.acme.com/software/thttpd/
   
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
#include "wthprv.h"

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


/* getcd

   get corrected data
*/
int 
getcd ( unsigned char *data, int *mdat, struct cmd *pcmd) {
    int i;
    int err;

    /* read answer of weather station from serial port */
    if ( ( err = getrd( data, mdat, pcmd)) == -1)
	  return(-1);
    
    /* echo raw dataframe */
    err = echodata( data, *mdat);
    
    /* check data frame and do 
    	 data correction for masqueraded bytes */
    if ( ( err = chkframe( data, mdat, pcmd)) == -1)
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
chkframe( unsigned char *data, int *mdat, struct cmd *pcmd) {
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
wstat(unsigned char *data, int mdat, struct wthio *rw ) {
    int i;
    char frame[MAXBUFF] = ""; /* WS2000 can hold 32 kByte of data, 
                                one dataset is 56 byte long, makes 572 max.
                             */
    char sf[3] = "";
    static char t[MAXBUFF];
    char *s;

    /* empty t */
    t[0] = '\0';

    /* status of first 8 temperature/humidity sensors */
    for ( i = 0; i < 8; i++) {
        rw->sens[2*i].status   = data[i];
        rw->sens[2*i+1].status = data[i];
    }
    
    /* rain sensor */
    rw->sens[16].status  = data[8];
    /* wind sensor */
    rw->sens[17].status  = data[9];
    rw->sens[18].status  = data[9];
    rw->sens[19].status  = data[9];
    /* indoor sensor : temp, hum, press */
    rw->sens[20].status  = data[10];
    rw->sens[21].status  = data[10];
    rw->sens[22].status  = data[10];
        
    /* status of temperature/humidity sensor 9 to 15 */
    for ( i = 12; i < 18; i++) {
	  rw->sens[2*i].status   = data[i];
	  rw->sens[2*i+1].status = data[i];
    }

    for ( i = 0; i < MAXSENSORS; i++ ) {
      sprintf(sf, "%2d:",i);
      strcat(frame, sf);
    }

    syslog(LOG_DEBUG, "wstat : %s\n", frame);    
    strcpy(frame, "");

    for ( i = 0; i < MAXSENSORS; i++ ) {
	  sprintf(sf, "%2x:",rw->sens[i].status);
      strcat(frame, sf);
    }
    syslog(LOG_DEBUG, "wstat : %s\n", frame);   


	
    /* interval time */
    rw->wstat.intvaltime = data[18];
	
    /* DCF status and synchronicity */
    rw->DCF.stat = getbits(data[19],0,1);
    rw->DCF.sync = getbits(data[19],3,1);

    /* number of sensors */
    if ( getbits(data[19],2,1) == 0 ) { 
        rw->wstat.nsens = 23;
    }
    else if ( getbits(data[19],2,1) == 1 ) {
        rw->wstat.nsens = 42;
    }
        
    /* version number */
    rw->wstat.version = data[20];

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
    
    syslog(LOG_DEBUG, "wstat : bit 4  : %d\n", getbits(data[19],4,1));
    syslog(LOG_DEBUG, "wstat : bit 5  : %d\n", getbits(data[19],5,1));
    syslog(LOG_DEBUG, "wstat : bit 6 : %d\n", getbits(data[19],6,1));
    syslog(LOG_DEBUG, "wstat : bit 7 : %d\n", getbits(data[19],7,1));

    syslog(LOG_DEBUG, "wstat: DCF.stat : %d\n", rw->DCF.stat);
    syslog(LOG_DEBUG, "wstat: DCF.sync : %d\n", rw->DCF.sync);
    syslog(LOG_DEBUG, "wstat: number sensors : %d\n", rw->wstat.nsens);
    syslog(LOG_DEBUG, "wstat: version : %x\n", rw->wstat.version);


    /* fill return buffer */
    s = mkmsg(
	      "Status\nVersion number\t:\t%x\nInterval time\t:\t%d (min)\n",
	      rw->wstat.version, rw->wstat.intvaltime);
    strncat( t, s, strlen(s));   

    if ( rw->DCF.stat == 1 ) {
      s = mkmsg("DCF status\t:\t%d (DCF receiver present)\n", 
		rw->DCF.stat);
    }
    else {
      s = mkmsg("DCF status\t:\t%d (no DCF receiver found)\n", 
		rw->DCF.stat);
    }

    strncat(t, s, strlen(s));
	
    if ( rw->DCF.sync == 1 ) {
      s = mkmsg("DCF sync.\t:\t%d (DCF synchronized)\n", rw->DCF.sync);
    }
    else {
      s = mkmsg("DCF sync.\t:\t%d (DCF NOT synchronized)\n", rw->DCF.sync);
    }
    strcat(t,s);
      
    s = mkmsg("Sensor status\t:\t( %d sensors)\n", rw->wstat.nsens);
    strcat(t,s);
      
    for ( i = 0; i < rw->wstat.nsens; i++ ) {
      s = mkmsg("%2d|", i);
      strcat(t,s);  
    }
    strcat(t,"\n");
      
    for ( i = 0; i < rw->wstat.nsens; i++ ) {
	s= mkmsg("%2x|", rw->sens[i].status);
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
settime (struct wthio *rw) {
  struct timezone tz;
  struct timeval tv;
 
  if ( gettimeofday( &tv, &tz) == -1 )
    return(-1);

  tv.tv_sec  = rw->DCF.time;
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
datex(unsigned char *data, int ndat, struct wthio *rw, struct cmd *pcmd) {
  int i;
  int j;
  int err;
  int mbit;
  int nbit;
  int Hi, Lo;
  long age;
  time_t mtim;
  char *clk;
  float temp;
	
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
  if ( rw->DCF.time != -1 ) {
    mtim = rw->DCF.time;
  }
  else { 
    syslog(LOG_INFO,
	   "datex : DCF not synchronized, using localtime for dataset\n");
    err = time(&mtim);
    syslog(LOG_DEBUG, 
	   "datex : present time: %lu (seconds since EPOCH)\n", 
           (long int)mtim);
  }
    
  /* dataset time is echoed in EPOCH seconds, dataset age in minutes
     up to present time
  */
  mtim = ( mtim/60 - age) * 60;
  clk  = ctime(&mtim);

  syslog(LOG_DEBUG, "datex : rw->wstat.ndats : %lu; age[min] : %lu\n",
	 rw->wstat.ndats, age);
  syslog(LOG_DEBUG, "datex : measured at : %lu (seconds since EPOCH)\n",
	 (long int)mtim);
  syslog(LOG_DEBUG, "datex : measured at : %s\n", clk);
  syslog(LOG_DEBUG, "datex : units : %s\n", pcmd->units);
	
  /* get data of the first 8 temperature/humidity sensors */
  for ( i = 0; i < 8; i++) {
    j = i % 2;
    /* even i */
    if ( (i % 2) == 0 ) {
      mbit=3; nbit=7;
      j = (5*i - j)/2 + 4;

      /* temperature */
      if ( rw->sens[2*i].status != 0 ) {
	rw->sens[2*i].mess[rw->wstat.ndats].age = age;

	rw->sens[2*i].mess[rw->wstat.ndats].time = mtim;
	rw->sens[2*i].mess[rw->wstat.ndats].value =  
	  10 * getbits(data[j+1], mbit-1, 3) + 
	  getbits(data[j], nbit, 4) +
	  0.1* getbits(data[j], mbit, 4);

	if( strncmp( pcmd->units, "US", 2) == 0 ) {
	  temp = rw->sens[2*i].mess[rw->wstat.ndats].value;
	  rw->sens[2*i].mess[rw->wstat.ndats].value = 9*temp/5+32; // temp F
	}

	rw->sens[2*i].mess[rw->wstat.ndats].sign   = 
	  getbits(data[j+1], mbit, 1);
      }

      /* humidity 
	 the value is calculated from an 8bit Byte which is formed
	 by two Nibbles    
      */
      if ( rw->sens[2*i+1].status != 0 ) {
	Lo = getbits(data[j+1], nbit, 4);
	Hi = getbits(data[j+2], mbit-1, 3) << 4;
	rw->sens[2*i+1].mess[rw->wstat.ndats].time  = mtim;
	rw->sens[2*i+1].mess[rw->wstat.ndats].age = age;
	rw->sens[2*i+1].mess[rw->wstat.ndats].value = Hi + Lo;
	/* Bit 3 of Hi Nibble is new flag */
	rw->sens[2*i+1].mess[rw->wstat.ndats].sign  = 
	  getbits(data[j+2], mbit, 1);
      }

    } /* odd i */ else if ( (i % 2) == 1) {
      mbit=7; nbit=3;
      j = (5*i - j)/2 + 4;

      /* temperature */
      if ( rw->sens[2*i].status != 0 ) {
	rw->sens[2*i].mess[rw->wstat.ndats].time = mtim;
	rw->sens[2*i].mess[rw->wstat.ndats].age = age;
		
	rw->sens[2*i].mess[rw->wstat.ndats].value =
	  10  * getbits(data[j+1], mbit-1, 3) +
	  getbits(data[j+1], nbit, 4) +
	  0.1 * getbits(data[j], mbit, 4);

	if( strncmp( pcmd->units, "US", 2) == 0 ){
	  temp = rw->sens[2*i].mess[rw->wstat.ndats].value;
	  rw->sens[2*i].mess[rw->wstat.ndats].value = 9*temp/5+32; // temp F
	}		

	rw->sens[2*i].mess[rw->wstat.ndats].sign = 
	  getbits(data[j+1], mbit, 1);
      }
      /* humidity 
	 the value is calculated from an 8bit Byte which is formed
	 by two Nibbles    
      */
      if ( rw->sens[2*i+1].status != 0 ) {
	Lo = getbits(data[j+2], nbit, 4);
	Hi = getbits(data[j+2], mbit-1, 3) << 4;

	rw->sens[2*i+1].mess[rw->wstat.ndats].time = mtim;
	rw->sens[2*i+1].mess[rw->wstat.ndats].age = age;
	rw->sens[2*i+1].mess[rw->wstat.ndats].value = Hi + Lo;
	/* Bit 3 of Hi Nibble is new flag */
	rw->sens[2*i+1].mess[rw->wstat.ndats].sign = 
	  getbits(data[j+2], mbit, 1);
      }
    }
  }

  /* rain sensor */
  Hi = getbits(data[25], 6, 7) << 8 ;
  Lo = getbits(data[24], 7, 8);

  rw->sens[16].mess[rw->wstat.ndats].time = mtim;
  rw->sens[16].mess[rw->wstat.ndats].age = age;
  rw->sens[16].mess[rw->wstat.ndats].value     = Hi + Lo;
  rw->sens[16].mess[rw->wstat.ndats].sign      = getbits(data[25], 7, 1);
        

  /* wind speed */
  rw->sens[17].mess[rw->wstat.ndats].time  = mtim;
  rw->sens[17].mess[rw->wstat.ndats].age   = age;
  rw->sens[17].mess[rw->wstat.ndats].sign  = 0;
  rw->sens[17].mess[rw->wstat.ndats].value = 
    100 * getbits(data[27], 6, 3) +
    10  * getbits(data[27], 3, 4) +
    getbits(data[26], 6, 3) +
    0.1 * getbits(data[26], 3, 4);

  if( strncmp( pcmd->units, "US", 2) == 0 ) {
    temp = rw->sens[17].mess[rw->wstat.ndats].value;
    rw->sens[17].mess[rw->wstat.ndats].value = temp/1.609; // speed mph
  }

  /* wind direction */
  rw->sens[18].mess[rw->wstat.ndats].time = mtim;
  rw->sens[18].mess[rw->wstat.ndats].age = age;
  rw->sens[18].mess[rw->wstat.ndats].sign     = 0;
  rw->sens[18].mess[rw->wstat.ndats].value =
    100 * getbits( data[29], 1, 2 ) +
    10  * getbits(data[28], 7, 4 ) +
    getbits(data[28], 3, 4 );

  /* mean deviation of wind direction */
  rw->sens[19].mess[rw->wstat.ndats].time  = mtim;
  rw->sens[19].mess[rw->wstat.ndats].age   = age;
  rw->sens[19].mess[rw->wstat.ndats].sign  = 0;
  rw->sens[19].mess[rw->wstat.ndats].value =
    getbits( data[29], 4, 2 );
   
  /* atmospheric pressure */
  rw->sens[20].mess[rw->wstat.ndats].time = mtim;
  rw->sens[20].mess[rw->wstat.ndats].age  = age;
  rw->sens[20].mess[rw->wstat.ndats].sign = 0;
  rw->sens[20].mess[rw->wstat.ndats].value = 
    100 *  getbits(data[30], 7, 4) +
    10  *  getbits(data[30], 3, 4) +
    getbits(data[29], 7, 4) + 200;

  if( strncmp( pcmd->units, "US", 2) == 0 ) {
    temp = rw->sens[20].mess[rw->wstat.ndats].value;
    rw->sens[20].mess[rw->wstat.ndats].value = temp/33.86389; // pressure inHg
  }

  /* indoor temperature */
  rw->sens[21].mess[rw->wstat.ndats].time  = mtim;
  rw->sens[21].mess[rw->wstat.ndats].age   = age;
  rw->sens[21].mess[rw->wstat.ndats].sign  = 0;
  rw->sens[21].mess[rw->wstat.ndats].value = 
    10  * getbits(data[32], 3, 4) + 
    getbits(data[31], 7, 4) +
    0.1 * getbits(data[31], 7, 4);

  if( strncmp( pcmd->units, "US", 2) == 0 ) {
    temp = rw->sens[21].mess[rw->wstat.ndats].value;
    rw->sens[21].mess[rw->wstat.ndats].value = 9*temp/5+32; // temp F
  }

  /* indoor humidity */
  Lo = getbits(data[32], 7, 4);
  Hi = getbits(data[33], 2, 3) << 4;
  rw->sens[22].mess[rw->wstat.ndats].time  = mtim;
  rw->sens[22].mess[rw->wstat.ndats].age   = age;
  rw->sens[22].mess[rw->wstat.ndats].sign  = 0;
  rw->sens[22].mess[rw->wstat.ndats].value = Hi + Lo;
	
  return(0);
};


/* pdata 

print the sensor data as hold in structure sens

*/
char *
pdata(struct wthio *wth, struct cmd *pcmd) {
  int i,j; 
  int size = 100;
  static char t[MAXBUFF];
  char *s;
  struct tm tm_buff;

  if ( strncmp(pcmd->outfmt,"old",3) == 0) {
    t[0] = '\0';
    s = mkmsg("Sensor\tType\tStatus\tdataset\ttime\t\tsign\tabs.value\n");
    strncat(t, s, strlen(s));
  
    for ( i = 0; i < MAXSENSORS; i++ ) {
      if ( wth->sens[i].status != 0 ) {   
	for ( j = 0; j < wth->wstat.ndats; j++) {

	  s = mkmsg("%d\t%s\t%d\t%d\t%lu\t%d\t%8.1f\n", i, 
		    wth->sens[i].type, wth->sens[i].status, j, 
		    wth->sens[i].mess[j].time,
		    wth->sens[i].mess[j].sign,
		    wth->sens[i].mess[j].value);
	
	  size = size + strlen(s) + 1;
	  strncat(t, s, strlen(s));
	}
      }
    }
  } else if ( strncmp(pcmd->outfmt,"std",3) == 0) {
      t[0] = '\0';
      s = mkmsg("Not implemented\n");
      strncat(t, s, strlen(s));
  } else if ( strncmp(pcmd->outfmt,"lline",5) == 0) {
#ifdef YYY
    strcpy(t, mkmsg("#time"));
    size = strlen(t);

    for ( i = 0; i < MAXSENSORS; i++ ) {
#ifdef XXX
      if ( wth->sens[i].status != 0 )
	{        s = mkmsg("\t%s_%d", wth->sens[i].type, i );
#else
	{       s = mkmsg("\t%d", i);
#endif
	size = size + strlen(s) + 1;
	strncat(t, s, strlen(s));
	}
	}
      size++;
      strncat(t,"\n",1);
#else
      t[0] = '\0';
#endif
      localtime_r(&wth->sens[0].mess[0].time, &tm_buff);
      s = mkmsg("%04d-%02d-%02d %02d:%02d:%02d", tm_buff.tm_year+1900,tm_buff.tm_mon+1,tm_buff.tm_mday,tm_buff.tm_hour,tm_buff.tm_min,tm_buff.tm_sec);
      size = size + strlen(s) + 1;
      strncat(t, s, strlen(s));

      for ( i = 0; i < MAXSENSORS; i++ ) {
	for ( j = 0; j < wth->wstat.ndats; j++) {
	  if ( wth->sens[i].status != 0 )
	    {
              s = mkmsg("\t%.1f", wth->sens[i].mess[j].value);
	    }
	  else
	    s = "\t-";
	  size = size + strlen(s) + 1;
	  strncat(t, s, strlen(s));
	}

      }

      size++;
      strncat(t,"\n",1);
      return (t);
    }

    return (t);
  }

#if defined POSTGRES
/* pg_data

insert sensor data into postgresql database

*/
int 
pg_data ( struct cmd *pcmd, struct wthio *wth) 
{
  int j;

  /* database variables */
  char *conninfo;
  PGconn *conn;
  PGresult *res;
  int humi, humo, bar, windd, rain;
  float tempi, tempo, winds;
  char sqlstr[300];

  time_t tpc; 
  struct tm *tmpc; 
  char timebuff[40] = "";

  int conncount;

  /* make a connection to the database "weather" */
  if (( conninfo = malloc(MAXBUFF)) == NULL )
    return 1;

  snprintf( conninfo, MAXBUFF, "dbname=%s user=%s", 
	    pcmd->database, pcmd->dbuser);
  conn = PQconnectdb(conninfo);

  /* Check to see that the backend connection was successfully made */
  if (PQstatus(conn) != CONNECTION_OK){
    syslog(LOG_INFO, "pg_data: Connection to database failed: %s",
	    PQerrorMessage(conn));
    PQfinish(conn);
    return(1);
  }

  /* reset data logger access command counter */
  conncount=0;

  /* Use simple variable names for the data just retreived */

  for ( j = 0; j < wth->wstat.ndats; j++) {
    tempo = -wth->sens[0].mess[j].value;
    if(!wth->sens[0].mess[j].sign){
      tempo = -tempo;
    }
    humo = (int)wth->sens[1].mess[j].value;
    rain = (int)wth->sens[16].mess[j].value;
    winds = wth->sens[17].mess[j].value;
    windd = (int)wth->sens[18].mess[j].value;
    bar = (int)wth->sens[20].mess[j].value;
    tempi = -wth->sens[21].mess[j].value;
    if(!wth->sens[21].mess[j].sign){
      tempi = -tempi;
    }
    humi = (int)wth->sens[22].mess[j].value;
      
    tpc = wth->sens[20].mess[j].time;
    tmpc = localtime(&tpc);
    strftime(timebuff, sizeof(timebuff), "%Y-%m-%d %X", tmpc);

    /* Write data out to the database */
    snprintf(sqlstr, MAXBUFF,
	    "INSERT into weather (" 
	    "tempo,"
	    "humo,"
	    "rain,"
	    "winds,"
	    "windd,"
	    "bar,"
	    "tempi,"
	    "humi,"
	    "t )"
	    " values ( '%3.1f', "
	    "'%3d','%4d','%3.1f','%3d','%3d','%3.1f','%3d','%s')",
	    tempo,humo,rain,winds,windd,bar,tempi,humi,timebuff);
    syslog(LOG_DEBUG, "pg_data: sqlstr: %s\n",sqlstr);
      
    res = PQexec(conn, sqlstr);
    if( PQresultStatus( res ) != PGRES_COMMAND_OK ) { 
      syslog(LOG_INFO, "pg_data: PQexec failed: %s", PQerrorMessage( conn ) );
      PQfinish(conn);
      return(1);
    }
    PQclear(res);
  }

  /* Clean up database connection when exiting */
  PQfinish(conn);
  return(0);

}
#endif

/* wcmd

function fills the datastructure rw

*/
char *
wcmd (struct cmd *pcmd, struct wthio *rw) {
  int ndat = 0;                   /* length of array containing the 
                                     data frame */
  int err;                        /* return value of functions, 
                                     useful for errorhandling */
  int retval = 0;                 
  long snum = 0;                   /* current dataset number */
  int command;
  int argcm;
  unsigned char data[MAXBUFF];    /* data array to store the raw dataframe 
                                     and the message datagram */
  char *clk;                      /* display time in reasonable format */
  char *rbuf;                     /* return buffer */
  

  syslog(LOG_DEBUG, "wcmd: called for command request: %d\n",pcmd->command);  
  rw->wstat.ndats = 0;
  command = pcmd->command; 
  argcm   = pcmd->argcmd; 

  /* first get status of weatherstation 
     needed to fill sens.status
  */
  pcmd->command = 5;
  if ( ( err = getcd( data, &ndat, pcmd)) == -1) {
    rbuf = mkmsg("wcmd: error data reception\n");
    return (rbuf);
  }
  pcmd->command = command;
  syslog(LOG_DEBUG, "wcmd : check status OK\n");

  /* status weatherstation */
  if ( ( rbuf = wstat(data, ndat, rw)) == NULL) {
    rbuf = mkmsg("wcmd: error in subroutine wstat\n");
    return (rbuf);
  }
  
  /* command 0 : poll DCF time */
  if (command == 0) {
    tzset();
	
    /* write command and retrieve data */
    if ( ( err = getcd( data, &ndat, pcmd)) == -1) {
      rbuf = mkmsg("wcmd: error data reception\n");
      return (rbuf);
    }

    /* calculate seconds since EPOCH if DCF synchronized */
    rw->DCF.time  = dcftime(data, ndat);
    if (rw->DCF.time == -1) {
      rbuf = mkmsg("DCF not synchronized\n");
    }
    else {
      clk = ctime(&rw->DCF.time);
      rbuf = mkmsg("%s", clk);
    }
  }

  /* command 1 : Request Dataset */
  else if (command == 1)  {
    /* first get DCF time if possible */
    pcmd->command = 0;
    if ( ( err = getcd( data, &ndat, pcmd)) == -1) {
      rbuf = mkmsg("wcmd: error data reception\n");
      return (rbuf);
    }
    pcmd->command = command;

    /* calculate seconds since EPOCH if DCF synchronized */
    rw->DCF.time  = dcftime(data, ndat);

    /* write command and retrieve data */
    if ( ( err = getcd( data, &ndat, pcmd)) == -1) {
      rbuf = mkmsg("wcmd: error data reception\n");
      return (rbuf);
    }

    /* weather station response : no data available: <DLE> */
    if ( ( ndat == 1 ) && ( data[0] == DLE ) ) {
      rbuf = mkmsg("no data available (<DLE> received)\n");
    }
    /* fill data structure sens */
    else {
      /* get one dataset */
      err = datex(data, ndat, rw, pcmd);
      syslog(LOG_DEBUG, "wcmd : returncode datex : %d\n", err);
      rw->wstat.ndats = rw->wstat.ndats + 1;
      snum++;
    }

    /* echo sensor data */
    if ( rw->wstat.ndats > 0 )
      rbuf = pdata(rw, pcmd);
  } 


  /* command 2 : Select next dataset */
  else if (command == 2) {
    /* write the command word to the weather station */ 
    /* extract message datagram */
    if ( ( err = getcd( data, &ndat, pcmd)) == -1) {
      rbuf = mkmsg("wcmd: error data reception\n");
      return (rbuf);
    }


    /* if DLE no data available */
    if ( ( ndat == 1 ) && ( data[0] == DLE ) ) {
      rbuf = mkmsg("no data available(<DLE> received)\n");
      retval = 16; 
    } else if ( ( ndat == 1 ) && ( data[0] == ACK ) )  {
      /* if ACK next dataset is available */
      rbuf = mkmsg("next dataset available (<ACK> received)\n");
      retval = 6;
    }
    /* exit if unknown response */
    else {
      rbuf = mkmsg("error next dataset : \"unknown response\"\n");
      retval = -1;
    }
  }


  /* command 3 : Activate 8 temperature sensors */
  else if (command == 3) {

    /* write the command word to the weather station */ 
    /* extract message datagram */
    if ( ( err = getcd( data, &ndat, pcmd)) == -1) {
      rbuf = mkmsg("wcmd: error data reception\n");
      return (rbuf);
    }

    /* weather station response : <ACK> */
    if ( ( ndat == 1 ) && ( data[0] == ACK ) ) {
      rbuf = mkmsg("set 9 temperature sensors (<ACK> received)\n");
    }

    /* update status */
    pcmd->command = 5;
    if ( ( err = getcd( data, &ndat, pcmd)) == -1) {
      rbuf = mkmsg("wcmd: error data reception\n");
      return (rbuf);
    }
    if ( ( wstat(data, ndat, rw)) == NULL) {
      rbuf = mkmsg("wcmd: error in subroutine wstat\n");
      return (rbuf);
    }	  
  } 


  /* command 4 : Activate 16 temperature sensors */
  else if (command == 4) {

    /* write the command word to the weather station */ 
    /* extract message datagram */
    if ( ( err = getcd( data, &ndat, pcmd)) == -1) {
      rbuf = mkmsg("wcmd: error data reception\n");
      return (rbuf);
    }

    /* weather station response : <ACK> */
    if ( ( ndat == 1 ) && ( data[0] == ACK ) ) {
      rbuf = mkmsg("set 16 temperature sensors(<ACK> received)\n");
    }

    /* update status */
    pcmd->command = 5;
    if ( ( err = getcd( data, &ndat, pcmd)) == -1) {
      rbuf = mkmsg("wcmd: error data reception\n");
      return (rbuf);
    }
    if ( ( wstat(data, ndat, rw)) == NULL) {
      rbuf = mkmsg("wcmd: error in subroutine wstat\n");
      return (rbuf);
    }	  
  } 


  /* command 5 : Request status of weatherstation */
  else if ( command == 5 ) {
    ; /* status known, drive thru */
  } 


  /* command 6 : Set logging intervall of weatherstation */
  else if (command == 6) {
    /* write the command word to the weather station */ 
    /* extract message datagram */
    if ( ( err = getcd( data, &ndat, pcmd)) == -1) {
      rbuf = mkmsg("wcmd: error data reception\n");
      return (rbuf);
    }

    /* weather station response : <ACK> */
    if ( ( ndat == 1 ) && ( data[0] == ACK ) ) {
      rbuf = mkmsg("set logging interval to %d [min] (<ACK> received)\n",
		   pcmd->argcmd);
    }

    /* update status */
    pcmd->command = 5;
    if ( ( err = getcd( data, &ndat, pcmd)) == -1) {
      rbuf = mkmsg("wcmd: error data reception\n");
      return (rbuf);
    }

    if ( ( wstat( data, ndat, rw)) == NULL) {
      rbuf = mkmsg("wcmd: error in subroutine wstat\n");
      return (rbuf);
    }
  } 


  /* command 12 : 
     Recursively, request dataset and select next dataset, 
     recursive combination of command 1 and 2 
  */
  else if ( ( command == 12) || ( command == 7) )  {      
    /* first get DCF time if possible */
    pcmd->command = 0;
    if ( ( err = getcd( data, &ndat, pcmd)) == -1) {
      rbuf = mkmsg("wcmd: error data reception\n");
      return (rbuf);
    }
    pcmd->command = command;

    /* calculate seconds since EPOCH if DCF synchronized */
    rw->DCF.time  = dcftime(data, ndat);

	  
    while (retval != 16) {
      /* write command and retrieve data */
      pcmd->command = 1;
      if ( ( err = getcd( data, &ndat, pcmd)) == -1) {
	rbuf = mkmsg("wcmd: error data reception\n");
	return (rbuf);
      }

      /* extract dataset from raw dataframe */
      /* weather station response : no data available: <DLE> */
      if ( ( ndat == 1 ) && ( data[0] == DLE ) ) {
	rbuf = mkmsg("no data available (<DLE> received)\n");
	retval = 16;
      }
      /* do extraction */
      else {
	if ( ( err = datex(data, ndat, rw, pcmd)) == -1) {
	  rbuf = mkmsg("wcmd: error extracting data frame");
	  return (rbuf);
	}
	rw->wstat.ndats = rw->wstat.ndats + 1;
      }
 
      /* write the command word to select next dataset and 
	 retrieve response*/ 
      pcmd->command = 2;
      if ( ( err = getcd( data, &ndat, pcmd)) == -1) {
	rbuf = mkmsg("wcmd: error data reception\n");
	return (rbuf);
      }

      /* stop if DLE no data available, */
      if ( ( ndat == 1 ) && ( data[0] == DLE ) ) {
	/* rbuf = mkmsg(
	   "<DLE> received :\"no data available\"!"); */
	retval = 16; 
      }
      /* if ACK next dataset is available */
      else if ( ( ndat == 1 ) && ( data[0] == ACK ) ) {
	syslog(LOG_DEBUG, \
	       "wcmd: next dataset available (<ACK> received)!\n");
	retval = 6;
      }
      /* return if unknown response */
      /* no data is returned, maybe too strict? */
      else {
	rbuf = mkmsg(
		     "wcmd: error request next dataset : unknown response\n");
	return (rbuf);
      }
      syslog(LOG_DEBUG, "wcmd: retval : %d\n", retval);
    }

    /* echo sensor data */
    if ( rw->wstat.ndats > 0 ) {
      rbuf = pdata(rw, pcmd);
      printf("w/o statement: return (rbuf)"); 
      //return ( rbuf);
#if defined POSTGRES
      if ( command == 7 ) {
	if ( ( err = pg_data( pcmd, rw)) == -1 ) {
	  rbuf = mkmsg("wmcd: postgres error\n") ;
	  return (rbuf);
	}
      }
#endif
    }

  }//command 12 || command 7

  else {
    rbuf = mkmsg("unknown command\n");
  }
  syslog(LOG_DEBUG, "wcmd: exit OK\n");
  return(rbuf);
}//wcmd()



/* initdata 

presets the datastructures
rw->sens
pcmd

*/
int initdata(struct wthio *rw) {
  int i;  /* array subscription */  

  /* set type of the sensors */
  /* first 8 temperature/humidity sensors */
  for ( i = 0; i < 8; i++) {
    rw->sens[2*i].type   = "temp";
    rw->sens[2*i+1].type = "hum";
  }
  
  /* rain sensor */
  rw->sens[16].type  = "rain";
  /* wind sensor */
  rw->sens[17].type  = "wspd";
  rw->sens[18].type  = "wdir";
  rw->sens[19].type  = "wdev";
  /* indoor sensor : temp, hum, press */
  rw->sens[20].type  = "pres";
  rw->sens[21].type  = "temp";
  rw->sens[22].type  = "hum";
  /* sensor 9 is combined temperature/humidity */
  rw->sens[23].type  = "temp";
  rw->sens[24].type  = "hum";
      
  /* status of pressure/temperature/humidity sensor 10 to 15 */
  for ( i = 0; i < 18; i = i + 3) {
    rw->sens[25 + i    ].type = "pres";
    rw->sens[25 + i + 1].type = "temp";
    rw->sens[25 + i + 2].type = "hum";
  }
  return(0);
}


/* initcmd

presets the datastructures
pcmd

*/
struct cmd
*initcmd( void) {  

  struct cmd *inipcmd;   
  /* initialize struct pcmd */
  inipcmd = ( struct cmd *) malloc(sizeof(struct cmd *));
  inipcmd->argcmd      = 0;
  inipcmd->timeout     = TIMEOUT;
  inipcmd->baudrate    = BAUDRATE;
  inipcmd->logfacility = LOGFACILITY;
  inipcmd->device      = SER_DEVICE;
  inipcmd->verbose     = 0;
  inipcmd->netflg      = -1;
  inipcmd->hostname    = "localhost"; 
  inipcmd->port        = WPORT; 
  inipcmd->tnport      = TNPORT;   
  inipcmd->wstype      = WSTYPE;
  inipcmd->xmlport     = XMLPORT;
  inipcmd->database    = DATABASE;
  inipcmd->dbuser      = DBUSER;
  inipcmd->units       = UNITS;
  inipcmd->outfmt      = OUTFMT;

  return(inipcmd);
  //return(0);
}


/* 
   getbits

   transfrom bytes of weather station data to BCD, otherwise picking single
   or group of bits.
   Kerninghan, Ritchie, The C Programming Language, p49.
*/
unsigned char
getbits(unsigned char x, int p, int n) {
  return ( x>>(p+1-n)) & ~(~0 <<n);
}


/* echodata
  
   print out raw data frame reveived from weather station 

*/
int
echodata(unsigned char *data, int mdat) {
    int i;
    char frame[MAXBUFF] = "";
    char sf[3] = "";

    syslog(LOG_DEBUG, "echodata : length dataframe : %d\n",mdat);

    /* for better readability label each byte in frame */    
    for ( i = 0; i < mdat; i++ ) {
	  sprintf(sf, "%2d:",i);
      strcat(frame, sf);
    }
    syslog(LOG_DEBUG, "echodata : %s\n", frame);    
    strcpy(frame, "");

    for ( i = 0; i < mdat; i++ ) {
	  sprintf(sf, "%2x:",data[i]);
      strcat(frame, sf);
    }

    syslog(LOG_DEBUG, "echodata : %s\n", frame);   

    return(0);
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



/* mkmsg

   man snprintf (LINUX) sample code modified
   
   old version - does malloc lead to seg fault?
*/
char *
mkmsg(const char *fmt, ...) {
   int n, size = 1024;
   char *p;
   va_list ap;
   if ((p = malloc(MAXBUFF)) == NULL)
      return NULL;
   //printf("mkmsg: alloc p OK\n");
   while (1) {
      va_start(ap, fmt);
      n = vsnprintf (p, size, fmt, ap);
      va_end(ap);
      if (n > -1 && n < size)
         return (p);
      if (n > -1)    /* LINUX glibc 2.1, FreeBSD */ 
		size = n+1;  
	  else           /* LINUX glibc 2.0 */
        size *= 2;  /* twice the old size */
      if ((p = realloc (p, size)) == NULL)
         return NULL;
   }
}



/* usage : print handling instructions of wthc */
int
usage (int exitcode, char *error, char *addl) {
  char *bufstr;

  if (( bufstr = malloc(MAXBUFF)) == NULL ) 
  {
    return(1);
  }

  snprintf( bufstr, MAXBUFF, "Usage: wthc [Options]\n"
	  "where options include:\n"
	  "\t-c <command>\texecute command\n"
	  "\t\t0\tPoll DCF Time\n"
	  "\t\t1\tRequest dataset\n"
	  "\t\t2\tSelect next dataset\n"
	  "\t\t3\tActivate 9 temperature sensors\n"
	  "\t\t4\tActivate 16 temperature sensors\n"
	  "\t\t5\tRequest status\n"
	  "\t\t6\tSet interval time,\n" 
	   "\t\t\t\trequires extra parameter specified by option -i\n");
#ifdef POSTGRES
  snprintf( bufstr, MAXBUFF, "%s" 
	  "\t\t7\tEvery 30 seconds, request data from the data\n"
	  "\t\t\tlogger,and store it into the weather table of a\n"
	   "\t\t\tpostgres database named weather\n", bufstr);
#endif
  snprintf( bufstr, MAXBUFF, "%s"
	  "\t\t12\tRequest all available data recursively\n"
	  "\t-i <interval>\tspecifies interval time\n"
	  "\t\tpermissible values for the interval lie\n"
	  "\t\twithin the range from 1 to 60 minutes\n"
	  "\t-h <hostname>\tconnect to <hostname/ipaddress>\n"
	  "\t-p <portnumber>\tuse portnumber at remote host\n"
	  "\t\tfor connection\n"
	  "\t-s\t\tuse local serial connection\n"
	  "\t-t\t\tset time using internal DCF77 receiver\n"
	  "\t\tneeds superuser privileges\n" 
	  "\t-u <units>\tset the units of measured values\n"
          "\t\tSI\tuse SI units for output: °C, m/s, hPa\n"
          "\t\tUS\tuse US units for output: °F, mph, inHg\n"
  ,bufstr);

  fprintf(stderr, "%s", bufstr);

  if (error) fprintf(stderr, "%s: %s\n", error, addl);
  exit(exitcode);
}

/* tnusage : print handling instructions for telnet access wthd */
char *
tnusage (int exitcode, char *error, char *addl) {
  char *s;

  s = mkmsg("Available commands include:\n"
                     "\t0\t\tPoll DCF Time\n"
                     "\t1\t\tRequest dataset\n"
                     "\t2\t\tSelect next dataset\n"
                     "\t3\t\tActivate 9 temperature sensors\n"
                     "\t4\t\tActivate 16 temperature sensors\n"
                     "\t5\t\tRequest status\n"
                     "\t6 <interval>\tSet <interval> time, " 
                     "requires extra parameter\n"
		     "\t\t\tpermissible values for <interval> lie\n"
		     "\t\t\twithin the range from <1> to <60> minutes\n");

      return(s);
}

/* usaged : print handling instructions of wthd */
int
usaged (int exitcode, char *error, char *addl) {
      fprintf(stderr,"Usage: wthd [Options]\n"
                     "where options include:\n"
                     "\t-d \t\tdo not daemonize, stay in foreground\n"
                     "\t-p <portnumber>\tuse portnumber to listen\n");

      if (error) fprintf(stderr, "%s: %s\n", error, addl);
      exit(exitcode);
}


/* readconfig : read configuration file */
char *
readconfig(struct cmd *pcmd) {
  int ival;
  FILE *cfg;
  char line[BUFFSIZE];
  char *name;
  char *value;
  char *cp, *cp2;
  char *rbuf;
  char *cfgfile;

   if ( ( cfg = fopen("/etc/wth/wth.conf","r")) != NULL ) {
    cfgfile = "/etc/wth/wth.conf";
  }
  else if ( ( cfg = fopen("/usr/local/etc/wth/wth.conf","r")) != NULL ) {
    cfgfile = "/usr/local/etc/wth.conf";
  }
  else {
    rbuf = mkmsg("No configuration file found, using default parameters\n");
    return(rbuf);
  }

  rbuf = mkmsg("Using config file: %s\n", cfgfile);

  /* stolen from thttpd code */
  while ( fgets(line, sizeof(line), cfg) != NULL) {
	/* Trim comments. */
    if ( ( cp = strchr( line, '#' ) ) != NULL )
	  *cp = '\0';
	/* or empty line */
    if ( ( cp = strchr( line, '\n' ) ) != NULL )
        *cp = '\0';
		
    /* Split line into words. */
    for ( cp = line; *cp != '\0'; cp = cp2 )
	  {
        /* Skip leading whitespace. */
        cp += strspn( cp, " \t\n\r" );
        /* Find next whitespace. */
        cp2 = cp + strcspn( cp, " \t\n\r" );
        name = cp;
        value = cp2;
        if ( value != (char*) 0 )
		  *value++ = '\0';
		
        /* Interpret. */		
        if ( strcasecmp( name, "timeout" ) == 0 ) {
		  pcmd->timeout = atoi(value);
        } else if ( strcasecmp( name, "baudrate" ) == 0 ) {
          ival = atoi(value);     
		  switch (ival) {
		  case     50: pcmd->baudrate =     B50; break;
		  case     75: pcmd->baudrate =     B75; break;
		  case    110: pcmd->baudrate =    B110; break;
		  case    134: pcmd->baudrate =    B134; break;
		  case    150: pcmd->baudrate =    B150; break;
		  case    200: pcmd->baudrate =    B200; break;
		  case    300: pcmd->baudrate =    B300; break;
		  case    600: pcmd->baudrate =    B600; break;
		  case   1200: pcmd->baudrate =   B1200; break;
		  case   1800: pcmd->baudrate =   B1800; break;
		  case   2400: pcmd->baudrate =   B2400; break;
		  case   4800: pcmd->baudrate =   B4800; break;
		  case   9600: pcmd->baudrate =   B9600; break;
		  case  19200: pcmd->baudrate =  B19200; break;
		  case  38400: pcmd->baudrate =  B38400; break;
		  case  57600: pcmd->baudrate =  B57600; break;
		  case 115200: pcmd->baudrate = B115200; break;
		  default:      pcmd->baudrate =  B9600; break; /* keep gcc happy */
		  }		  
        } else if ( strcasecmp( name, "logfacility" ) == 0 ) {
		    if ( strcasecmp( value, "kern") == 0 ) 
			  pcmd->logfacility = LOG_KERN;
			else if ( strcasecmp( value, "user") == 0 ) 
			  pcmd->logfacility = LOG_USER;
			else if ( strcasecmp( value, "mail") == 0 ) 
			  pcmd->logfacility = LOG_MAIL;     
			else if ( strcasecmp( value, "daemon") == 0 ) 
			  pcmd->logfacility = LOG_DAEMON;
			else if ( strcasecmp( value, "syslog") == 0 ) 
			  pcmd->logfacility = LOG_SYSLOG;
			else if ( strcasecmp( value, "lpr") == 0 ) 
			  pcmd->logfacility = LOG_LPR;
			else if ( strcasecmp( value, "news") == 0 ) 
			  pcmd->logfacility = LOG_NEWS;
			else if ( strcasecmp( value, "uucp") == 0 ) 
			  pcmd->logfacility = LOG_UUCP;
			else if ( strcasecmp( value, "cron") == 0 ) 
			  pcmd->logfacility = LOG_CRON;
			else if ( strcasecmp( value, "ftp") == 0 ) 
			  pcmd->logfacility = LOG_FTP;
			else if ( strcasecmp( value, "local0") == 0 ) 
			  pcmd->logfacility = LOG_LOCAL0;
			else if ( strcasecmp( value, "local1") == 0 ) 
			  pcmd->logfacility = LOG_LOCAL1;
			else if ( strcasecmp( value, "local2") == 0 ) 
			  pcmd->logfacility = LOG_LOCAL2;
			else if ( strcasecmp( value, "local3") == 0 ) 
			  pcmd->logfacility = LOG_LOCAL3;
			else if ( strcasecmp( value, "local4") == 0 ) 
			  pcmd->logfacility = LOG_LOCAL4;
			else if ( strcasecmp( value, "local5") == 0 ) 
			  pcmd->logfacility = LOG_LOCAL5;
			else if ( strcasecmp( value, "local6") == 0 ) 
			  pcmd->logfacility = LOG_LOCAL6;
			else if ( strcasecmp( value, "local7") == 0 ) 
			  pcmd->logfacility = LOG_LOCAL7;
        } else if ( strcasecmp( name, "device" ) == 0 ) {
		  pcmd->device = strdup(value);
        } else if ( strcasecmp( name, "port" ) == 0 ) {
		  pcmd->port = strdup(value);
        } else if ( strcasecmp( name, "tnport" ) == 0 ) {
		  pcmd->tnport = strdup(value);
        } else if ( strcasecmp( name, "xmlport" ) == 0 ) {
		  pcmd->xmlport = strdup(value);
        } else if ( strcasecmp( name, "database" ) == 0 ) {
		  pcmd->database = strdup(value);
        } else if ( strcasecmp( name, "dbuser" ) == 0 ) {
		  pcmd->dbuser = strdup(value);
        } else {
	  rbuf = mkmsg("unknown option '%s' inf configuration file\n", name );
          return(rbuf);
        }
    }		
  }
  return(rbuf);  
}


/* echoconfig

   echo datastructure cmd
*/
char *
echoconfig (struct cmd *pcmd) {
  int size;
  char *s;
  static char t[MAXBUFF];

  s = mkmsg("Configuration parameters\n");
  size = strlen(s) + 1;
  
  strncat(t,s,strlen(s));
  s = mkmsg("\tpcmd->command: %d\n", pcmd->command);
  strncat(t,s, strlen(s));

  s = mkmsg("\tpcmd->argcmd: %d\n",pcmd->argcmd);
  strncat(t,s, strlen(s));

  s = mkmsg("\tpcmd->netflg: %d\n",pcmd->netflg);
  strncat(t,s, strlen(s));
  s = mkmsg("\tpcmd->verbose: %d\n",pcmd->verbose);
  strncat(t,s, strlen(s));
  s = mkmsg("\tpcmd->timeout: %d\n",pcmd->timeout);
  strncat(t,s, strlen(s));
  s = mkmsg("\tpcmd->baudrate: %d\n",pcmd->baudrate);
  strncat(t,s, strlen(s));
  s = mkmsg("\tpcmd->logfacility: %d\n",pcmd->logfacility);
  strncat(t,s, strlen(s));
  s = mkmsg("\tpcmd->device: %s\n",pcmd->device);
  strncat(t,s, strlen(s));
  s = mkmsg("\tpcmd->hostname: %s\n",pcmd->hostname);
  strncat(t,s, strlen(s));
  s = mkmsg("\tpcmd->port: %s\n",pcmd->port);
  strncat(t,s, strlen(s));
  s = mkmsg("\tpcmd->tnport: %s\n",pcmd->tnport);
  strncat(t,s, strlen(s)); 
  s = mkmsg("\tpcmd->xmlport: %s\n",pcmd->xmlport);
  strncat(t,s, strlen(s)); 

  return(t);
}

Sigfunc *
signal(int signo, Sigfunc *func)
{
	struct sigaction	act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (signo == SIGALRM) {
#ifdef	SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;	/* SunOS 4.x */
#endif
	} else {
#ifdef	SA_RESTART
		act.sa_flags |= SA_RESTART;		/* SVR4, 44BSD */
#endif
	}
	if (sigaction(signo, &act, &oact) < 0) {
	  return(SIG_ERR);
	}
	return(oact.sa_handler);
}
/* end signal */


pid_t
Fork(void)
{
	pid_t	pid;

	if ( (pid = fork()) == -1) {
	  syslog(LOG_INFO,"fork error");
	  werrno = ESIG;
	  return(-1);
	}
	return(pid);
}


int
Close(int fd)
{
  if (close(fd) == -1) {
	werrno = errno;
	syslog(LOG_INFO,"Close: close error: %s",
		   strerror(werrno));
	return(-1);
  }
  return(0);
}

ssize_t	/* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (errno == EINTR)
				nwritten = 0;	/* and call write() again */
			else
				return(-1);	/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}
/* end writen */

int
Writen(int fd, void *ptr, size_t nbytes)
{
  if (writen(fd, ptr, nbytes) != nbytes) {
	werrno = ENET;
	syslog(LOG_INFO,"Writen: writen error");
	return(-1);
  }
  return(0);
}


ssize_t
Read(int fd, void *ptr, size_t nbytes)
{
	ssize_t		n;

	if ( (n = read(fd, ptr, nbytes)) == -1) {
	  werrno = errno;
	  syslog(LOG_INFO,"Read: read error: %s",
			 strerror(werrno));
	  return(-1);
	}
	return(n);
}

int
Write(int fd, void *ptr, size_t nbytes)
{
  if (write(fd, ptr, nbytes) != nbytes) {
	werrno = errno;
	syslog(LOG_INFO,"Write: write error: %s",
		   strerror(werrno));
	return(-1);
  }
  return(0);
}
