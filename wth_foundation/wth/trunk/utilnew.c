/* utilnew.c

   $Id$
   $Revision$

   Copyright 2007 Volker Jahns <volker@thalreit.de>

   collection of utility functions

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


/* initdata 

   set initial values of global datastructures

   ws2000station
   pcwsrstation
   wsconf
 
*/
int initdata( ) {
  /* default parameters valid for all stations */
  wsconf.timeout     = 30;
  wsconf.logfacility = LOG_LOCAL7;
  wsconf.verbose     = 1;
  wsconf.debug       = 1;
  wsconf.netflg      = 0;
  wsconf.hostname    = "localhost";
  wsconf.port        = "5001";
  wsconf.tnport      = "5002";
  wsconf.xmlport     = "8001";
  wsconf.wwwport     = "8880";
  wsconf.units       = "SI";
  wsconf.outfmt      = "old";
  
  /* ws2000 weatherstations sensors */
  ws2000station.sensor[0].sensorname = "Sensor1";
  ws2000station.sensor[0].rrdfile    = "Sensor1.rrd";
  ws2000station.sensor[0].param[0].paramname ="Temp1";
  ws2000station.sensor[0].param[1].paramname ="Humi1";
  ws2000station.sensor[1].sensorname = "Sensor2";
  ws2000station.sensor[1].rrdfile    = "Sensor2.rrd";
  ws2000station.sensor[1].param[0].paramname ="Temp2";
  ws2000station.sensor[1].param[1].paramname ="Humi2";
  ws2000station.sensor[2].sensorname = "Sensor3";
  ws2000station.sensor[2].rrdfile    = "Sensor3.rrd";
  ws2000station.sensor[2].param[0].paramname ="Temp3";
  ws2000station.sensor[2].param[1].paramname ="Humi3";
  ws2000station.sensor[3].sensorname = "Sensor4";
  ws2000station.sensor[3].rrdfile    = "Sensor4.rrd";
  ws2000station.sensor[3].param[0].paramname ="Temp4";
  ws2000station.sensor[3].param[1].paramname ="Humi4";
  ws2000station.sensor[4].sensorname = "Sensor5";
  ws2000station.sensor[4].rrdfile    = "Sensor5.rrd";
  ws2000station.sensor[4].param[0].paramname ="Temp5";
  ws2000station.sensor[4].param[1].paramname ="Humi5";
  ws2000station.sensor[5].sensorname = "Sensor6";
  ws2000station.sensor[5].rrdfile    = "Sensor6.rrd";
  ws2000station.sensor[5].param[0].paramname ="Temp6";
  ws2000station.sensor[5].param[1].paramname ="Humi6";
  ws2000station.sensor[6].sensorname = "Sensor7";
  ws2000station.sensor[6].rrdfile    = "Sensor7.rrd";
  ws2000station.sensor[6].param[0].paramname ="Temp7";
  ws2000station.sensor[6].param[1].paramname ="Humi7";
  ws2000station.sensor[7].sensorname = "Sensor8";
  ws2000station.sensor[7].rrdfile    = "Sensor8.rrd";
  ws2000station.sensor[7].param[0].paramname ="Temp8";
  ws2000station.sensor[7].param[1].paramname ="Humi8";
  ws2000station.sensor[8].sensorname = "Rain";
  ws2000station.sensor[8].rrdfile    = "Rain.rrd";
  ws2000station.sensor[8].param[0].paramname ="RainVol";
  ws2000station.sensor[8].param[1].paramname ="RainNew";
  ws2000station.sensor[9].sensorname = "Wind";
  ws2000station.sensor[9].rrdfile    = "Wind.rrd";
  ws2000station.sensor[9].param[0].paramname ="WindSpeed";
  ws2000station.sensor[9].param[1].paramname ="WindDirection";
  ws2000station.sensor[9].param[2].paramname ="WindDeviation";
  ws2000station.sensor[10].sensorname = "Indoor";
  ws2000station.sensor[10].rrdfile    = "Indoor.rrd";
  ws2000station.sensor[10].param[0].paramname ="PressureI";
  ws2000station.sensor[10].param[1].paramname ="TempI";
  ws2000station.sensor[10].param[2].paramname ="HumiI";
  ws2000station.sensor[11].sensorname = "Sensor9";
  ws2000station.sensor[11].rrdfile    = "Sensor9.rrd";
  ws2000station.sensor[11].param[0].paramname ="Pressure9";
  ws2000station.sensor[11].param[1].paramname ="Temp9";
  ws2000station.sensor[11].param[2].paramname ="Humi9";
  ws2000station.sensor[12].sensorname = "Sensor10";
  ws2000station.sensor[12].rrdfile    = "Sensor10.rrd";
  ws2000station.sensor[12].param[0].paramname ="Pressure10";
  ws2000station.sensor[12].param[1].paramname ="Temp10";
  ws2000station.sensor[12].param[2].paramname ="Humi10";
  ws2000station.sensor[13].sensorname = "Sensor11";
  ws2000station.sensor[13].rrdfile    = "Sensor11.rrd";
  ws2000station.sensor[13].param[0].paramname ="Pressure11";
  ws2000station.sensor[13].param[1].paramname ="Temp11";
  ws2000station.sensor[13].param[2].paramname ="Humi11";
  ws2000station.sensor[14].sensorname = "Sensor12";
  ws2000station.sensor[14].rrdfile    = "Sensor12.rrd";
  ws2000station.sensor[14].param[0].paramname ="Pressure12";
  ws2000station.sensor[14].param[1].paramname ="Temp12";
  ws2000station.sensor[14].param[2].paramname ="Humi12";
  ws2000station.sensor[15].sensorname = "Sensor13";
  ws2000station.sensor[15].rrdfile    = "Sensor13.rrd";
  ws2000station.sensor[15].param[0].paramname ="Pressure13";
  ws2000station.sensor[15].param[1].paramname ="Temp13";
  ws2000station.sensor[15].param[2].paramname ="Humi13";
  ws2000station.sensor[16].sensorname = "Sensor14";
  ws2000station.sensor[16].rrdfile    = "Sensor14.rrd";
  ws2000station.sensor[16].param[0].paramname ="Pressure14";
  ws2000station.sensor[16].param[1].paramname ="Temp14";
  ws2000station.sensor[16].param[2].paramname ="Humi14";
  ws2000station.config.dbfile        = "ws2000.db";
  ws2000station.config.device        = "/dev/ttyd0";
  ws2000station.status.interval      = 300;  

  /* pcwsr sensors */
  /* T outdoor sensor V1.1 */
  pcwsrstation.sensor[0x00].sensorname  = "T_OutdoorSensor1V1";
  pcwsrstation.sensor[0x00].rrdfile     = "T_OutdoorSensor1V1.rrd";
  pcwsrstation.sensor[0x00].address     = 0x00;
  pcwsrstation.sensor[0x00].param[0].paramname = "Temperature";

  pcwsrstation.sensor[0x01].sensorname  = "T_OutdoorSensor2V1";
  pcwsrstation.sensor[0x01].rrdfile     = "T_OutdoorSensor2V1.rrd";
  pcwsrstation.sensor[0x01].address     = 0x01;
  pcwsrstation.sensor[0x01].param[0].paramname = "Temperature";

  pcwsrstation.sensor[0x02].sensorname  = "T_OutdoorSensor3V1";
  pcwsrstation.sensor[0x02].rrdfile     = "T_OutdoorSensor3V1.rrd";
  pcwsrstation.sensor[0x02].address     = 0x02;
  pcwsrstation.sensor[0x02].param[0].paramname = "Temperature";

  pcwsrstation.sensor[0x03].sensorname  = "T_OutdoorSensor4V1";
  pcwsrstation.sensor[0x03].rrdfile     = "T_OutdoorSensor4V1.rrd";
  pcwsrstation.sensor[0x03].address     = 0x03;
  pcwsrstation.sensor[0x03].param[0].paramname = "Temperature";

  pcwsrstation.sensor[0x04].sensorname  = "T_OutdoorSensor5V1";
  pcwsrstation.sensor[0x04].rrdfile     = "T_OutdoorSensor5V1.rrd";
  pcwsrstation.sensor[0x04].address     = 0x04;
  pcwsrstation.sensor[0x04].param[0].paramname = "Temperature";

  pcwsrstation.sensor[0x05].sensorname  = "T_OutdoorSensor6V1";
  pcwsrstation.sensor[0x05].rrdfile     = "T_OutdoorSensor6V1.rrd";
  pcwsrstation.sensor[0x05].address     = 0x05;
  pcwsrstation.sensor[0x05].param[0].paramname = "Temperature";

  pcwsrstation.sensor[0x06].sensorname  = "T_OutdoorSensor7V1";
  pcwsrstation.sensor[0x06].rrdfile     = "T_OutdoorSensor7V1.rrd";
  pcwsrstation.sensor[0x06].address     = 0x06;
  pcwsrstation.sensor[0x06].param[0].paramname = "Temperature";

  pcwsrstation.sensor[0x07].sensorname  = "T_OutdoorSensor8V1";
  pcwsrstation.sensor[0x07].rrdfile     = "T_OutdoorSensor8V1.rrd";
  pcwsrstation.sensor[0x07].address     = 0x07;
  pcwsrstation.sensor[0x07].param[0].paramname = "Temperature";
 
  /* T outdoor sensor V1.2 */
  pcwsrstation.sensor[0x08].sensorname  = "T_OutdoorSensor1V2";
  pcwsrstation.sensor[0x08].rrdfile     = "T_OutdoorSensor1.rrd";
  pcwsrstation.sensor[0x08].address     = 0x08;
  pcwsrstation.sensor[0x08].param[0].paramname = "Temperature";

  pcwsrstation.sensor[0x09].sensorname  = "T_OutdoorSensor2V2";
  pcwsrstation.sensor[0x09].rrdfile     = "T_OutdoorSensor2V2.rrd";
  pcwsrstation.sensor[0x09].address     = 0x09;
  pcwsrstation.sensor[0x09].param[0].paramname = "Temperature";

  pcwsrstation.sensor[0x0a].sensorname  = "T_OutdoorSensor3V2";
  pcwsrstation.sensor[0x0a].rrdfile     = "T_OutdoorSensor3V2.rrd";
  pcwsrstation.sensor[0x0a].address     = 0x0a;
  pcwsrstation.sensor[0x0a].param[0].paramname = "Temperature";

  pcwsrstation.sensor[0x0b].sensorname  = "T_OutdoorSensor4V2";
  pcwsrstation.sensor[0x0b].rrdfile     = "T_OutdoorSensor4V2.rrd";
  pcwsrstation.sensor[0x0b].address     = 0x0b;
  pcwsrstation.sensor[0x0b].param[0].paramname = "Temperature";

  pcwsrstation.sensor[0x0c].sensorname  = "T_OutdoorSensor5V2";
  pcwsrstation.sensor[0x0c].rrdfile     = "T_OutdoorSensor5V2.rrd";
  pcwsrstation.sensor[0x0c].address     = 0x0c;
  pcwsrstation.sensor[0x0c].param[0].paramname = "Temperature";

  pcwsrstation.sensor[0x0d].sensorname  = "T_OutdoorSensor6V2";
  pcwsrstation.sensor[0x0d].rrdfile     = "T_OutdoorSensor6V2.rrd";
  pcwsrstation.sensor[0x0d].address     = 0x0d;
  pcwsrstation.sensor[0x0d].param[0].paramname = "Temperature";

  pcwsrstation.sensor[0x0e].sensorname  = "T_OutdoorSensor7V2";
  pcwsrstation.sensor[0x0e].rrdfile     = "T_OutdoorSensor7V2.rrd";
  pcwsrstation.sensor[0x0e].address     = 0x0e;
  pcwsrstation.sensor[0x0e].param[0].paramname = "Temperature";

  pcwsrstation.sensor[0x0f].sensorname  = "T_OutdoorSensor8V2";
  pcwsrstation.sensor[0x0f].rrdfile     = "T_OutdoorSensor8V2.rrd";
  pcwsrstation.sensor[0x0f].address     = 0x0f;
  pcwsrstation.sensor[0x0f].param[0].paramname = "Temperature";

  /* T/H outdoor sensor V1.1 */
  pcwsrstation.sensor[0x10].sensorname  = "TH_OutdoorSensor1V1";
  pcwsrstation.sensor[0x10].rrdfile     = "TH_OutdoorSensor1V1.rrd";
  pcwsrstation.sensor[0x10].address     = 0x10;
  pcwsrstation.sensor[0x10].param[0].paramname = "Temperature";
  pcwsrstation.sensor[0x10].param[1].paramname = "Humidity";

  pcwsrstation.sensor[0x11].sensorname  = "TH_OutdoorSensor2V1";
  pcwsrstation.sensor[0x11].rrdfile     = "TH_OutdoorSensor2V1.rrd";
  pcwsrstation.sensor[0x11].address     = 0x11;
  pcwsrstation.sensor[0x11].param[0].paramname = "Temperature";
  pcwsrstation.sensor[0x11].param[1].paramname = "Humidity";

  pcwsrstation.sensor[0x12].sensorname  = "TH_OutdoorSensor3V1";
  pcwsrstation.sensor[0x12].rrdfile     = "TH_OutdoorSensor3V1.rrd";
  pcwsrstation.sensor[0x12].address     = 0x12;
  pcwsrstation.sensor[0x12].param[0].paramname = "Temperature";
  pcwsrstation.sensor[0x12].param[1].paramname = "Humidity";

  pcwsrstation.sensor[0x13].sensorname  = "TH_OutdoorSensor4V1";
  pcwsrstation.sensor[0x13].rrdfile     = "TH_OutdoorSensor4V1.rrd";
  pcwsrstation.sensor[0x13].address     = 0x13;
  pcwsrstation.sensor[0x13].param[0].paramname = "Temperature";
  pcwsrstation.sensor[0x13].param[1].paramname = "Humidity";

  pcwsrstation.sensor[0x14].sensorname  = "TH_OutdoorSensor5V1";
  pcwsrstation.sensor[0x14].rrdfile     = "TH_OutdoorSensor5V1.rrd";
  pcwsrstation.sensor[0x14].address     = 0x14;
  pcwsrstation.sensor[0x14].param[0].paramname = "Temperature";
  pcwsrstation.sensor[0x14].param[1].paramname = "Humidity";

  pcwsrstation.sensor[0x15].sensorname  = "TH_OutdoorSensor6V1";
  pcwsrstation.sensor[0x15].rrdfile     = "TH_OutdoorSensor6V1.rrd";
  pcwsrstation.sensor[0x15].address     = 0x15;
  pcwsrstation.sensor[0x15].param[0].paramname = "Temperature";
  pcwsrstation.sensor[0x15].param[1].paramname = "Humidity";

  pcwsrstation.sensor[0x16].sensorname  = "TH_OutdoorSensor7V1";
  pcwsrstation.sensor[0x16].rrdfile     = "TH_OutdoorSensor7V1.rrd";
  pcwsrstation.sensor[0x16].address     = 0x16;
  pcwsrstation.sensor[0x16].param[0].paramname = "Temperature";
  pcwsrstation.sensor[0x16].param[1].paramname = "Humidity";

  pcwsrstation.sensor[0x17].sensorname  = "TH_OutdoorSensor8V1";
  pcwsrstation.sensor[0x17].rrdfile     = "TH_OutdoorSensor8V1.rrd";
  pcwsrstation.sensor[0x17].address     = 0x17;
  pcwsrstation.sensor[0x17].param[0].paramname = "Temperature";
  pcwsrstation.sensor[0x17].param[1].paramname = "Humidity";

  /* T/H outdoor sensor V1.2 */ 
  pcwsrstation.sensor[0x18].sensorname  = "TH_OutdoorSensor1V2";
  pcwsrstation.sensor[0x18].rrdfile     = "TH_OutdoorSensor1.rrd";
  pcwsrstation.sensor[0x18].address     = 0x18;
  pcwsrstation.sensor[0x18].param[0].paramname = "Temperature";
  pcwsrstation.sensor[0x18].param[1].paramname = "Humidity";

  pcwsrstation.sensor[0x19].sensorname  = "TH_OutdoorSensor2V2";
  pcwsrstation.sensor[0x19].rrdfile     = "TH_OutdoorSensor2V2.rrd";
  pcwsrstation.sensor[0x19].address     = 0x19;
  pcwsrstation.sensor[0x19].param[0].paramname = "Temperature";
  pcwsrstation.sensor[0x19].param[1].paramname = "Humidity";

  pcwsrstation.sensor[0x1a].sensorname  = "TH_OutdoorSensor3V2";
  pcwsrstation.sensor[0x1a].rrdfile     = "TH_OutdoorSensor3V2.rrd";
  pcwsrstation.sensor[0x1a].address     = 0x1a;
  pcwsrstation.sensor[0x1a].param[0].paramname = "Temperature";
  pcwsrstation.sensor[0x1a].param[1].paramname = "Humidity";

  pcwsrstation.sensor[0x1b].sensorname  = "TH_OutdoorSensor4V2";
  pcwsrstation.sensor[0x1b].rrdfile     = "TH_OutdoorSensor4V2.rrd";
  pcwsrstation.sensor[0x1b].address     = 0x1b;
  pcwsrstation.sensor[0x1b].param[0].paramname = "Temperature";
  pcwsrstation.sensor[0x1b].param[1].paramname = "Humidity";

  pcwsrstation.sensor[0x1c].sensorname  = "TH_OutdoorSensor5V2";
  pcwsrstation.sensor[0x1c].rrdfile     = "TH_OutdoorSensor5V2.rrd";
  pcwsrstation.sensor[0x1c].address     = 0x1c;
  pcwsrstation.sensor[0x1c].param[0].paramname = "Temperature";
  pcwsrstation.sensor[0x1c].param[1].paramname = "Humidity";

  pcwsrstation.sensor[0x1d].sensorname  = "TH_OutdoorSensor6V2";
  pcwsrstation.sensor[0x1d].rrdfile     = "TH_OutdoorSensor6V2.rrd";
  pcwsrstation.sensor[0x1d].address     = 0x1d;
  pcwsrstation.sensor[0x1d].param[0].paramname = "Temperature";
  pcwsrstation.sensor[0x1d].param[1].paramname = "Humidity";

  pcwsrstation.sensor[0x1e].sensorname  = "TH_OutdoorSensor7V2";
  pcwsrstation.sensor[0x1e].rrdfile     = "TH_OutdoorSensor7V2.rrd";
  pcwsrstation.sensor[0x1e].address     = 0x1e;
  pcwsrstation.sensor[0x1e].param[0].paramname = "Temperature";
  pcwsrstation.sensor[0x1e].param[1].paramname = "Humidity";

  pcwsrstation.sensor[0x1f].sensorname  = "TH_OutdoorSensor8V2";
  pcwsrstation.sensor[0x1f].rrdfile     = "TH_OutdoorSensor8V2.rrd";
  pcwsrstation.sensor[0x1f].address     = 0x1f;
  pcwsrstation.sensor[0x1f].param[0].paramname = "Temperature";
  pcwsrstation.sensor[0x1f].param[1].paramname = "Humidity";
 
  pcwsrstation.config.dbfile         = "pcwsr.db";
  pcwsrstation.config.device         = "/dev/ttyd1";
 
  return(0);
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

char *
mkmsg2( const char *fmt, ...) {
  int size;
  va_list ap;
  static char buff[MAXMSGLEN+1];
  
  size = MAXMSGLEN;
  va_start(ap, fmt);
  vsnprintf(buff, size, fmt, ap);
  va_end(ap);

  return(buff);
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
readconfig( ) {
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
		  wsconf.timeout = atoi(value);
        } else if ( strcasecmp( name, "logfacility" ) == 0 ) {
		    if ( strcasecmp( value, "kern") == 0 ) 
			  wsconf.logfacility = LOG_KERN;
			else if ( strcasecmp( value, "user") == 0 ) 
			  wsconf.logfacility = LOG_USER;
			else if ( strcasecmp( value, "mail") == 0 ) 
			  wsconf.logfacility = LOG_MAIL;     
			else if ( strcasecmp( value, "daemon") == 0 ) 
			  wsconf.logfacility = LOG_DAEMON;
			else if ( strcasecmp( value, "syslog") == 0 ) 
			  wsconf.logfacility = LOG_SYSLOG;
			else if ( strcasecmp( value, "lpr") == 0 ) 
			  wsconf.logfacility = LOG_LPR;
			else if ( strcasecmp( value, "news") == 0 ) 
			  wsconf.logfacility = LOG_NEWS;
			else if ( strcasecmp( value, "uucp") == 0 ) 
			  wsconf.logfacility = LOG_UUCP;
			else if ( strcasecmp( value, "cron") == 0 ) 
			  wsconf.logfacility = LOG_CRON;
			else if ( strcasecmp( value, "ftp") == 0 ) 
			  wsconf.logfacility = LOG_FTP;
			else if ( strcasecmp( value, "local0") == 0 ) 
			  wsconf.logfacility = LOG_LOCAL0;
			else if ( strcasecmp( value, "local1") == 0 ) 
			  wsconf.logfacility = LOG_LOCAL1;
			else if ( strcasecmp( value, "local2") == 0 ) 
			  wsconf.logfacility = LOG_LOCAL2;
			else if ( strcasecmp( value, "local3") == 0 ) 
			  wsconf.logfacility = LOG_LOCAL3;
			else if ( strcasecmp( value, "local4") == 0 ) 
			  wsconf.logfacility = LOG_LOCAL4;
			else if ( strcasecmp( value, "local5") == 0 ) 
			  wsconf.logfacility = LOG_LOCAL5;
			else if ( strcasecmp( value, "local6") == 0 ) 
			  wsconf.logfacility = LOG_LOCAL6;
			else if ( strcasecmp( value, "local7") == 0 ) 
			  wsconf.logfacility = LOG_LOCAL7;
        } else if ( strcasecmp( name, "device" ) == 0 ) {
		  ws2000station.config.device = strdup(value);
        } else if ( strcasecmp( name, "port" ) == 0 ) {
		  wsconf.port = strdup(value);
        } else if ( strcasecmp( name, "tnport" ) == 0 ) {
		  wsconf.tnport = strdup(value);
        } else if ( strcasecmp( name, "xmlport" ) == 0 ) {
		  wsconf.xmlport = strdup(value);
        } else if ( strcasecmp( name, "rrdfile" ) == 0 ) {
		  ws2000station.sensor[0].rrdfile = strdup(value);
                  /* this is not complete and 
                     has to be changed
		  */
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
echoconfig ( ) {
  int size;
  char *s;
  static char t[MAXBUFF];

  s = mkmsg("Configuration parameters\n");
  size = strlen(s) + 1;
  
  strncat(t,s,strlen(s));
  s = mkmsg("\twsconf.command: %d\n", wsconf.command);
  strncat(t,s, strlen(s));

  s = mkmsg("\twsconf.argcmd: %d\n",wsconf.argcmd);
  strncat(t,s, strlen(s));

  s = mkmsg("\twsconf.netflg: %d\n",wsconf.netflg);
  strncat(t,s, strlen(s));
  s = mkmsg("\twsconf.verbose: %d\n",wsconf.verbose);
  strncat(t,s, strlen(s));
  s = mkmsg("\twsconf.timeout: %d\n",wsconf.timeout);
  strncat(t,s, strlen(s));
  s = mkmsg("\twsconf.logfacility: %d\n",wsconf.logfacility);
  strncat(t,s, strlen(s));
  s = mkmsg("\tws2000station.config.device: %s\n",ws2000station.config.device);
  strncat(t,s, strlen(s));
  s = mkmsg("\twsconf.hostname: %s\n",wsconf.hostname);
  strncat(t,s, strlen(s));
  s = mkmsg("\twsconf.port: %s\n",wsconf.port);
  strncat(t,s, strlen(s));
  s = mkmsg("\twsconf.tnport: %s\n",wsconf.tnport);
  strncat(t,s, strlen(s)); 
  s = mkmsg("\twsconf.xmlport: %s\n",wsconf.xmlport);
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


/*
  chklockf

  returns 0, if 
    lockfile of port does not exist 
    lockfile is not locked by another process

  returns -1, if
    open lockfile fails
    fcntl lockfile fails

  returns 1, if
    lockfile is locked by another process     
*/

/*
int
chklockf( const char *lockfile) {
  int ret, cfd;
  struct stat st;
  struct flock chklck;

  if ( ( ret = stat( lockfile, &st) ) == -1 ) {
    printf("can't stat lockfile: %s\n", strerror(errno));
    return(0);
  }
*/
  /* lockfile procedure to protect serial port */
  //memset( &chklck, 0, sizeof( struct flock));
/*
  chklck.l_type   = F_WRLCK;
  chklck.l_whence = SEEK_SET;
  chklck.l_start  = 0;
  chklck.l_len    = 0;
  chklck.l_pid    = getpid();
  printf("cfd: opening lockfile: ");
  cfd = open( lockfile, O_RDWR | O_CREAT, 0660);
  if ( cfd < 0 ) {
    printf(" %s\n", strerror(errno));
    return(-1);
  } 
*/

  /* check if lockfile is locked by another process */
/*
  if  ( ( ret = fcntl( cfd, F_GETLK, &chklck)) < 0 ) {
    printf("fcntl return code: %d: %s\n", ret, strerror(errno));
    return (-1);
  } else {     
    if ( chklck.l_type == F_UNLCK) {
      printf("l_type: F_UNLCK\n");
      return(0);
    } else if ( chklck.l_type == F_WRLCK) {
      printf("l_type: F_WRLCK: by process: %d\n", chklck.l_pid);
      printf("checking if locking process still running:");
      if ( ( ret = kill( chklck.l_pid, 0)) == 0) {
        printf(" yes\n"); 
        return(1);
      } else {
        printf(" no\n");
        return(0);
      } 
    } else if ( chklck.l_type == F_RDLCK) {
      printf("l_type: F_RDLCK: by process: %d\n", chklck.l_pid);
      return(1);
    } else {
      printf("l_type: UNKNOWN\n");
      return(-1);
    }
  }

  return(0);
}
*/

/*
  setlck
  open lockfile and set lock
*/
/*
int 
setlck( const char *lockfile) {
  int fd, ret;
  struct flock wthlck;
*/
  /* lockfile procedure to protect serial port */
  //memset( &wthlck, 0, sizeof( struct flock));

/*
  wthlck.l_type   = F_WRLCK;
  wthlck.l_whence = SEEK_SET;
  wthlck.l_start  = 0;
  wthlck.l_len    = 0;
  wthlck.l_pid    = getpid();
*/

  /* open lockfile */
  //printf("fd: opening lockfile: ");
/*
  fd = open( lockfile, O_RDWR | O_CREAT, 0660);
  if ( fd < 0 ) {
    printf(" %s\n", strerror(errno));
    return(fd);
  } 
  wthlck.l_type   = F_WRLCK;
  wthlck.l_pid    = getpid();
  if ( ( ret = fcntl( fd, F_SETLKW, &wthlck)) < 0 ) {
    printf("can't lock lockfile\n");
  };

  return(fd);

}
*/

/*

  unlck
  unlock and remove lockfile 
*/
/*
int
unlck( const char *lockfile, int fd) {
  int ret;
  struct flock wthlck;

  wthlck.l_type   = F_UNLCK;
  ret = fcntl( fd, F_SETLKW, &wthlck);
  printf("fd: lockfile unlocked\n");
  ret = close( fd);
  return(ret);
}
*/
