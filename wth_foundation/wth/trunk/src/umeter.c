/* 
  umeter.c

  ULTIMETER weatherstation handler implemented as POSIX thread

  $Id$
  $Revision$

  Copyright (C) 2010 Volker Jahns <volker@thalreit.de>

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

/*
  umeter_hd

  POSIX thread to handle ULTIMETER weatherstation

  opens port
  call to data reading subroutine

*/
void *
umeter_hd( void *arg) {
  int pfd, err;
  struct termios tp, op;

  syslog( LOG_DEBUG, "umeter_hd: start of execution");

  /* initialize serial port */
  if ( initumeter(&pfd, &tp, &op) == -1 )
    return( ( void *) &failure);

  umeterstation.status.is_present = 1;

  /* read ULTIMETER weatherstation */
  err = umeter_rd( pfd);

  closeumeter(pfd, &op);

  return( ( void *) &success);
}

/*
  datalogger_rd

  reading data records with ULTIMETER in data logger mode
*/
int
datalogger_rd( unsigned char * datalogdata, int ndat) {

  int  base=16;
  char umeterstr[5];

  unsigned int temp_out;
  unsigned int rain_total;
  unsigned int baro;
  unsigned int humid_out;
  unsigned int rain_today;
  unsigned int windspeed_avg;
  unsigned int windspeed;
  unsigned int winddir;
  unsigned int temp_in;
  unsigned int humid_in;
  int day_of_year;
  int min_of_day;

  time_t clock;
  int minute, hour, year;
  struct tm tm;
  struct tm *ptm;
  char buf[TBUFF+1];
  time_t dataset_date;

  syslog(LOG_DEBUG, "datalog_rd: data: %s\n", datalogdata);
  time(&dataset_date);

  strncpy(umeterstr, (const char * )(datalogdata+2), 5); 
  umeterstr[4] = 0;
  windspeed = strtol(umeterstr, NULL, base);
  printf("windspeed: %d\n", windspeed);
  measval_db( "windsensor", "Windspeed", dataset_date, (float)windspeed, umeterdb);

  strncpy(umeterstr, (const char *)(datalogdata+6), 5); 
  umeterstr[4] = 0;
  winddir = strtol(umeterstr, NULL, base);
  printf("winddir: %d\n", winddir);

  strncpy(umeterstr, (const char *)(datalogdata+10), 5); 
  umeterstr[4] = 0;
  temp_out = strtol(umeterstr, NULL, base);
  printf("temp_out: %d\n", temp_out);

  strncpy(umeterstr, (const char *)(datalogdata+14), 5); 
  umeterstr[4] = 0;
  rain_total = strtol(umeterstr, NULL, base);
  printf("rain_total: %d\n", rain_total);

  strncpy(umeterstr, (const char *)(datalogdata+18), 5); 
  umeterstr[4] = 0;
  baro = strtol(umeterstr, NULL, base);
  printf("baro pressure: %d\n", baro);

  strncpy(umeterstr, (const char *)(datalogdata+22), 5); 
  umeterstr[4] = 0;
  temp_in = strtol(umeterstr, NULL, base);
  printf("temp_in: %d\n", temp_in);

  strncpy(umeterstr, (const char *)(datalogdata+26), 5); 
  umeterstr[4] = 0;
  humid_out = strtol(umeterstr, NULL, base);
  printf("humid_out: %d\n", humid_out);

  strncpy(umeterstr, (const char *)(datalogdata+30), 5); 
  umeterstr[4] = 0;
  humid_in = strtol(umeterstr, NULL, base);
  printf("humid_in: %d\n", humid_in);

  strncpy(umeterstr, (const char *)(datalogdata+34), 5); 
  umeterstr[4] = 0;
  day_of_year = strtol(umeterstr, NULL, base);
  printf("day_of_year: %d\n", day_of_year);

  strncpy(umeterstr, (const char *)(datalogdata+38), 5); 
  umeterstr[4] = 0;
  min_of_day = strtol(umeterstr, NULL, base);
  printf("min_of_day: %d\n", min_of_day);

  strncpy(umeterstr, (const char *)(datalogdata+42), 5); 
  umeterstr[4] = 0;
  rain_today = strtol(umeterstr, NULL, base);
  printf("rain_today: %d\n", rain_today);

  strncpy(umeterstr, (const char *)(datalogdata+46), 5); 
  umeterstr[4] = 0;
  windspeed_avg = strtol(umeterstr, NULL, base);
  printf("windspeed_avg: %d\n", windspeed_avg);

  memset(&tm, 0, sizeof(struct tm ));
  time(&clock);
  ptm = gmtime(&clock);
  year = 1900 + ptm->tm_year;
  minute  = min_of_day % 60;
  hour = min_of_day / 60;
  snprintf(buf, sizeof(buf), "%d %d %02d:%02d:00", year, day_of_year, hour, minute);
  strptime(buf,"%Y %j %H:%M:%S", &tm);
  strftime(buf, sizeof(buf), "%d %b %Y %H:%M", &tm);
  puts(buf);

  return(0);
}

/*
  packet_rd

  reading data records with ULTIMETER in packet mode
*/
int
packet_rd( unsigned char * packetdata, int ndat) {
  int err;
  int  base=16;

  char umeterstr[5];
  char umeterlstr[9];
  int out_th_present = 0;

  /* parameters of packet mode */
  float windspeed_peak;
  float winddir_peak;
  float temp_out;
  float rain_total;
  float baro;
  float baro_chg;
  unsigned int baro_corr; // 8 bytes
  float humid_out;
  float rain_today;
  float windspeed_avg;

  int day_of_year;
  int min_of_day;

  time_t clock;
  int minute, hour, year;
  struct tm tm;
  struct tm *ptm;
  char buf[TBUFF+1];
  time_t dataset_date;

  /*
    parse data in packetmode
  */
  printf("packetdata: '%s'\n", packetdata);
  time(&dataset_date);

  /* open sqlite db file */
  if ( ( err = sqlite3_open( umeterstation.config.dbfile, &umeterdb))) {
    syslog(LOG_ALERT, "statdb: Failed to open database %s. Error: %s\n", 
      umeterstation.config.dbfile, sqlite3_errmsg(umeterdb));
    return(err);
  }

  strncpy(umeterstr, (const char *)(packetdata+5), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: '%s'\n", umeterstr);
  windspeed_peak = (float)strtol(umeterstr, NULL, base);
  windspeed_peak = (1.0/36.0)*windspeed_peak; /* 0.1 kph to ms-1 */
  syslog(LOG_DEBUG, "windspeed_peak: %f\n", windspeed_peak);
  measval_db( "Wind Sensor", "Windspeed Peak 5 Minute", dataset_date, (float)windspeed_peak, umeterdb);

  strncpy(umeterstr, (const char *)(packetdata+9), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: '%s'\n", umeterstr);
  winddir_peak = (float)strtol(umeterstr, NULL, base);
  winddir_peak = (360.0/255.0)*winddir_peak; /* 0-255 to 0-360 deg */
  measval_db( "Wind Sensor", "Wind Direction", dataset_date, (float)winddir_peak, umeterdb);
  syslog(LOG_DEBUG, "winddir_peak: %f\n", winddir_peak);

  strncpy(umeterstr, (const char *)(packetdata+13), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: '%s'\n", umeterstr);
  temp_out = (float)strtol(umeterstr, NULL, base);
  temp_out = ((1.0/10.0)*temp_out -32.0)*5.0/9.0; /* 0.1 degF to degC */
  /* temperature data will not be written to database until it is clear that no TH sensor is installed */
  syslog(LOG_DEBUG, "temp_out: %f\n", temp_out);

  strncpy(umeterstr, (const char *)(packetdata+17), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: '%s'\n", umeterstr);
  rain_total = (float)strtol(umeterstr, NULL, base);
  rain_total = (25.4/10.0)*rain_total; /* 0.1in to mm */
  measval_db( "Rain Gauge", "Rain Longterm Total", dataset_date, (float)rain_total, umeterdb);
  syslog(LOG_DEBUG, "rain_total: %f\n", rain_total);

  strncpy(umeterstr, (const char *)(packetdata+21), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: '%s'\n", umeterstr);
  baro = (float)strtol(umeterstr, NULL, base);
  baro = baro/10.0; /* 0.1mbar to mbar */
  measval_db( "Indoor Temperator/Barometer Sensor", "Current Barometer", dataset_date, (float)baro, umeterdb);
  syslog(LOG_DEBUG, "baro: %f\n", baro);

  strncpy(umeterstr, (const char *)(packetdata+25), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: '%s'\n", umeterstr);
  baro_chg = (float)strtol(umeterstr, NULL, base); 
  baro_chg = baro_chg/10.0; /* 0.1mbar to mbar */
  measval_db( "Indoor Temperator/Barometer Sensor", "Barometer Delta", dataset_date, (float)baro_chg, umeterdb);
  syslog(LOG_DEBUG, "baro_chg: %f\n", baro_chg);

  strncpy(umeterlstr, (const char *)(packetdata+29), 9); 
  umeterlstr[8] = 0;
  syslog(LOG_DEBUG, "umeterlstr: '%s'\n", umeterlstr);
  baro_corr = strtol(umeterlstr, NULL, base);
  measval_db( "Indoor Temperator/Barometer Sensor", "Barometer Correction Factor", dataset_date, (float)baro_corr, umeterdb);
  syslog(LOG_DEBUG, "baro_corr: %d\n", baro_corr);

  strncpy(umeterstr, (const char *)(packetdata+37), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: '%s'\n", umeterstr);
  err = strncmp( umeterstr, "----", 4);
  /* Temperature Sensor installed */
  if ( err == 0 ) {
    syslog(LOG_INFO, "No Outdoor Humidity/Temperature Sensor found");
    measval_db( "Temperature Sensor", "Current Outdoor Temp", dataset_date, (float)temp_out, umeterdb);
  } else /* Outdoor Humidity Temperature sensor installed */ {
    out_th_present = 1;
    humid_out = (float)strtol(umeterstr, NULL, base);
    humid_out = (1.0/10.0)*humid_out; /* 0.1% to %rel.hum. */
    measval_db( "Outdoor Humidity/Temperature Sensor", "Current Outdoor Temp", dataset_date, (float)temp_out, umeterdb);
    measval_db( "Outdoor Humidity/Temperature Sensor", "Current Outdoor Humidity", dataset_date, (float)humid_out, umeterdb);
    syslog(LOG_DEBUG, "humid_out: %f\n", humid_out); 
  }


  strncpy(umeterstr, (const char *)(packetdata+41), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: '%s'\n", umeterstr);
  day_of_year = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "day_of_year: %d\n", day_of_year);

  strncpy(umeterstr, (const char *)(packetdata+45), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: '%s'\n", umeterstr);
  min_of_day = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "min_of_day: %d\n", min_of_day);

  memset(&tm, 0, sizeof(struct tm ));
  time(&clock);
  ptm = gmtime(&clock);
  year = 1900 + ptm->tm_year;
  minute  = min_of_day % 60;
  hour = min_of_day / 60;

  snprintf( buf, sizeof(buf), "%d %d %02d:%02d:00", year, day_of_year+1, hour, minute);
  strptime(buf,"%Y %j %H:%M:%S", &tm);
  strftime(buf, sizeof(buf), "%d %b %Y %H:%M", &tm);
  syslog(LOG_DEBUG, "packet_rd: %s", buf);

  strncpy(umeterstr, (const char *)(packetdata+49), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: '%s'\n", umeterstr);
  rain_today = (float)strtol(umeterstr, NULL, base);
  rain_today = (25.4/10.0)*rain_today; /* 0.1in to mm */

  measval_db( "Rain Gauge", "Today Rain Total", dataset_date, (float)rain_total, umeterdb);
  syslog(LOG_DEBUG, "rain_today: %f\n", rain_today);

  strncpy(umeterstr, (const char *)(packetdata+53), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: '%s'\n", umeterstr);
  windspeed_avg = (float)strtol(umeterstr, NULL, base);
  windspeed_avg = (1.0/36.0)*windspeed_avg; /* 0.1 kph to ms-1 */
  measval_db( "Wind Sensor", "5 Minute Windspeed Average", dataset_date, (float)windspeed_avg, umeterdb);
  syslog(LOG_DEBUG, "windspeed_avg: %f\n", windspeed_avg);

  sqlite3_close( umeterdb);
  return(0);
}

/*
  umeter_rd

  reading data records from serial port
  data acquisition to database

*/
int 
umeter_rd( int rfd) {
  int err = -1;
  int ndat;
  static unsigned char data[TBUFF+1];

  const char dataloghd[3] = "!!";
  const char packethd[6] = "$ULTW";
  int dataloglen = 51; /* NL is ignored */
                               /*  thus record size is 48 hex bytes + 2 header bytes + carriage return */
  int packetlen  = 58; /* NL is ignored */
                               /* thus record size is 52 hex bytes + 5 header bytes + carriage */


  for (;;) {
    ndat = read(rfd,data,1024); 
    data[ndat-1]=0;

    if ( ( strncmp( (const char *)data, (const char *)dataloghd, 2) == 0) 
          && ( ndat == dataloglen) ) {
      syslog(LOG_DEBUG, "umeter_rd: datalogger mode\n");
      datalogger_rd( data, ndat);
      
    } else if ( ( strncmp( (const char *)data, (const char *)packethd, 5) == 0) 
          && ( ndat ==packetlen )) {
      syslog(LOG_DEBUG, "umeter_rd: packet mode\n");
      packet_rd( data, ndat);
    } else {
      syslog(LOG_DEBUG, "data (garbage): '%s' : %d\n", data, ndat);
    }
  }
  return(err); 
}

/*  initumeter

  opens serial port for communication
  
  serial port settings:
  ULTIMETER 2100 (and similar stations)
     2400, 8 data bits, no parity, 1 stop
*/
int 
initumeter (int *pfd, struct termios *newtio,
	    struct termios *oldtio) 
{

  *pfd = open(umeterstation.config.device, O_RDWR| O_NOCTTY );
  if ( pfd <0) {
    werrno = errno;
    syslog(LOG_INFO, "initumeter: error opening serial device : %s",
	   strerror(werrno));
    return(-1);
  }

  tcgetattr(*pfd,oldtio); /* save current serial port settings */
  memset(newtio, 0, sizeof(*newtio)); 
        
  newtio->c_cflag =  CS8 | CLOCAL | CREAD;
  newtio->c_iflag = IGNPAR | IGNCR | ICRNL;
  newtio->c_oflag = 0;
  newtio->c_lflag = ICANON;

  cfsetospeed(newtio,B2400);            // 2400 baud
  cfsetispeed(newtio,B2400);            // 2400 baud

  tcsetattr(*pfd,TCSANOW,newtio);

  return(0);
}

/*
   closeumeter

   restore the old port settings
   lower DTR on serial line

*/
int closeumeter( int fd, struct termios *oldtio) {

    /* restore old port settings */
    /* takes approx 3 sec to reopen fd under FreeBSD
       if tcsetattr is called to restore */
    if ( tcsetattr(fd,TCSANOW,oldtio) == -1 ) {
	  werrno = errno;
	  syslog(LOG_INFO, "closeumeter: error ioctl: %s",
			 strerror(werrno));
	  return(-1);	
	}
	
    /* close serial port */
    if ( close(fd) == -1 ) {
	  werrno = errno;
	  syslog(LOG_INFO, "closeumeter: error close: %s",
			 strerror(werrno));
	  return(-1);	
    }
    return(0);
}

