/* wthlib.c */

#include "global.h"
#include "config.h"
#include "const.h"
#include "dates.h"
#include "net.h"
#include "serial.h"
#include "util.h"

/* datacorr
  
   dataframe correction: STX, ETX and ENQ are masqued in the
   raw frame sent by the weather station. 

   function datacorr demasquerades these bytes.

   STX is masqued as ENQ DC2 in raw data
   ETX is masqued as ENQ DC3 in raw data
   ENQ is masqued as ENQ NAK in raw data

*/
int datacorr(u_char *data, int *mdat) {
    int i;
    int j;

    syslog(LOG_DEBUG, "datacorr : length dataframe : %d\n", *mdat);

    /* process all data but first and last byte */
    syslog(LOG_DEBUG, "datacorr : comparing data\n");
    for ( i = 1; i < *mdat - 2; i++ ) {
        syslog(LOG_DEBUG, "datacorr : i %d: data[i] %x: data[i+1] %x\n", i, data[i], data[i+1]);

        if ( data[i] == ENQ && data[i+1] == DC2 ) {
            syslog(LOG_INFO, "datacorr : STX found\n");
            data[i] = STX;
            for ( j = i+1; j < *mdat - 1; j++ ) data[j] = data[j + 1]; 
            *mdat -=1;
        }
        if ( data[i] == ENQ && data[i+1] == DC3 ) {
            syslog(LOG_INFO, "datacorr : ETX found\n");
            data[i] = ETX;
            for ( j = i+1; j < *mdat - 1; j++ ) data[j] = data[j + 1]; 
            *mdat -= 1; 
        }
        if ( data[i] == ENQ && data[i+1] == NAK ) {
            syslog(LOG_INFO, "datacorr : ENQ found\n");
            data[i] = ENQ;
            for ( j = i+1; j < *mdat - 1; j++ ) data[j] = data[j + 1]; 
            *mdat -= 1;
        }

    }
    syslog(LOG_INFO, "datacorr : data length after corr :%d\n", *mdat);
    return(0);
} 


/* chkframe
  
   dataframe consistency checks:

   STX 1st byte in dataframe
   ETX last byte in dataframe
   framelength OK
   messagelength OK
   check NAK received.

*/
int chkframe(u_char *data, int *mdat) {
    int i;
    int chksum;
    int ldat;
    char *nakfram = "\x02\x01\x15\xe8\x03";

    ldat = *mdat;

  
    syslog(LOG_INFO, "chkframe\n");

    /* check fist byte is STX */
    if ( data[0]      == STX ) 
	syslog(LOG_INFO, "chkframe : STX OK\n");

    /* check last byte is ETX */
    if ( data[ldat-1] == ETX ) 
	syslog(LOG_INFO, "chkframe : ETX OK\n");

    /* check length of dataframe :

       mdat is length of data array
       calculate length by function wstrlen and compare both integers
    */
    if ( wstrlen(data) == ldat ) 
	syslog(LOG_INFO, "chkframe : frame length OK\n");


    /* before calculating the checksum the dataframe 
       has to corrected for masqueraded STX, ETX and ENQ 

       don't know what happens if checksum equals STX, ETX or ENQ
       it should be masqueraded in the dataframe

    */
    datacorr(data, &ldat);

    /* calculate chksum */
    chksum = 0;
    for ( i = 0; i < ldat - 2; i++ ) {
	chksum = chksum + data[i];
        syslog(LOG_DEBUG, "chkframe : data %d: chksum %d\n",data[i], 256 - chksum);
    }
    chksum = 256 - chksum;
    if ( chksum == data[ldat - 2] ) syslog(LOG_INFO, "chkframe : checksum match OK\n");


    /* check length of data string 

       length of message is stored in data[1] , 2nd byte   
       length of dataframe is mdat 
     
       dataframe format is <STX><length><message><chksum><ETX>

       i.e. length of dataframe = 4 + length of message 
    */

        
    syslog(LOG_DEBUG, "chkframe : length of data :%d\n", data[1]);
    syslog(LOG_DEBUG, "chkframe : length dataframe : %d\n",ldat);
   

    if ( data[1] == (ldat - 4) ) syslog(LOG_INFO, "chkframe : message length OK\n"); 

    /* check if NAK dataframe has been received */
    if ( !strcmp(data,nakfram) ) {
	syslog(LOG_EMERG, "chkframe : NAK received : faulty data reception\n");
	printf("Weatherstation response : <NAK> : faulty data reception\n");
    }
    *mdat = ldat;
    return(0);
} 



/* getrd

   get raw data
*/
int getrd (char *data, int *mdat, struct cmd *pcmd) {
    int err;

    /* read answer of weather station from serial port */
    if ( pcmd->netflg == 0 ) {
	  if ( ( err = getsrd(data, mdat, pcmd)) == -1)
		return(-1);
    }
    else if (pcmd->netflg == 1 ) {
	  if ( ( err = getnrd(data, mdat, pcmd)) == -1)
		return(-1);
    }
    return(0);
}



/* getcd

   get corrected data
*/
int getcd (char *data, int *mdat, struct cmd *pcmd) {
    int i;
    int err;

    /* read answer of weather station from serial port */
    if ( ( err = getrd(data, mdat, pcmd)) == -1)
	  return(-1);
    
    /* echo raw dataframe */
    err = echodata(data, *mdat);
    
    /* check data frame and do 
    	 data correction for masqueraded bytes */
    err = chkframe(data, mdat);
    
    /* echo raw demasqueraded dataframe */
    err = echodata(data, *mdat);

    for ( i = 0; i < *mdat - 4; i++ ) {
        data[i] = data[i+2];
    }
    *mdat = *mdat - 4;

    syslog(LOG_INFO, "getcd : Data length getcd : %d\n", *mdat);
    return(0);
}



/* wstat 

   return status of weather station
   commandword is 5

*/
int wstat(char *data, int mdat, struct DCFstruct *DCF, struct sensor sens[], struct wstatus *setting) {
    int i;
    int rlen = 21;

    if ( mdat == rlen ) syslog(LOG_INFO, "wstat : status response length OK\n");

    /* status of first 8 temperature/humidity sensors */
    for ( i = 0; i < 8; i++) {
        sens[2*i].status   = data[i];
        sens[2*i+1].status = data[i];
	syslog(LOG_INFO, "wstat : status sensor %d : %d\n", i, data[i]);
    }
    
    /* rain sensor */
    sens[16].status  = data[8];
    /* wind sensor */
    sens[17].status  = data[9];
    sens[18].status  = data[9];
    /* indoor sensor : temp, hum, press */
    sens[19].status  = data[10];
    sens[20].status  = data[10];
    sens[21].status  = data[10];
        
    /* status of temperature/humidity sensor 9 to 15 */
    for ( i = 11; i < 18; i++) {
        sens[2*i].status   = data[i];
        sens[2*i+1].status = data[i];
	syslog(LOG_INFO, "wstat : status sensor %d : %d\n", i, data[i]);
    }

    setting->intv = data[18];
    syslog(LOG_INFO, "wstat : Intervall time [min] : %d\n", data[18]);
    syslog(LOG_INFO, "wstat : DCF Status   Bit 0 : %d\n", getbits(data[19],0,1));
    syslog(LOG_INFO, "wstat : HF Status    Bit 1 : %d\n", getbits(data[19],1,1));
    syslog(LOG_INFO, "wstat : 8/16 Sensor  Bit 2 : %d\n", getbits(data[19],2,1));
    syslog(LOG_INFO, "wstat : DCF sync     Bit 3 : %d\n", getbits(data[19],3,1));
    
    syslog(LOG_INFO, "wstat : bit 4  : %d\n", getbits(data[19],4,1));
    syslog(LOG_INFO, "wstat : bit 5  : %d\n", getbits(data[19],5,1));
    syslog(LOG_INFO, "wstat : bit 6 : %d\n", getbits(data[19],6,1));
    syslog(LOG_INFO, "wstat : bit 7 : %d\n", getbits(data[19],7,1));



    /* DCF status and synchronicity */
    DCF->stat = getbits(data[19],0,1);
    DCF->sync = getbits(data[19],3,1);

    /* number of sensors */
    if ( getbits(data[19],2,1) == 0 ) { 
        setting->numsens = 8;
    }
    else if ( getbits(data[19],2,1) == 1 ) {
        setting->numsens = 16;
    }
        
    /* version number */
    setting->version = data[20];
    
    return 0;
}


/* dcftime 

   calculate time from weather station response to "poll DCF time"
   returns 
     seconds sinche EPOCH, if DCF is synchronized 
     -1, if DCF not synchronized.
   
*/
int dcftime(u_char *data, int ndat) {
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

    err = time(&ltim);
    clk = ctime(&ltim);
    syslog(LOG_DEBUG, "dcftime : local clk : %s\n", clk);
    
    dtim = mktime(&t);
    clk = ctime(&dtim);
    syslog(LOG_DEBUG, "dcftime : dtim : %lu\n", dtim);
    syslog(LOG_INFO, "dcftime : DCF clk : %s\n", clk);

    return(dtim);
}

/* getone 
   
   stores the message datagram of the weather station to the structure sens

*/
int getone(char *data, int ndat, struct sensor sens[], long setno, struct DCFstruct DCF) {
    int i;
    int j;
    int err;
    int mbit;
    int nbit;
    int Hi, Lo;
    long age;
    time_t mtim;
    char *clk;

    syslog(LOG_INFO, "getone : ndat in getone : %d\n", ndat);

    /* block number */
    syslog(LOG_INFO, "getone : block NO (byte 1) : %d\n", data[0]);
    syslog(LOG_INFO, "getone : block NO (byte 2) : %d\n", data[1]);

    /* age of dataset in minutes up to current time */
    syslog(LOG_INFO, "getone : age (byte 1) : %d\n", data[2]);
    syslog(LOG_INFO, "getone : age (byte 2) : %d\n", data[3]);
    age = ( data[2] & 0xff )  + ( data[3] << 8 & 0xff00 );


    /* convert into seconds and subtract from elapsed seconds since EPOCH */
    age = age * 60;


    /* time of dataset 
       check if DCF synchronized:
       if yes, use this timevalue
       if no, use localtime
    */
    if ( DCF.time != -1 ) {
	mtim = DCF.time;
    }
  
    else { 
	syslog(LOG_ALERT, "getone : DCF not synchronized, using localtime for dataset\n");
	err = time(&mtim);
	syslog(LOG_INFO, "getone : seconds since EPOCH (now): %lu\n", mtim);
    }
    

    mtim = mtim - age;
    clk = ctime(&mtim);

     
    syslog(LOG_INFO, "getone : setno : %lu; age[min] : %lu\n", setno, age);
    syslog(LOG_INFO, "getone : seconds since EPOCH (dataset): %lu\n", mtim);
    syslog(LOG_INFO, "getone : measured at clk : %s\n", clk);

    /* get data of the first 8 temperature/humidity sensors */
    for ( i = 0; i < 8; i++) {
        j = i % 2;
        /* even i */
        if ( (i % 2) == 0 ) {
	    mbit=3; nbit=7;
	    j = (5*i - j)/2 + 4;
            /* temperature */
            if ( sens[2*i].status != 0 ) {
		sens[2*i].mess[setno].time   = mtim;
		sens[2*i].mess[setno].hundreds = 0;
		sens[2*i].mess[setno].tenths = getbits(data[j], mbit, 4);
		sens[2*i].mess[setno].units  = getbits(data[j], nbit, 4);
		sens[2*i].mess[setno].tens   = getbits(data[j+1], mbit-1, 3);
		sens[2*i].mess[setno].sign   = getbits(data[j+1], mbit, 1);
	    }

            /* humidity 
               the value is calculated from an 8bit Byte which is formed
               by two Nibbles    
            */
            if ( sens[2*i+1].status != 0 ) {
		Lo = getbits(data[j+1], nbit, 4);
		Hi = getbits(data[j+2], mbit-1, 3) << 4;
		sens[2*i+1].mess[setno].time   = mtim;
		sens[2*i+1].mess[setno].hundreds = 0;
		sens[2*i+1].mess[setno].tenths = 0;
		sens[2*i+1].mess[setno].tens   = (Hi + Lo)/10;
		sens[2*i+1].mess[setno].units  = (Hi + Lo)%10;
		/* Bit 3 of Hi Nibble is new flag */
		sens[2*i+1].mess[setno].sign   = getbits(data[j+2], mbit, 1);
	    }

        } /* odd i */ else if ( (i % 2) == 1) {
	    mbit=7; nbit=3;
	    j = (5*i - j)/2 + 4;

            /* temperature */
            if ( sens[2*i].status != 0 ) {
		sens[2*i].mess[setno].time   = mtim;
		sens[2*i].mess[setno].hundreds = 0;
		sens[2*i].mess[setno].tenths  = getbits(data[j], mbit, 4);
		sens[2*i].mess[setno].units   = getbits(data[j+1], nbit, 4);
		sens[2*i].mess[setno].tens    = getbits(data[j+1], mbit-1, 3);
		sens[2*i].mess[setno].sign    = getbits(data[j+1], mbit, 1);
	    }
            /* humidity 
               the value is calculated from an 8bit Byte which is formed
               by two Nibbles    
            */
            if ( sens[2*i+1].status != 0 ) {
		Lo = getbits(data[j+2], nbit, 4);
		Hi = getbits(data[j+2], mbit-1, 3) << 4;
		sens[2*i+1].mess[setno].time   = mtim;
		sens[2*i+1].mess[setno].hundreds = 0;
		sens[2*i+1].mess[setno].tenths = 0;
		sens[2*i+1].mess[setno].tens   = (Hi + Lo)/10;
		sens[2*i+1].mess[setno].units  = (Hi + Lo)%10;
		/* Bit 3 of Hi Nibble is new flag */
		sens[2*i+1].mess[setno].sign   = getbits(data[j+2], mbit, 1);
	    }
	}

    }


    /* rain sensor */
    /* no data available yet */    

    /* wind speed */
    sens[17].mess[setno].time   = mtim;
    sens[17].mess[setno].sign   = 0;
    sens[17].mess[setno].hundreds = getbits(data[27], 6, 3);
    sens[17].mess[setno].tens     = getbits(data[27], 3, 4);
    sens[17].mess[setno].units    = getbits(data[26], 6, 3);
    sens[17].mess[setno].tenths   = getbits(data[26], 3, 4);


    /* wind direction */
    sens[18].mess[setno].time     = mtim;
    sens[18].mess[setno].sign     = 0;
    /* sens[18].mess[setno].hundreds = getbits(data[29], 3, 4); */
    /* hmm is this a violation of the protocol specification ? */
    sens[18].mess[setno].hundreds = data[29] & 0x03;
    sens[18].mess[setno].tens     = getbits(data[28], 7, 4);
    sens[18].mess[setno].units    = getbits(data[28], 3, 4);
    sens[18].mess[setno].tenths   = 0;


    /* indoor pressure */
    sens[19].mess[setno].time     = mtim;
    sens[19].mess[setno].sign     = 0;
    sens[19].mess[setno].hundreds = getbits(data[30], 7, 4);
    sens[19].mess[setno].tens     = getbits(data[30], 3, 4);
    sens[19].mess[setno].units    = getbits(data[29], 7, 4);
    sens[19].mess[setno].tenths   = 0;


    /* indoor temperature */
    sens[20].mess[setno].time     = mtim;
    sens[20].mess[setno].sign     = 0;
    sens[20].mess[setno].hundreds = 0;
    sens[20].mess[setno].tens     = getbits(data[32], 3, 4);
    sens[20].mess[setno].units    = getbits(data[31], 7, 4);
    sens[20].mess[setno].tenths   = getbits(data[31], 7, 4);


    /* indoor humidity */
    Lo = getbits(data[32], 7, 4);
    Hi = getbits(data[33], 2, 3) << 4;
    sens[21].mess[setno].time     = mtim;
    sens[21].mess[setno].sign     = 0;
    sens[21].mess[setno].hundreds = 0;
    sens[21].mess[setno].tens     = (Hi + Lo)/10;
    sens[21].mess[setno].units    = (Hi + Lo)%10;
    sens[21].mess[setno].tenths   = 0;
    return(0);
};


/* pdata 

   print the sensor data as hold in structure sens

*/
int pdata(struct sensor sens[], int snum) {
    int i,j;         /* array indices of sensor and setno */

    printf("Sensor\tType\tStatus\tdataset\ttime\t\t\tsign\tXYZ.t\n");

    for ( i = 0; i < MAXSENSORS; i++ ) {
        if ( sens[i].status != 0 ) {   
	    for ( j = 0; j < snum; j++) {
		printf("%d\t%s\t%d\t%d\t%lu\t\t%d\t%d%d%d.%d\n", i, \
                sens[i].type, sens[i].status, j, sens[i].mess[j].time, \
                sens[i].mess[j].sign, sens[i].mess[j].hundreds, sens[i].mess[j].tens, \
                sens[i].mess[j].units, sens[i].mess[j].tenths);
	    }
        }
    }
    return(0);
}


/* wcmd

   function fills the datastructure sens

*/
int wcmd (struct sensor sens[], struct cmd *pcmd, int *setno) {
   
  int i;                          /* for array subscription */
  int ndat = 0;                   /* length of array containing the 
                                     data frame */
  int err;                        /* return value of functions, 
                                     useful for errorhandling */
  int retval = 0;                 /* return values for this function */
                                  /*  0 : everythings fine */
                                  /*  1 : no data available */
                                  /* -1 : unknown response of weatherstation */  
  int snum = 0;                   /* number of data sets */
  int command;
  int argcm;

  struct DCFstruct DCF;           /* DCF status and synchronous */
  struct wstatus setting;

  u_char data[MAXBUFF];           /* data array to store the raw dataframe 
                                     and the message datagram */
  char *clk;                      /* display time in reasonable format */

  command = pcmd->command; 
  argcm   = pcmd->argcmd; 

  /* first get status of weatherstation 
     needed to fill sens.status
  */
  pcmd->command = 5;
  if ( ( err = getcd( data, &ndat, pcmd)) == -1)
	return(-1);
  pcmd->command = command;

  syslog(LOG_INFO, "wcmd : check status OK\n");

  /* status weatherstation */
  err = wstat(data, ndat, &DCF, sens, &setting);
  syslog(LOG_INFO, "wcmd : DCF.stat : %d\n", DCF.stat);
  syslog(LOG_INFO, "wcmd : DCF.sync : %d\n", DCF.sync);
  syslog(LOG_INFO, "wcmd : number sensors : %d\n", setting.numsens);
  syslog(LOG_INFO, "wcmd : version : %x\n", setting.version);
  for ( i = 0;  i < MAXSENSORS; i++ ) {
    syslog(LOG_DEBUG, "wcmd : sens[%d].status: %d, type: %s\n", i, sens[i].status, sens[i].type);
  }


  /* command 0 : poll DCF time */
  if (command == 0) {
      /* write command and retrieve data */
	  if ( ( err = getcd( data, &ndat, pcmd)) == -1)
	     return(-1);

      /* calculate seconds since EPOCH if DCF synchronized */
      DCF.time  = dcftime(data, ndat);
      if (DCF.time == -1) 
		printf("Weatherstation not synchronized\n");
      else { 
		clk = ctime(&DCF.time);
		printf("%s", clk);
      }
  }

  /* command 1 : Request Dataset */
  else if (command == 1)  {

      /* first get DCF time if possible */
      pcmd->command = 0;
      if ( ( err = getcd( data, &ndat, pcmd)) == -1)
		   return(-1);
      pcmd->command = command;

      /* calculate seconds since EPOCH if DCF synchronized */
      DCF.time  = dcftime(data, ndat);

      /* write command and retrieve data */
      if ( ( err = getcd( data, &ndat, pcmd)) == -1)
	     return(-1);

      /* weather station response : no data available: <DLE> */
      if ( ( ndat == 1 ) && ( data[0] == DLE ) ) {
	  syslog(LOG_INFO, "wcmd : DLE received : weather station :\"no data available\": exit!\n");
	  retval = 16;
      }
      /* fill data structure sens */
      else {
	  /* get one dataset */
	  err = getone(data, ndat, sens, snum, DCF);
	  syslog(LOG_INFO, "wcmd : returncode getone : %d\n", err);
          snum++;
      }

      /* echo sensor data */
      err = pdata(sens, snum);
  } 


  /* command 2 : Select next dataset */
  else if (command == 2) {
      /* write the command word to the weather station */ 
      /* extract message datagram */
	  if ( ( err = getcd( data, &ndat, pcmd)) == -1)
		return(-1);

      /* if DLE no data available */
      if ( ( ndat == 1 ) && ( data[0] == DLE ) ) {
	  syslog(LOG_INFO, \
              "wcmd : DLE received : weather station :\"no data available\"!\n");
          retval = 16; 
      } else if ( ( ndat == 1 ) && ( data[0] == ACK ) ) /* if ACK next dataset is available */ {
	  syslog(LOG_INFO, \
             "wcmd : ACK received : weather station :\"next dataset available\": exit!\n");
          retval = 6;
      }
      /* exit if unknown response */
      else {
	  syslog(LOG_EMERG, "wcmd : error request next dataset : unknown response: exit \n");
          retval = -1;
      }
  }


  /* command 3 : Activate 8 temperature sensors */
  else if (command == 3) {

      /* write the command word to the weather station */ 
      /* extract message datagram */
	  if ( ( err = getcd( data, &ndat, pcmd)) == -1)
		return(-1);

      /* weather station response : <ACK> */
      if ( ( ndat == 1 ) && ( data[0] == ACK ) ) {
          syslog(LOG_INFO, "wcmd : ACK received\n");
          printf("Weatherstation response : <ACK>\n");
      }

  } 


  /* command 4 : Activate 16 temperature sensors */
  else if (command == 4) {

      /* write the command word to the weather station */ 
      /* extract message datagram */
	  if ( ( err = getcd( data, &ndat, pcmd)) == -1)
		return(-1);

      /* weather station response : <ACK> */
      if ( ( ndat == 1 ) && ( data[0] == ACK ) ) {
          syslog(LOG_INFO, "wcmd : ACK received\n");
          printf("Weatherstation response : <ACK>\n");
      }

  } 


  /* command 5 : Request status of weatherstation */
  else if ( command == 5 ) {
      /* check status - this is done at every call of wcmd*/
	  if ( ( err = getcd( data, &ndat, pcmd)) == -1)
		return(-1);

      /* status weatherstation */
      err = wstat(data, ndat, &DCF, sens, &setting);

      syslog(LOG_INFO, "wcmd : DCF.stat : %d\n", DCF.stat);
      syslog(LOG_INFO, "wcmd : DCF.sync : %d\n", DCF.sync);
      syslog(LOG_INFO, "wcmd : number sensors : %d\n", setting.numsens);
      syslog(LOG_INFO, "wcmd : version : %x\n", setting.version);


      for ( i = 0;  i < MAXSENSORS; i++ ) {
	  syslog(LOG_DEBUG, "wcmd : sens[%d].status: %d, type: %s\n", i, sens[i].status, sens[i].type);
      }

      printf("Status weatherstation\n");
      printf("Version : %x\n", setting.version);  
      printf("Interval : %d\n", setting.intv);  
      printf("Numsens : %d\n", setting.numsens);  
      printf("DCF.stat : %d\n", DCF.stat);
      printf("DCF.sync : %d\n", DCF.sync);

  } 


  /* command 6 : Set logging intervall of weatherstation */
  else if (command == 6) {

      /* write the command word to the weather station */ 
      /* extract message datagram */
	  if ( ( err = getcd( data, &ndat, pcmd)) == -1)
	    return(-1);

      /* weather station response : <ACK> */
      if ( ( ndat == 1 ) && ( data[0] == ACK ) ) {
          syslog(LOG_INFO, "wcmd : ACK received\n");
          printf("Weatherstation response : <ACK>\n");
      }

  } 


  /* command 12 : 
      Recursively, request dataset and select next dataset, 
      recursive combination of command 1 and 2 
  */
  else if (command == 12) {

      
      /* first get DCF time if possible */
      pcmd->command = 0;
      if ( ( err = getcd( data, &ndat, pcmd)) == -1)
		return(-1);
      pcmd->command = command;

      /* calculate seconds since EPOCH if DCF synchronized */
      DCF.time  = dcftime(data, ndat);

      while (retval != 16) {
		  /* write command and retrieve data */
		  pcmd->command = 1;
		  if ( ( err = getcd( data, &ndat, pcmd)) == -1)
		    return(-1);

		  /* weather station response : no data available: <DLE> */
		  if ( ( ndat == 1 ) && ( data[0] == DLE ) ) {
			syslog(LOG_INFO, \
              "wcmd : DLE received : weather station :\"no data available\"!\n");
			retval = 16;
		  }
		  /* fill data structure sens */
		  else {
			/* get one dataset */
			err = getone(data, ndat, sens, snum, DCF);
            syslog(LOG_INFO, "wcmd : returncode getone : %d\n", err);

			/* increase dataset number index */
			snum++;
		  }
 
		  /* write the command word to select next dataset and retrieve response*/ 
		  pcmd->command = 2;
		  
		  if ( ( err = getcd( data, &ndat, pcmd)) == -1)
			return(-1);

		  /* if DLE no data available */
		  if ( ( ndat == 1 ) && ( data[0] == DLE ) ) {
			syslog(LOG_INFO, \
	          "wcmd : DLE received : weather station :\"no data available\"!\n");

			printf("DLE received : weather station :\"no data available\"!\n");
			retval = 16; 
		  }
		  /* if ACK next dataset is available */
		  else if ( ( ndat == 1 ) && ( data[0] == ACK ) ) {
			syslog(LOG_INFO, \
                  "wcmd : ACK received : weather station :\"next dataset available\"!\n");
			retval = 6;
		  }
		  /* exit if unknown response */
		  else {
              syslog(LOG_EMERG, \
	          "wcmd : error request next dataset : unknown response: exit \n");
			  retval = -2;
		  }
          syslog(LOG_INFO, "wcmd : retval : %d\n", retval);
      }

      /* echo sensor data */
      err = pdata(sens, snum);
  } else {

      printf("Unknown command: exit!\n");
      exit(1);
  }

  *setno = snum;
  syslog(LOG_INFO, "wcmd : returncode wcmd : %d, number of sets : %d\n", retval, *setno);
 
  return(retval);

}



/* initsens 

   presets the datastructure sens

*/
int initsens(struct sensor sens[]) {
  int i;  /* array subscription */  

  /* set type of the sensors */
  /* first 8 temperature/humidity sensors */
  for ( i = 0; i < 8; i++) {
      sens[2*i].type   = "temp";
      sens[2*i+1].type = "hum";
  }
  
  /* rain sensor */
  sens[16].type  = "rain";
  /* wind sensor */
  sens[17].type  = "wspd";
  sens[18].type  = "wdir";
  /* indoor sensor : temp, hum, press */
  sens[19].type  = "pres";
  sens[20].type  = "temp";
  sens[21].type  = "hum";
      
  /* status of temperature/humidity sensor 9 to 15 */
  for ( i = 11; i < 18; i++) {
      sens[2*i].type   = "temp";
      sens[2*i+1].type = "hum";
  }

  return(0);
}

