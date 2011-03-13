/* 
  wmr9x8.c

  wmr9x8 handler implemented as POSIX thread

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
#define WINDLEN 11
#define RAINLEN 16
#define THINLEN 9
#define THOUTLEN 9
#define TINLEN 7
#define THBLEN 13
#define THBNEWLEN 14
#define MINLEN 5
#define CLOCKLEN 9


#define WINDTYP   0x00
#define RAINTYP   0x01
#define THINTYP   0x02
#define THOUTTYP  0x03
#define TINTYP    0x04
#define THBTYP    0x05
#define THBNEWTYP 0x06
#define MINTYP    0x0e
#define CLOCKTYP  0x0f

/*
  wmr9x8_hd

  POSIX thread to handle WMR9x8 weatherstation

  opens port
  call to data reading subroutine

*/
void *
wmr9x8_hd( void *arg) {
  int rfd, err;
  struct termios tp, op;

  syslog( LOG_DEBUG, "wmr9x8_hd: start of execution");

  /* initialize serial port */
  if ( initwmr9x8(&rfd, &tp, &op) == -1 )
    return( ( void *) &failure);
  
  wmr9x8station.status.is_present = 1;

  /* read WMR 9x8 weatherstation */
  err = wmr9x8rd( rfd);

  closewmr9x8(rfd, &op);

  return( ( void *) &success);
}


/*
  wmr9x8rd

  reading data records from serial port
  data acquisition to database
  
  note: checksum might be 0xff

*/
int 
wmr9x8rd( int rfd) {
  int i;
  int err = -1;
  int sync = 0;
  int ndat = 0;
  int dtyp = -1;
  unsigned char schr = 0x00;
  unsigned char data[TBUFF+1];

  memset( data, 0 , TBUFF);
  for (;;) {
    
    err = resetwmr9x8( rfd);
    if ( err == 1) {
      syslog(LOG_ALERT, "wmr9x8rd: reset serial port failure");
    }
    err = getschar( rfd, &schr);
    if ( err == 1) {
      data[ndat] = schr;
      ndat++;
    } else {
      syslog(LOG_DEBUG, "wmr9x8rd: could not read 1 char err: %d\n", err);
    }

    if ( schr == 0xff) {
      sync++;
    }

    if (sync == 2) {
      if (( schr == 0x00) || ( schr == 0x01 ) || (schr == 0x02) || (schr == 0x03) || (schr == 0x05) || ( schr == 0x0e) || ( schr == 0x0f) || ( schr == 0x06)) {
        syslog( LOG_DEBUG,"devicetyp: %x", schr);
	dtyp = schr;
	sync++;
      }
    }
    
    if ( sync == 3) {
      syslog(LOG_DEBUG, "wmr9x8rd: header sync + devicetype %x received", dtyp);
      data[0] = 0xff;
      data[1] = 0xff;
      data[2] = dtyp;

      if ( dtyp == WINDTYP ) {
	syslog(LOG_DEBUG, "wmr9x8rd: devicetyp is WINDTYP");
	for ( i = 0; i < WINDLEN - 3 ; i++) {
	  err = getschar( rfd, &schr);
          data[i+3] = schr;
	}
	ndat = WINDLEN;

      } else if ( dtyp == RAINTYP ) {
	syslog(LOG_DEBUG, "wmr9x8rd: devicetyp is RAINTYP");
	for ( i = 0; i < RAINLEN - 3; i++) {
	  err = getschar( rfd, &schr);
          data[i+3] = schr;
	}
	ndat = RAINLEN;

      } else if ( dtyp == THINTYP ) {
	syslog(LOG_DEBUG, "wmr9x8rd: devicetyp is THINTYP");
	for ( i = 0; i < THINLEN - 3; i++) {
	  err = getschar( rfd, &schr);
          data[i+3] = schr;
	}
	ndat = THINLEN;

      } else if ( dtyp == THOUTTYP ) {
	syslog(LOG_DEBUG, "wmr9x8rd: devicetyp is THOUTTYP");
	for ( i = 0; i < THOUTLEN-3; i++) {
	  err = getschar( rfd, &schr);
          data[i+3] = schr;
	}
	ndat = THOUTLEN;

      } else if ( dtyp == TINTYP ) {
	syslog(LOG_DEBUG, "wmr9x8rd: devicetyp is TINTYP");
	for ( i = 0; i < TINLEN - 3; i++) {
	  err = getschar( rfd, &schr);
          data[i+3] = schr;
	}
	ndat = TINLEN;
      } else if ( dtyp == THBTYP ) {
	syslog(LOG_DEBUG, "wmr9x8rd: devicetyp is THBTYP");
	for ( i = 0; i < THBLEN - 3; i++) {
	  err = getschar( rfd, &schr);
          data[i+3] = schr;
	}
	ndat = THBLEN;

      } else if ( dtyp == MINTYP ) {
	syslog(LOG_DEBUG, "wmr9x8rd: devicetyp is MINTYP");
	for ( i = 0; i < MINLEN - 3; i++) {
	  err = getschar( rfd, &schr);
          data[i+3] = schr;
	}
	ndat = MINLEN;
      } else if ( dtyp == CLOCKTYP ) {
	syslog(LOG_DEBUG, "wmr9x8rd: devicetyp is CLOCKTYP");
	for ( i = 0; i < CLOCKLEN - 3; i++) {
	  err = getschar( rfd, &schr);
          data[i+3] = schr;
	}
	ndat = CLOCKLEN;

      } else if ( dtyp == THBNEWTYP ) {
	syslog(LOG_DEBUG, "wmr9x8rd: devicetyp is THBNEWTYP");
	for ( i = 0; i < THBNEWLEN - 3; i++) {
	  err = getschar( rfd, &schr);
          data[i+3] = schr;
	}
	ndat = THBNEWLEN;
      } else {
	syslog(LOG_ALERT, "wmr9x8rd: UNKNOWN devicetyp");

      }
      syslog(LOG_DEBUG, "wmr9x8rd: data record\n");
      echodata( data, ndat);
      wmr9x8dac( data, ndat);

      sync = 0; ndat = 0; memset( data, 0, TBUFF);    
    }

  }

  return(err); 
}

/*
  wmr9x8dac

  data acquisition

*/
int
wmr9x8dac( unsigned char *data, int ndat) {
  int err;
  unsigned char devtype = 0xff;

  syslog(LOG_DEBUG, "wmr9x8dac: data record");
  echodata( data, ndat);

  //err = checksum( data, ndat);

  devtype = data[2];
  //syslog(LOG_DEBUG, "wmr9x8dac: devicetype: %d", devtype);

  if ( devtype == 0x00 && ndat == WINDLEN) 
    wind_dac( data); 
  
  if ( devtype == 0x01 && ndat == RAINLEN) 
    rain_dac( data); 
  
  if ( devtype == 0x02 && ndat == THINLEN) 
    thin_dac( data); 
  
  if ( devtype == 0x03 && ndat == THOUTLEN) 
    thout_dac( data); 
  
  if ( devtype == 0x04 && ndat == TINLEN) 
    tin_dac( data); 
  
  if ( devtype == 0x05 && ndat == THBLEN) 
    thb_dac( data); 
  
  if ( devtype == 0x06 && ndat == THBNEWLEN) 
    thbnew_dac( data); 
  
  if ( devtype == 0x0e && ndat == MINLEN) 
    minute_dac( data); 
  
  if ( devtype == 0x0f && ndat == CLOCKLEN) 
    clock_dac( data); 
  
  return(err);
}

/*
  wind_dac
  windsensor
*/
int
wind_dac( unsigned char *data) {
  int err;

  unsigned char gust_overrange;
  unsigned char average_overrange;
  unsigned char low_battery;
  int wind_direction;
  float gust_windspeed;
  float average_windspeed;
  unsigned char chill_nodata;
  unsigned char chill_overrange;
  unsigned char sign;
  int windchill;
  time_t dataset_date;

  syslog( LOG_DEBUG,"wind_dac: data acquisition of wind data");

  time(&dataset_date);

  gust_overrange    = getbits( data[3], 4, 1);
  average_overrange = getbits( data[3], 5, 1);
  low_battery       = getbits( data[3], 6, 1);
  syslog(LOG_DEBUG,"wind_dac: gust_overrange: %d\n", gust_overrange);
  syslog(LOG_DEBUG,"wind_dac: average_overrange: %d\n", average_overrange);
  syslog(LOG_DEBUG,"wind_dac: low_battery: %d\n", low_battery);

  wind_direction =      lownibble(data[4]) + 
                   10 * highnibble(data[4]) + 
                  100 * lownibble(data[5]);
  syslog(LOG_DEBUG,"wind_dac: wind_direction: %d\n", wind_direction);

  gust_windspeed =  0.1 * highnibble(data[5]) +
                    1.0 * lownibble(data[6]) +
                   10.0 * highnibble(data[6]);
  syslog(LOG_DEBUG,"wind_dac: gust_windspeed: %f\n", gust_windspeed);

  average_windspeed =  0.1 * lownibble(data[7]) +
                       1.0 * highnibble(data[7]) +
                      10.0 * lownibble(data[8]);
  syslog(LOG_DEBUG,"wind_dac: average_windspeed: %f\n", average_windspeed);

  chill_nodata    = getbits( data[8], 5, 1);
  chill_overrange = getbits( data[8], 6, 1);
  sign            = getbits( data[8], 7, 1);

  syslog(LOG_DEBUG,"wind_dac: chill_nodata: %d\n", chill_nodata);
  syslog(LOG_DEBUG,"wind_dac: chill_overrange: %d\n", chill_overrange);
  syslog(LOG_DEBUG,"wind_dac: sign: %d\n", sign);
  windchill =  1.0  * lownibble(data[9]) +
                10.0 * highnibble(data[9]);
  if ( sign == 1) 
    windchill = - windchill;

  syslog(LOG_DEBUG,"wind_dac: windchill: %d\n", windchill); 

  /* open sqlite db file */
  if ( ( err = sqlite3_open( wmr9x8station.config.dbfile, &wmr9x8db))) {
    syslog(LOG_ALERT, "statdb: Failed to open database %s. Error: %s\n", 
      wmr9x8station.config.dbfile, sqlite3_errmsg(wmr9x8db));
    return(err);
  }

  statval_db("windsensor", "gust_overrange", dataset_date, (unsigned long int)gust_overrange, wmr9x8db);
  statval_db("windsensor", "average_overrange", dataset_date, (unsigned long int)average_overrange, wmr9x8db);
  statval_db("windsensor", "low_battery", dataset_date, (unsigned long int)low_battery, wmr9x8db);
  measval_db( "windsensor", "wind_direction", dataset_date, (float)wind_direction, wmr9x8db);
  measval_db( "windsensor", "gust_windspeed", dataset_date, (float)gust_windspeed, wmr9x8db);
  measval_db( "windsensor", "average_windspeed", dataset_date, (float)average_windspeed, wmr9x8db);
  measval_db( "windsensor", "windchill", dataset_date, (float)windchill, wmr9x8db);

  statval_db("windsensor", "chill_nodata", dataset_date, (unsigned long int)chill_nodata, wmr9x8db);
  statval_db("windsensor", "chill_overrange", dataset_date, (unsigned long int)chill_overrange, wmr9x8db);

  sqlite3_close( wmr9x8db);

  return err;
}

/*
  rain_dac
  rainsensor
*/
int
rain_dac( unsigned char *data) {
  int err;
  unsigned char rate_overrange;
  unsigned char total_overrange;
  unsigned char low_battery;
  unsigned char yesterday_overrange;

  float current_rainrate;
  float total_rainfall;
  float yesterday_rainfall;

  struct tm *total_startdate;
  time_t startdate;
  int t_minute;
  int t_hour;
  int t_day;
  int t_month;
  int t_year;
  time_t dataset_date;
  char tbuf[TBUFF];

  syslog( LOG_DEBUG,"rain_dac: data acquisition of rain data");
  time(&dataset_date);
  time(&startdate);
  total_startdate = gmtime(&startdate);

  rate_overrange      = getbits( data[3], 4, 1);
  total_overrange     = getbits( data[3], 5, 1);
  low_battery         = getbits( data[3], 6, 1);
  yesterday_overrange = getbits( data[3], 7, 1);

  current_rainrate    =   1.0 * lownibble( data[4]) +
                         10.0 * highnibble( data[4]) +
                        100.0 * lownibble( data[5]);

  total_rainfall      =   0.1 * highnibble(data[5]) +
                          1.0 * lownibble(data[6]) +
                         10.0 * highnibble(data[6]) +
                        100.0 * lownibble(data[7]) +
                       1000.0 * highnibble(data[7]);

  yesterday_rainfall  =   1.0 * lownibble(data[8]) +
                         10.0 * highnibble(data[8]) +
                        100.0 * lownibble(data[9]) +
                       1000.0 * highnibble(data[9]);

  t_minute            =         lownibble( data[10]) + 
                           10 * highnibble(data[10]);

  t_hour              =         lownibble( data[11]) + 
                           10 * highnibble(data[11]);

  t_day               =         lownibble( data[12]) + 
                           10 * highnibble(data[12]);

  t_month             =         lownibble( data[13]) + 
                           10 * highnibble(data[13]);

  t_year              =         lownibble( data[14]) + 
                           10 * highnibble(data[14]);

  total_startdate->tm_sec  = 0;
  total_startdate->tm_min  = t_minute;
  total_startdate->tm_hour = t_hour;
  total_startdate->tm_mday = t_day;
  total_startdate->tm_mon  = t_month - 1;
  total_startdate->tm_year = t_year;
  startdate = mktime( total_startdate);
  strftime(tbuf, sizeof(tbuf), "%x %X", total_startdate);

  syslog(LOG_DEBUG, "rain_dac: rate_overrange: %d\n", rate_overrange);
  syslog(LOG_DEBUG, "rain_dac: total_overrange: %d\n", total_overrange);
  syslog(LOG_DEBUG, "rain_dac: low_battery: %d\n", low_battery);
  syslog(LOG_DEBUG, "rain_dac: yesterday_overrange: %d\n", yesterday_overrange);

  syslog(LOG_DEBUG, "rain_dac: current_rainrate: %f\n", current_rainrate);
  syslog(LOG_DEBUG, "rain_dac: total_rainfall: %f\n", total_rainfall);
  syslog(LOG_DEBUG, "rain_dac: yesterday_rainfall: %f\n", yesterday_rainfall);

  syslog(LOG_DEBUG, "rain_dac: t_minute: %d\n", t_minute);
  syslog(LOG_DEBUG, "rain_dac: t_hour: %d\n", t_hour);
  syslog(LOG_DEBUG, "rain_dac: t_day: %d\n", t_day);
  syslog(LOG_DEBUG, "rain_dac: t_month: %d\n", t_month);
  syslog(LOG_DEBUG, "rain_dac: t_year: %d\n", t_year);
  syslog(LOG_DEBUG, "rain_dac: total_startdate: %lu\n", (long unsigned int)startdate);
  syslog(LOG_DEBUG, "rain_dac: total_startdate: %s\n", tbuf);

  /* open sqlite db file */
  if ( ( err = sqlite3_open( wmr9x8station.config.dbfile, &wmr9x8db))) {
    syslog(LOG_ALERT, "statdb: Failed to open database %s. Error: %s\n", 
      wmr9x8station.config.dbfile, sqlite3_errmsg(wmr9x8db));
    return(err);
  }

  statval_db("rainsensor", "rate_overrange", dataset_date, (unsigned long int)rate_overrange, wmr9x8db);
  statval_db("rainsensor", "total_overrange", dataset_date, (unsigned long int)total_overrange, wmr9x8db);
  statval_db("rainsensor", "low_battery", dataset_date, (unsigned long int)low_battery, wmr9x8db);
  statval_db("rainsensor", "yesterday_overrange", dataset_date, (unsigned long int)yesterday_overrange, wmr9x8db);
  measval_db( "rainsensor", "current_rainrate", dataset_date, (float)current_rainrate, wmr9x8db);
  measval_db( "rainsensor", "total_rainfall", dataset_date, (float)total_rainfall, wmr9x8db);
  measval_db( "rainsensor", "yesterday_rainfall", dataset_date, (float)yesterday_rainfall, wmr9x8db);
  statval_db("rainsensor", "total_startdate", dataset_date, (unsigned long int)startdate, wmr9x8db);

  sqlite3_close( wmr9x8db);

  return err;
}

/*
  thin_dac

  temperature, humidity indoor sensor 
  untested 
*/
int
thin_dac( unsigned char *data) {
  int err;
  unsigned char channel_no;
  unsigned char dew_underrange;
  unsigned char low_battery;
  unsigned char over_underrange;
  unsigned char sign;
  float temperature;
  float humidity;
  float dew_temperature;
  time_t dataset_date;

  syslog( LOG_DEBUG,"thin_dac: data acquisition of indoor temperature/humidity data");
  time(&dataset_date);
  channel_no      = lownibble(data[3]);
  dew_underrange  = getbits(data[3], 4, 1);
  low_battery     = getbits(data[3], 6, 1);

  temperature     =  0.1 * lownibble(data[4]) +
                     1.0 * highnibble(data[4]) +
                    10.0 * lownibble(data[5]) +
                   100.0 * getbits(data[5], 5, 2);
 
  over_underrange = getbits(data[5], 6, 1); 
  sign            = getbits(data[5], 7, 1); 
  if ( sign == 1) 
    temperature = -temperature;

  humidity        =  1.0 * lownibble(data[6]) +
                    10.0 * highnibble(data[6]);

  dew_temperature =  1.0 * lownibble(data[7]) +
                    10.0 * highnibble(data[7]);

  syslog(LOG_DEBUG, "thin_dac: dew_underrange: %d\n", dew_underrange);
  syslog(LOG_DEBUG, "thin_dac: low_battery: %d\n", low_battery);
  syslog(LOG_DEBUG, "thin_dac: temperature: %f\n", temperature);
  syslog(LOG_DEBUG, "thin_dac: humidity: %f\n", humidity);
  syslog(LOG_DEBUG, "thin_dac: dew_temperature: %f\n", dew_temperature);
  syslog(LOG_DEBUG, "thin_dac: over_underrange: %d\n", over_underrange);
  /* open sqlite db file */
  if ( ( err = sqlite3_open( wmr9x8station.config.dbfile, &wmr9x8db))) {
    syslog(LOG_ALERT, "statdb: Failed to open database %s. Error: %s\n", 
      wmr9x8station.config.dbfile, sqlite3_errmsg(wmr9x8db));
    return(err);
  }

  statval_db("thin_sensor", "dew_underrange", dataset_date, (unsigned long int)dew_underrange, wmr9x8db);
  statval_db("thin_sensor", "low_battery", dataset_date, (unsigned long int)low_battery, wmr9x8db);
  measval_db( "thin_sensor", "temperature", dataset_date, (float)temperature, wmr9x8db);
  measval_db( "thin_sensor", "humidity", dataset_date, (float)humidity, wmr9x8db);
  measval_db( "thin_sensor", "dew_temperature", dataset_date, (float)dew_temperature, wmr9x8db);
  statval_db("thin_sensor", "over_underrange", dataset_date, (unsigned long int)over_underrange, wmr9x8db);

  sqlite3_close( wmr9x8db);


  return err;
}

/*
  thout_dac
  temperature, humidity outdoor sensor
*/
int
thout_dac( unsigned char *data) {
  int err;
  unsigned char dew_underrange;
  unsigned char low_battery;
  unsigned char over_underrange;
  unsigned char sign;
  float temperature;
  float humidity;
  float dew_temperature;
  time_t dataset_date;

  syslog( LOG_DEBUG,"thout_dac: data acquisition of outdoor temperature/humidity data");
  time(&dataset_date);
  dew_underrange  = getbits(data[3], 4, 1);
  low_battery     = getbits(data[3], 6, 1);

  temperature     =  0.1 * lownibble(data[4]) +
                     1.0 * highnibble(data[4]) +
                    10.0 * lownibble(data[5]) +
                   100.0 * getbits(data[5], 5, 2);
 
  over_underrange = getbits(data[5], 6, 1); 
  sign            = getbits(data[5], 7, 1); 
  if ( sign == 1) 
    temperature = -temperature;

  humidity        =  1.0 * lownibble(data[6]) +
                    10.0 * highnibble(data[6]);

  dew_temperature =  1.0 * lownibble(data[7]) +
                    10.0 * highnibble(data[7]);

  syslog(LOG_DEBUG,"thout_dac: dew_underrange: %d\n", dew_underrange);
  syslog(LOG_DEBUG,"thout_dac: low_battery: %d\n", low_battery);
  syslog(LOG_DEBUG,"thout_dac: temperature: %f\n", temperature);
  syslog(LOG_DEBUG,"thout_dac: humidity: %f\n", humidity);
  syslog(LOG_DEBUG,"thout_dac: dew_temperature: %f\n", dew_temperature);
  syslog(LOG_DEBUG,"thout_dac: over_underrange: %d\n", over_underrange);

  /* open sqlite db file */
  if ( ( err = sqlite3_open( wmr9x8station.config.dbfile, &wmr9x8db))) {
    syslog(LOG_ALERT, "statdb: Failed to open database %s. Error: %s\n", 
      wmr9x8station.config.dbfile, sqlite3_errmsg(wmr9x8db));
    return(err);
  }

  statval_db("thout_sensor", "dew_underrange", dataset_date, (unsigned long int)dew_underrange, wmr9x8db);
  statval_db("thout_sensor", "low_battery", dataset_date, (unsigned long int)low_battery, wmr9x8db);
  measval_db( "thout_sensor", "temperature", dataset_date, (float)temperature, wmr9x8db);
  measval_db( "thout_sensor", "humidity", dataset_date, (float)humidity, wmr9x8db);
  measval_db( "thout_sensor", "dew_temperature", dataset_date, (float)dew_temperature, wmr9x8db);
  statval_db("thout_sensor", "over_underrange", dataset_date, (unsigned long int)over_underrange, wmr9x8db);

  sqlite3_close( wmr9x8db);


  return err;
}

/*
  tin_dac
  temperature indoor sensor
  untested
*/
int
tin_dac( unsigned char *data) {
  int err;
  unsigned char channel_no;
  unsigned char low_battery;
  unsigned char over_underrange;
  unsigned char sign;
  float temperature;
  time_t dataset_date;

  syslog( LOG_DEBUG,"tin_dac: data acquisition of indoor temperature data");
  time(&dataset_date);
  channel_no      = lownibble(data[3]);
  low_battery     = getbits(data[3], 6, 1);

  temperature     = 0.1 * lownibble(data[4]) +
                    1.0 * highnibble(data[4]) +
                   10.0 * lownibble(data[5]) +
                  100.0 * getbits(data[5], 5, 2);
 
  over_underrange = getbits(data[5], 6, 1); 
  sign                    = getbits(data[5], 7, 1); 

  if ( sign == 1) 
    temperature = -temperature;

  syslog(LOG_DEBUG,"tin_dac: low_battery: %d\n", low_battery);
  syslog(LOG_DEBUG,"tin_dac: temperature: %f\n", temperature);
  syslog(LOG_DEBUG,"tin_dac: over_underrange: %d\n", over_underrange);

  /* open sqlite db file */
  if ( ( err = sqlite3_open( wmr9x8station.config.dbfile, &wmr9x8db))) {
    syslog(LOG_ALERT, "statdb: Failed to open database %s. Error: %s\n", 
      wmr9x8station.config.dbfile, sqlite3_errmsg(wmr9x8db));
    return(err);
  }

  statval_db("tin_sensor", "low_battery", dataset_date, (unsigned long int)low_battery, wmr9x8db);
  measval_db("tin_sensor", "temperature", dataset_date, (float)temperature, wmr9x8db);
  statval_db("tin_sensor", "over_underrange", dataset_date, (unsigned long int)over_underrange, wmr9x8db);

  sqlite3_close( wmr9x8db);

  return err;
}

/*
  thb_dac
  temperature, humidity, barometric pressure sensor
  devicetype 0x05 - old version

  untested
*/
int
thb_dac( unsigned char *data) {
  int err;
  unsigned char dew_underrange;
  unsigned char low_battery;
  float temperature;
  float humidity;
  float pressure;
  float dew_temperature;
  unsigned char sign;
  unsigned char over_underrange;
  unsigned char weatherstatus;
  int sealevel_offset;
  time_t dataset_date;

  syslog( LOG_DEBUG,"thb_dac: data acquisition of indoor temperature/humdity/barometer  data");
  time(&dataset_date);
  dew_underrange = getbits( data[3], 4,1);
  low_battery    = getbits( data[3], 6,1);
  temperature    = 0.1 * lownibble(data[4]) +
                   1.0 * highnibble(data[4]) +
                  10.0 * lownibble(data[5]) +
                 100.0 * getbits(data[5], 4,2);

  over_underrange = getbits(data[5], 6, 1);
  sign            = getbits(data[5], 7, 1);

  humidity        = 1.0 * lownibble(data[6]) +
                   10.0 * highnibble(data[6]);

  dew_temperature = 1.0 * lownibble(data[7]) +
                   10.0 * highnibble(data[7]);

  pressure        = 795.0 + 1.0 * data[8];

  weatherstatus   = lownibble(data[9]);

  sealevel_offset = 0.1 * lownibble(data[10]) +
                    1.0 * highnibble(data[10]) +
                   10.0 * lownibble(data[11]) +
                  100.0 * highnibble(data[11]);

  syslog(LOG_DEBUG, "thb_dac: dew_underrange: %d\n", dew_underrange);
  syslog(LOG_DEBUG, "thb_dac: low_battery: %d\n", low_battery);
  syslog(LOG_DEBUG, "thb_dac: temperature: %f\n", temperature);
  syslog(LOG_DEBUG, "thb_dac: over_underrange: %d\n", over_underrange);
  syslog(LOG_DEBUG, "thb_dac: humidity: %f\n", humidity);
  syslog(LOG_DEBUG, "thb_dac: dew_temperature: %f\n", dew_temperature);
  syslog(LOG_DEBUG, "thb_dac: weatherstatus: %x\n", weatherstatus);
  syslog(LOG_DEBUG, "thb_dac: sealevel_offset: %d\n", sealevel_offset);

  /* open sqlite db file */
  if ( ( err = sqlite3_open( wmr9x8station.config.dbfile, &wmr9x8db))) {
    syslog(LOG_ALERT, "statdb: Failed to open database %s. Error: %s\n", 
      wmr9x8station.config.dbfile, sqlite3_errmsg(wmr9x8db));
    return(err);
  }

  statval_db("thb_sensor","dew_underrange", dataset_date, (unsigned long int)dew_underrange, wmr9x8db);
  statval_db("thb_sensor","low_battery", dataset_date, (unsigned long int)low_battery, wmr9x8db);
  measval_db("thb_sensor","temperature", dataset_date, (float)temperature, wmr9x8db);
  measval_db("thb_sensor","humidity", dataset_date, (float)humidity, wmr9x8db);
  measval_db("thb_sensor","dew_temperature", dataset_date, (float)dew_temperature, wmr9x8db);
  measval_db("thb_sensor","pressure", dataset_date, (float)pressure, wmr9x8db);
  statval_db("thb_sensor","over_underrange", dataset_date, (unsigned long int)over_underrange, wmr9x8db);
  statval_db("thb_sensor","weatherstatus", dataset_date, (unsigned long int)weatherstatus, wmr9x8db);
  statval_db("thb_sensor","sealevel_offset", dataset_date, (unsigned long int)sealevel_offset, wmr9x8db);

  sqlite3_close( wmr9x8db);



  return err;
}

/*
  thb_dac
  temperature, humidity, barometric pressure sensor
  devicetype 0x06 - new version
*/
int
thbnew_dac( unsigned char *data) {
  int err;
  unsigned char dew_underrange;
  unsigned char low_battery;
  float temperature;
  float humidity;
  float pressure;
  float dew_temperature;
  unsigned char sign;
  unsigned char over_underrange;
  unsigned char weatherstatus;
  int sealevel_offset;
  time_t dataset_date;

  syslog(LOG_DEBUG,"thbnew_dac: data acquisition of indoor temperature/humdity/barometer data");
  time(&dataset_date);
  dew_underrange = getbits( data[3], 4,1);
  low_battery    = getbits( data[3], 6,1);
  temperature    = 0.1 * lownibble(data[4]) +
                   1.0 * highnibble(data[4]) +
                  10.0 * lownibble(data[5]) +
                 100.0 * getbits(data[5], 4,2);

  over_underrange = getbits(data[5], 6, 1);
  sign            = getbits(data[5], 7, 1);
  if ( sign == 1 ) 
    temperature = -temperature;

  humidity        = 1.0 * lownibble(data[6]) +
                   10.0 * highnibble(data[6]);

  dew_temperature = 1.0 * lownibble(data[7]) +
                   10.0 * highnibble(data[7]);

  pressure        = 600.0 + 1.0 * ( data[8] + ( getbits(data[9], 0, 1) << 8));

  weatherstatus   = highnibble(data[9]);

  sealevel_offset = 0.1 * highnibble(data[10]) +
                    1.0 * lownibble(data[11]) +
                   10.0 * highnibble(data[11]) +
                  100.0 * lownibble(data[12]) +
                 1000.0 * highnibble(data[12]);

  syslog(LOG_DEBUG,"thbnew_sensor: dew_underrange: %d\n", dew_underrange);
  syslog(LOG_DEBUG,"thbnew_sensor: low_battery: %d\n", low_battery);
  syslog(LOG_DEBUG,"thbnew_sensor: temperature: %f\n", temperature);
  syslog(LOG_DEBUG,"thbnew_sensor: over_underrange: %d\n", over_underrange);
  syslog(LOG_DEBUG,"thbnew_sensor: humidity: %f\n", humidity);
  syslog(LOG_DEBUG,"thbnew_sensor: dew_temperature: %f\n", dew_temperature);
  syslog(LOG_DEBUG,"thbnew_sensor: pressure: %f\n", pressure);
  syslog(LOG_DEBUG,"thbnew_sensor: weatherstatus: %d\n", weatherstatus);
  syslog(LOG_DEBUG,"thbnew_sensor: sealevel_offset: %d\n", sealevel_offset);

  /* open sqlite db file */
  if ( ( err = sqlite3_open( wmr9x8station.config.dbfile, &wmr9x8db))) {
    syslog(LOG_ALERT, "statdb: Failed to open database %s. err : %d", 
      wmr9x8station.config.dbfile, err);
    return(err);
  }

  statval_db("thbnew_sensor","dew_underrange", dataset_date, (unsigned long int)dew_underrange, wmr9x8db);
  statval_db("thbnew_sensor","low_battery", dataset_date, (unsigned long int)low_battery, wmr9x8db);
  measval_db("thbnew_sensor","temperature", dataset_date, (float)temperature, wmr9x8db);
  measval_db("thbnew_sensor","humidity", dataset_date, (float)humidity, wmr9x8db);
  measval_db("thbnew_sensor","dew_temperature", dataset_date, (float)dew_temperature, wmr9x8db);
  measval_db("thbnew_sensor","pressure", dataset_date, (float)pressure, wmr9x8db);
  statval_db("thbnew_sensor","over_underrange", dataset_date, (unsigned long int)over_underrange, wmr9x8db);
  statval_db("thbnew_sensor","weatherstatus", dataset_date, (unsigned long int)weatherstatus, wmr9x8db);
  statval_db("thbnew_sensor","sealevel_offset", dataset_date, (unsigned long int)sealevel_offset, wmr9x8db);

  sqlite3_close( wmr9x8db);


  return err;
}

int
minute_dac( unsigned char *data) {
  int err;
  unsigned char onedigit;
  unsigned char tendigit;
  int minute;
  unsigned char low_battery;

  syslog( LOG_DEBUG,"minute_dac: data acquisition of minute data");
  err = 0;
  onedigit = lownibble( data[3]);
  tendigit = getbits( data[3],6,3);
  minute   =      onedigit + 
             10 * tendigit;
  syslog(LOG_DEBUG, "minute_dac: minute: %d\n", minute);

  low_battery = getbits( data[3],7,1);
  syslog(LOG_DEBUG, "minute_dac: low_battery: %d\n", low_battery);

  return err;
}

int
clock_dac( unsigned char *data) {
  int err;
  int minute;
  unsigned char low_battery;
  int hour;
  int day;
  int month;
  int year;
  time_t dataset_date;

  syslog( LOG_DEBUG,"clock_dac: data acquisition of clock data");
  err = 0;
  time(&dataset_date);

  minute      = 1 * lownibble(data[3]) +
               10 * highnibble(data[3]);

  low_battery = getbits(data[3], 7, 1);
  hour        = 1 * lownibble(data[4]) +
               10 * highnibble(data[4]);

  day         = 1 * lownibble(data[5]) +
               10 * highnibble(data[5]);

  month       = 1 * lownibble(data[6]) +
               10 * highnibble(data[6]);

  year        = 1 * lownibble(data[7]) +
               10 * highnibble(data[7]);

  printf("clock: hour:minute %d:%d\n", hour, minute);
  printf("clock: day.month.year %d.%d.%d\n", day, month, year);

  return err;
}

/*  initwmr9x8

  opens serial port for communication
  
  serial port settings:
  WMR928 (and similar stations)
     9600, 8, Even Parity,  2Stop
     raise RTS voltage


*/
int 
initwmr9x8 (int *pfd, struct termios *newtio,
	    struct termios *oldtio) 
{
  int i, itio;

  /* open the device to be non-blocking (read will return immediatly) */

  *pfd = open(wmr9x8station.config.device, O_RDWR| O_NOCTTY| O_NDELAY| O_NONBLOCK );
  if ( *pfd <0) {
    werrno = errno;
    syslog(LOG_INFO, "initwmr9x8: error opening serial device : %s",
	   strerror(werrno));
    return(-1);
  }

  /* is this asynchronous? probably not */
  if ( fcntl(*pfd, F_SETFL, 0) == -1 ) {
    werrno = errno;
    syslog(LOG_INFO, "initwmr9x8: error fcntl: %s",
	   strerror(werrno));
    return(-1);	
  }

  /* save current port settings */
  if ( tcgetattr(*pfd,oldtio) == -1 ) {  
    werrno = errno;
    syslog(LOG_INFO, "initwmr9x8: error tcgetattr oldtio: %s",
	   strerror(werrno));
    return(-1);	
  }

  /* prefill port settings */
  if ( tcgetattr(*pfd,newtio) == -1 ) { 
    werrno = errno;
    syslog(LOG_INFO, "initwmr9x8: error tcgetattr newtio: %s",
	   strerror(werrno));
    return(-1);	
  }
  
  /* set communication link parameters */
  if ( cfsetispeed(newtio, B9600) == -1 ) {
    werrno = errno;
    syslog(LOG_INFO, "initwmr9x8: error cfsetispeed: %s",
	   strerror(werrno));
    return(-1);	
  }
  
  if ( cfsetospeed(newtio, B9600) == -1 ) {
    werrno = errno;
    syslog(LOG_INFO, "initwmr9x8: error cfsetospeed: %s",
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
    syslog(LOG_INFO, "initwmr9x8: error tcsetattr: %s",
	   strerror(werrno));
    return(-1);	
  }

  /* lower DTR and RTS on serial line */
  if ( ioctl(*pfd, TIOCMGET, &itio) == -1 ) {
    werrno = errno;
    syslog(LOG_INFO, "initwmr9x8: error ioctl: %s",
	   strerror(werrno));
    return(-1);	
  }
  itio &= ~TIOCM_DTR;
  itio &= ~TIOCM_RTS;
  if ( ioctl(*pfd, TIOCMSET, &itio) == -1 ) {
    errno = errno;
    syslog(LOG_INFO, "initwmr9x8: error ioctl: %s",
	   strerror(werrno));
    return(-1);	
  }

  /* raise RTS */
  itio |= TIOCM_RTS;
  if ( ioctl(*pfd, TIOCMSET, &itio) == -1 ) {
    werrno = errno;
    syslog(LOG_INFO, "initwmr9x8: error ioctl: %s",
	   strerror(werrno));
    return(-1);	
  }
  return(0);
}


/*


*/
int resetwmr9x8( int fd) {
  int itio;

  /* lower DTR and RTS on serial line */

  itio &= ~TIOCM_DTR;
  itio &= ~TIOCM_RTS;
  if ( ioctl(fd, TIOCMSET, &itio) == -1 ) {
    errno = errno;
    syslog(LOG_INFO, "initwmr9x8: error ioctl: %s",
	   strerror(werrno));
    return(-1);	
  }

  /* raise RTS */
  itio |= TIOCM_RTS;
  if ( ioctl(fd, TIOCMSET, &itio) == -1 ) {
    werrno = errno;
    syslog(LOG_INFO, "initwmr9x8: error ioctl: %s",
	   strerror(werrno));
    return(-1);	
  }
  return(0);
}


/*
   closewmr9x8

   restore the old port settings
   lower DTR on serial line

*/
int closewmr9x8( int fd, struct termios *oldtio) {
    int tset;

    /* lower RTS on serial line */
    if ( ioctl(fd, TIOCMGET, &tset) == -1 ) {
	  werrno = errno;
	  syslog(LOG_INFO, "closewmr9x8: error ioctl: %s",
			 strerror(werrno));
	  return(-1);	
	}
    tset &= ~TIOCM_RTS;
    if ( ioctl(fd, TIOCMSET, &tset) == -1 ) {
      werrno = errno;
      syslog(LOG_INFO, "closewmr9x8: error ioctl: %s",
	     strerror(werrno));
      return(-1);	
    }

    /* restore old port settings */
    /* takes approx 3 sec to reopen fd under FreeBSD
       if tcsetattr is called to restore */
    if ( tcsetattr(fd,TCSANOW,oldtio) == -1 ) {
	  werrno = errno;
	  syslog(LOG_INFO, "closewmr9x8: error ioctl: %s",
			 strerror(werrno));
	  return(-1);	
	}
	
    /* close serial port */
    if ( close(fd) == -1 ) {
	  werrno = errno;
	  syslog(LOG_INFO, "closewmr9x8: error close: %s",
			 strerror(werrno));
	  return(-1);	
    }
    return(0);
}


int 
getschar (int fd, unsigned char *schar) 
{
  int err = -1;
  unsigned char rbyte;
  int maxfd = fd + 1;
  fd_set readfs;
  struct timeval tv;

  tv.tv_sec  = 7;
  tv.tv_usec = 0;

  FD_ZERO(&readfs);
  FD_SET(fd, &readfs);

  err = select(maxfd, &readfs, NULL, NULL, &tv);

  if ( err < 0 ) {
    if ( errno == EBADF ) 
      syslog(LOG_ALERT,"getschar: select error: EBADF");
    else if ( errno == EINTR ) 
      syslog(LOG_ALERT,"getschar: select error: EINTR");
    else if ( errno == EINVAL ) 
      syslog(LOG_ALERT,"getschar: select error: EINVAL");
    else
      syslog(LOG_ALERT,"getschar: select error");
  }

  if ( err && FD_ISSET (fd, &readfs)) {

    if ((err = read (fd, &rbyte, 1)) < 0)  { 
      syslog (LOG_ALERT,"getschar: reading device failed \n");
      return (err);
    }
    else if (err == 1) {
      *schar = rbyte;
      return( err);
    }
  }
  return( err);
}

int
shuffdat( unsigned char *data, int ndat) {
  int i;
  unsigned char tdat[TBUFF+1]; 
  tdat[0] = 0xff;
  tdat[1] = 0xff;

  for ( i = 0; i < ndat-2; i++) {
    tdat[i+2] = data[i];
  }

  for ( i = 0; i < ndat; i++) 
    data[i] = tdat[i];

  return(0);
}

int
checksum ( unsigned char *data, int ndat) {
  int i, err;
  unsigned  char checksum;

  for ( i = 0; i < ndat -1; i++ )
    checksum = checksum + data[i];
 
  if ( checksum != data [ndat-1] ) {
    syslog(LOG_ALERT,"checksum error: in record: %x calculated: %x", 
      checksum, data[ndat-1]); 
    err = 1;
  } 
  syslog(LOG_DEBUG, "checksum calculated: %x", checksum);
  syslog(LOG_DEBUG, "checksum: last data byte: %x", data[ndat-1]);
  return(err);
}

