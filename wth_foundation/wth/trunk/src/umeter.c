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
  int err;
  int  base=16;
  char umeterstr[5];
  int out_th_present = 0;

  /* parameters in data logger mode */
  float temp_out;
  float rain_total;
  float baro;
  float humid_out;
  float rain_today;
  float windspeed_avg;
  float windspeed;
  float winddir;
  float temp_in;
  float humid_in;
  int day_of_year;
  int min_of_day;

  time_t umclock;
  int minute, hour, year;
  struct tm tm;
  struct tm *ptm;
  char buf[TBUFF+1];
  time_t dataset_date;
  int tdiff;

  char tstrg[TBUFF+1];
  char rrdfile[TBUFF+1];
  char tmpstr[TBUFF+1];
  char template[TBUFF+1];
  char **ustrg;

  syslog(LOG_DEBUG, "datalog_rd: data: %s\n", datalogdata);
  time(&dataset_date);

  /* open sqlite db file */

  if ( sqlite3_open( (const char *)umeterstation.config.dbfile, &umeterdb) != SQLITE_OK ) {
    return(1);
  }

  ustrg = malloc(sizeof(char)*TBUFF);
  ustrg[2] = malloc(sizeof(char)*NBUFF);

  /* Wind Speed */
  strncpy(umeterstr, (const char * )(datalogdata+2), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  windspeed = strtol(umeterstr, NULL, base);
  windspeed = (1.0/36.0)*windspeed; /* 0.1 kph to ms-1 */
  syslog(LOG_DEBUG, "windspeed: %f\n", windspeed);
  measval_db( "WindSensor", "Wind Speed", dataset_date, (float)windspeed, umeterdb);

  /* Wind direction */
  strncpy(umeterstr, (const char *)(datalogdata+6), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  winddir = strtol(umeterstr, NULL, base);
  winddir = (360.0/255.0)*winddir; /* 0-255 to 0-360 deg */
  syslog(LOG_DEBUG, "winddir: %f\n", winddir);
  measval_db( "WindSensor", "Wind Direction", dataset_date, (float)winddir, umeterdb);

  /* Outdoor temperature */
  strncpy(umeterstr, (const char *)(datalogdata+10), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  temp_out = strtol(umeterstr, NULL, base);
  temp_out = ((1.0/10.0)*temp_out -32.0)*5.0/9.0; /* 0.1 degF to degC */
  /* temperature data will not be written to database until it is clear that no TH sensor is installed */
  syslog(LOG_DEBUG, "tempout: %f\n", temp_out);

  /* Rain longterm total */
  strncpy(umeterstr, (const char *)(datalogdata+14), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  rain_total = strtol(umeterstr, NULL, base);
  rain_total = (25.4/10.0)*rain_total; /* 0.1in to mm */
  syslog(LOG_DEBUG, "rain_total: %f\n", rain_total);
  measval_db( "RainGauge", "Rain Longterm Total", dataset_date, (float)rain_total, umeterdb);

  /* Barometer */
  strncpy(umeterstr, (const char *)(datalogdata+18), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  baro = strtol(umeterstr, NULL, base);
  baro = baro/10.0; /* 0.1mbar to mbar */
  syslog(LOG_DEBUG, "baro: %f\n", baro);
  measval_db( "IndoorTemperator_BarometerSensor", "Barometer", 
	      dataset_date, (float)baro, umeterdb);

  /* indoor temperature */
  strncpy(umeterstr, (const char *)(datalogdata+22), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  temp_in = strtol(umeterstr, NULL, base);
  temp_in = ((1.0/10.0)*temp_in -32.0)*5.0/9.0; /* 0.1 degF to degC */
  syslog(LOG_DEBUG, "temp_in: %f\n", temp_in);
  measval_db( "IndoorTemperator_BarometerSensor", "Indoor temp", 
	      dataset_date, (float)temp_in, umeterdb);

  /* Outdoor humdity - if sensor is installed */
  strncpy(umeterstr, (const char *)(datalogdata+26), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  err = strncmp( umeterstr, "----", 4);
  /* Temperature Sensor installed */
  if ( err == 0 ) {
    syslog(LOG_INFO, "No Outdoor Humidity/Temperature Sensor found");
    measval_db( "TemperatureSensor", "Current Outdoor Temp", 
		dataset_date, (float)temp_out, umeterdb);
  } else /* Outdoor Humidity Temperature sensor installed */ {
    out_th_present = 1;
    humid_out = (float)strtol(umeterstr, NULL, base);
    humid_out = (1.0/10.0)*humid_out; /* 0.1% to %rel.hum. */
    measval_db( "OutdoorHumidity_TemperatureSensor", "Current Outdoor Temp", 
		dataset_date, (float)temp_out, umeterdb);
    measval_db( "OutdoorHumidity_TemperatureSensor", "Current Outdoor Humidity", 
		dataset_date, (float)humid_out, umeterdb);
    syslog(LOG_DEBUG, "humid_out: %f\n", humid_out); 
  }

  /* indoor humidity - if sensor is installed */
  strncpy(umeterstr, (const char *)(datalogdata+30), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  err = strncmp( umeterstr, "----", 4);
  /* Indoor Humidity Sensor installed */
  if ( err == 0 ) {
    syslog(LOG_INFO, "No Indoor HumiditySensor found");
  } else {
    humid_in = strtol(umeterstr, NULL, base);
    humid_in = (1.0/10.0)*humid_in; /* 0.1% to %rel.hum. */
    syslog(LOG_DEBUG, "humid_in: %f\n", humid_in);
    measval_db( "IndoorHumiditySensor", "Indoor Humidity", dataset_date, (float)humid_in, umeterdb);
  }

  /* day of year */
  strncpy(umeterstr, (const char *)(datalogdata+34), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  day_of_year = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "day_of_year: %d\n", day_of_year);

  /* minute of day */
  strncpy(umeterstr, (const char *)(datalogdata+38), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  min_of_day = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "min_of_day: %d\n", min_of_day);

  memset(&tm, 0, sizeof(struct tm ));
  time(&umclock);
  ptm = gmtime(&umclock);
  year = 1900 + ptm->tm_year;
  minute  = min_of_day % 60;
  hour = min_of_day / 60 - 1;

  snprintf( buf, sizeof(buf), "%d %d %02d:%02d:00", year, day_of_year+1, hour, minute);
  strptime(buf,"%Y %j %H:%M:%S", &tm);
  strftime(buf, sizeof(buf), "%d %b %Y %H:%M", &tm);
  syslog(LOG_DEBUG, "packet_rd: %s", buf);

  umclock = mktime( &tm);
  tdiff = ( int)(umclock - dataset_date);
  syslog(LOG_DEBUG,"time difference Ultimeter clock  - PC clock: %d [s]", tdiff);
  if ( abs(tdiff) > 3600 )
    syslog(LOG_ALERT, "time difference Ultimeter clock  - PC clock: %d [s]", tdiff);

  /* Today's rain total */
  strncpy(umeterstr, (const char *)(datalogdata+42), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  rain_today = strtol(umeterstr, NULL, base);
  rain_today = (25.4/10.0)*rain_today; /* 0.1in to mm */
  syslog(LOG_DEBUG, "rain_today: %f\n", rain_today);
  measval_db( "RainGauge", "Today Rain Total", dataset_date, (float)rain_today, umeterdb);

  /* 1 Minute windspeed average */
  strncpy(umeterstr, (const char *)(datalogdata+46), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  windspeed_avg = strtol(umeterstr, NULL, base);
  windspeed_avg = (1.0/36.0)*windspeed_avg; /* 0.1 kph to ms-1 */
  syslog(LOG_DEBUG, "windspeed_avg: %f\n", windspeed_avg);
  measval_db( "WindSensor", "1 Minute Windspeed Average", 
	      dataset_date, (float)windspeed_avg, umeterdb);

  snprintf(tstrg,MAXMSGLEN, "%lu", (long int)dataset_date);
  snprintf(template,MAXMSGLEN,"%f", windspeed);
  strncat(tstrg, ":", 1);
  strncat(tstrg, template, strlen(template));
  strncat(tstrg, ":", 1);
  snprintf(template,MAXMSGLEN,"%f", winddir);
  strncat(tstrg, template, strlen(template));
  strncat(tstrg, ":", 1);
  snprintf(template,MAXMSGLEN,"%f", windspeed_avg);
  strncat(tstrg, template, strlen(template));

  snprintf( rrdfile, MAXMSGLEN, "%s", umeterstation.config.rrdpath);
  snprintf( tmpstr, MAXMSGLEN, "%s.rrd", "WindSensor");
  strncat( rrdfile, tmpstr, 2*MAXMSGLEN+1);
  syslog(LOG_DEBUG, "packet_rd: rrdfile: %s", rrdfile);
  syslog(LOG_DEBUG, "packet_rd: update string: %s", tstrg);
  snprintf(ustrg[2], MAXMSGLEN-2, "%s", tstrg);
 

  rrd_clear_error();
  rrd_get_context();
  rrd_update_r( rrdfile, NULL, 1, (const char **)(ustrg + 2));
  if ( ( err = rrd_test_error())) {
    syslog( LOG_ALERT, "packet_rd: RRD return code: %d",
            rrd_test_error());
    syslog(LOG_ALERT, "packet_rd: RRD error: %s", rrd_get_error());
  } else {
    syslog(LOG_DEBUG,"packet_rd: rrd_test_error return code OK");
  }

  free(ustrg[2]);
  free(ustrg);

  sqlite3_close( umeterdb);
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

  time_t umclock;
  int minute, hour, year;
  struct tm tm;
  struct tm *ptm;
  char buf[TBUFF+1];
  time_t dataset_date;
  int tdiff;

  char tstrg[TBUFF+1];
  char rrdfile[TBUFF+1];
  char tmpstr[TBUFF+1];
  char template[TBUFF+1];
  char **ustrg;

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

  ustrg = malloc(sizeof(char)*TBUFF);
  ustrg[2] = malloc(sizeof(char)*NBUFF);

  /* Windspeed Peak */
  strncpy(umeterstr, (const char *)(packetdata+5), 5); 
  umeterstr[4] = 0;
  windspeed_peak = (float)strtol(umeterstr, NULL, base);
  windspeed_peak = (1.0/36.0)*windspeed_peak; /* 0.1 kph to ms-1 */
  syslog(LOG_DEBUG, "packet_rd: windspeed_peak: %f\n", windspeed_peak);
  measval_db( "WindSensor", "Windspeed Peak 5 Minute", 
	      dataset_date, (float)windspeed_peak, umeterdb);

  /* Wind Direction */
  strncpy(umeterstr, (const char *)(packetdata+9), 5); 
  umeterstr[4] = 0;
  winddir_peak = (float)strtol(umeterstr, NULL, base);
  winddir_peak = (360.0/255.0)*winddir_peak; /* 0-255 to 0-360 deg */
  measval_db( "WindSensor", "Wind Direction", dataset_date, (float)winddir_peak, umeterdb);
  syslog(LOG_DEBUG, "packet_rd: winddir_peak: %f\n", winddir_peak);

  strncpy(umeterstr, (const char *)(packetdata+13), 5); 
  umeterstr[4] = 0;
  temp_out = (float)strtol(umeterstr, NULL, base);
  temp_out = ((1.0/10.0)*temp_out -32.0)*5.0/9.0; /* 0.1 degF to degC */
  /* temperature data will not be written to database until it is clear that no TH sensor is installed */
  syslog(LOG_DEBUG, "packet_rd: temp_out: %f\n", temp_out);

  strncpy(umeterstr, (const char *)(packetdata+17), 5); 
  umeterstr[4] = 0;
  rain_total = (float)strtol(umeterstr, NULL, base);
  rain_total = (25.4/10.0)*rain_total; /* 0.1in to mm */
  measval_db( "RainGauge", "Rain Longterm Total", dataset_date, (float)rain_total, umeterdb);
  syslog(LOG_DEBUG, "packet_rd: rain_total: %f\n", rain_total);

  strncpy(umeterstr, (const char *)(packetdata+21), 5); 
  umeterstr[4] = 0;
  baro = (float)strtol(umeterstr, NULL, base);
  baro = baro/10.0; /* 0.1mbar to mbar */
  measval_db( "IndoorTemperator_BarometerSensor", "Current Barometer", dataset_date, (float)baro, umeterdb);
  syslog(LOG_DEBUG, "packet_rd: baro: %f\n", baro);

  strncpy(umeterstr, (const char *)(packetdata+25), 5); 
  umeterstr[4] = 0;
  baro_chg = (float)strtol(umeterstr, NULL, base); 
  baro_chg = baro_chg/10.0; /* 0.1mbar to mbar */
  measval_db( "IndoorTemperator_BarometerSensor", "Barometer Delta", dataset_date, (float)baro_chg, umeterdb);
  syslog(LOG_DEBUG, "packet_rd: baro_chg: %f\n", baro_chg);

  strncpy(umeterlstr, (const char *)(packetdata+29), 9); 
  umeterlstr[8] = 0;
  baro_corr = strtol(umeterlstr, NULL, base);
  measval_db( "IndoorTemperator_BarometerSensor", "Barometer Correction Factor", dataset_date, (float)baro_corr, umeterdb);
  syslog(LOG_DEBUG, "packet_rd: baro_corr: %d\n", baro_corr);

  strncpy(umeterstr, (const char *)(packetdata+37), 5); 
  umeterstr[4] = 0;
  err = strncmp( umeterstr, "----", 4);
  /* Temperature Sensor installed */
  if ( err == 0 ) {
    syslog(LOG_INFO, "packet_rd: No Outdoor Humidity/Temperature Sensor found");
    measval_db( "TemperatureSensor", "Current Outdoor Temp", 
		dataset_date, (float)temp_out, umeterdb);
  } else /* Outdoor Humidity Temperature sensor installed */ {
    out_th_present = 1;
    humid_out = (float)strtol(umeterstr, NULL, base);
    humid_out = (1.0/10.0)*humid_out; /* 0.1% to %rel.hum. */
    measval_db( "OutdoorHumidity_TemperatureSensor", "Current Outdoor Temp", 
		dataset_date, (float)temp_out, umeterdb);
    measval_db( "OutdoorHumidity_TemperatureSensor", "Current Outdoor Humidity", 
		dataset_date, (float)humid_out, umeterdb);
    syslog(LOG_DEBUG, "packet_rd: humid_out: %f\n", humid_out); 
  }

  /* day of year */
  strncpy(umeterstr, (const char *)(packetdata+41), 5); 
  umeterstr[4] = 0;
  day_of_year = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "packet_rd: day_of_year: %d\n", day_of_year);

  /* minute of day */
  strncpy(umeterstr, (const char *)(packetdata+45), 5); 
  umeterstr[4] = 0;
  min_of_day = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "packet_rd: min_of_day: %d\n", min_of_day);

  /* Ultimeter clock deviation from PC clock */
  memset(&tm, 0, sizeof(struct tm ));
  time(&umclock);
  ptm = gmtime(&umclock);
  year = 1900 + ptm->tm_year;
  minute  = min_of_day % 60;
  hour = min_of_day / 60 -1 ;

  snprintf( buf, sizeof(buf), "%d %d %02d:%02d:00", year, day_of_year+1, hour, minute);
  strptime(buf,"%Y %j %H:%M:%S", &tm);
  strftime(buf, sizeof(buf), "%d %b %Y %H:%M", &tm);
  syslog(LOG_DEBUG, "packet_rd: %s", buf);

  umclock = mktime( &tm);
  tdiff = ( int)(umclock - dataset_date);
  syslog(LOG_DEBUG,"packet_rd: time difference Ultimeter clock  - PC clock: %d [s]", tdiff);
  if ( abs(tdiff) > 3600 )
    syslog(LOG_ALERT, "packet_rd: time difference Ultimeter clock  - PC clock: %d [s]", tdiff);

  /* today's rain total */
  strncpy(umeterstr, (const char *)(packetdata+49), 5); 
  umeterstr[4] = 0;
  rain_today = (float)strtol(umeterstr, NULL, base);
  rain_today = (25.4/10.0)*rain_today; /* 0.1in to mm */
  measval_db( "RainGauge", "Today Rain Total", 
	      dataset_date, (float)rain_total, umeterdb);
  syslog(LOG_DEBUG, "packet_rd: rain_today: %f\n", rain_today);

  /* 5 Minute windspeed average */
  strncpy(umeterstr, (const char *)(packetdata+53), 5); 
  umeterstr[4] = 0;
  windspeed_avg = (float)strtol(umeterstr, NULL, base);
  windspeed_avg = (1.0/36.0)*windspeed_avg; /* 0.1 kph to ms-1 */
  measval_db( "WindSensor", "5 Minute Windspeed Average", 
	      dataset_date, (float)windspeed_avg, umeterdb);
  syslog(LOG_DEBUG, "packet_rd: windspeed_avg: %f\n", windspeed_avg);


  snprintf(tstrg,MAXMSGLEN, "%lu", (long int)dataset_date);
  snprintf(template,MAXMSGLEN,"%f", windspeed_peak);
  strncat(tstrg, ":", 1);
  strncat(tstrg, template, strlen(template));
  strncat(tstrg, ":", 1);
  snprintf(template,MAXMSGLEN,"%f", winddir_peak);
  strncat(tstrg, template, strlen(template));
  strncat(tstrg, ":", 1);
  snprintf(template,MAXMSGLEN,"%f", windspeed_avg);
  strncat(tstrg, template, strlen(template));

  snprintf( rrdfile, MAXMSGLEN, "%s", umeterstation.config.rrdpath);
  snprintf( tmpstr, MAXMSGLEN, "%s.rrd", "WindSensor");
  strncat( rrdfile, tmpstr, 2*MAXMSGLEN+1);
  syslog(LOG_DEBUG, "packet_rd: rrdfile: %s", rrdfile);
  syslog(LOG_DEBUG, "packet_rd: update string: %s", tstrg);
  snprintf(ustrg[2], MAXMSGLEN-2, "%s", tstrg);
 
  rrd_clear_error();
  rrd_get_context();
  rrd_update_r( rrdfile, NULL, 1, (const char **)(ustrg + 2));
  if ( ( err = rrd_test_error())) {
    syslog( LOG_ALERT, "packet_rd: RRD return code: %d",
            rrd_test_error());
    syslog(LOG_ALERT, "packet_rd: RRD error: %s", rrd_get_error());
  } else {
    syslog(LOG_DEBUG,"packet_rd: rrd_test_error return code OK");
  }

  free(ustrg[2]);
  free(ustrg);

  sqlite3_close( umeterdb);
  return(0);
}

/*
  complete_rd

  reading data records with ULTIMETER in complete mode

*/
int
complete_rd( unsigned char * completedata, int ndat) {
  int err = 0;
  time_t dataset_date;

  int  base=16;
  char umeterstr[5];
  char umeterlstr[9];
  int out_th_present = 0;

  /* parameters in complete record mode */
  float windspeed;
  float winddir;
  float windspeed_5min_peak;
  float winddir_5min_peak;
  float windchill;
  float temp_out;
  float rain_total_today;
  float baro;
  float baro_3hr_chg;
  unsigned int baro_corr; // 8 bytes
  float temp_in;
  float humid_out;
  float humid_in;
  float dew_point;

  int day_of_year;
  int min_of_day;

  float chill_today;
  float chill_today_time;

  float chill_yday;
  float chill_yday_time;

  float lowchill_longterm_date;
  float lowchill_longterm;
  float lowchill_longterm_time;

  float lowtemp_out_today;
  float lowtemp_out_today_time;

  float lowtemp_out_yday;
  float lowtemp_out_yday_time;

  float lowtemp_out_longterm_date;
  float lowtemp_out_longterm;
  float lowtemp_out_longterm_time;

  float lowbaro_today;
  float lowbaro_today_time;


  float lowbaro_yday;
  float lowbaro_yday_time;

  float lowbaro_longterm_date;
  float lowbaro_longterm;
  float lowbaro_longterm_time;

  float lowtemp_in_today;
  float lowtemp_in_today_time;

  float lowtemp_in_yday;
  float lowtemp_in_yday_time;

  float lowtemp_in_longterm_date;
  float lowtemp_in_longterm;
  float lowtemp_in_longterm_time;

  float lowhumid_out_today;
  float lowhumid_out_today_time;

  float lowhumid_out_yday;
  float lowhumid_out_yday_time;

  float lowhumid_out_longterm_date;
  float lowhumid_out_longterm;
  float lowhumid_out_longterm_time;

  float lowhumid_in_today;
  float lowhumid_in_today_time;

  float lowhumid_in_yday;
  float lowhumid_in_yday_time;

  float lowhumid_in_longterm_date;
  float lowhumid_in_longterm;
  float lowhumid_in_longterm_time;

  float windspeed_today;
  float windspeed_today_time;

  float windspeed_yday;
  float windspeed_yday_time;

  float windspeed_longterm_date;
  float windspeed_longterm;
  float windspeed_longterm_time;

  float hightemp_out_today;
  float hightemp_out_today_time;


  float hightemp_out_yday;
  float hightemp_out_yday_time;

  float hightemp_out_longterm_date;
  float hightemp_out_longterm;
  float hightemp_out_longterm_time;

  float highbaro_today;
  float highbaro_today_time;

  float highbaro_yday;
  float highbaro_yday_time;

  float highbaro_longterm_date;
  float highbaro_longterm;
  float highbaro_longterm_time;

  float hightemp_in_today;
  float hightemp_in_today_time;

  float hightemp_in_yday;
  float hightemp_in_yday_time;

  float hightemp_in_longterm_date;
  float hightemp_in_longterm;
  float hightemp_in_longterm_time;

  float highhumid_out_today;
  float highhumid_out_today_time;

  float highhumid_out_yday;
  float highhumid_out_yday_time;

  float highhumid_out_longterm_date;
  float highhumid_out_longterm;
  float highhumid_out_longterm_time;

  float highhumid_in_today;
  float highhumid_in_today_time;

  float highhumid_in_yday;
  float highhumid_in_yday_time;

  float highhumid_in_longterm_date;
  float highhumid_in_longterm;
  float highhumid_in_longterm_time;

  float rain_total_yday;
  float rain_total_longterm_date;
  float rain_total_longterm;

  int leap_year;
  int WDCF;
  int winddir_today;
  int winddir_yday;
  int spare;
  int winddir_longterm;

  float windspeed_avg_1min;

  time_t umclock;
  int minute, hour, year;
  struct tm tm;
  struct tm *ptm;
  char buf[TBUFF+1];

  int tdiff;

  char tstrg[TBUFF+1];
  char rrdfile[TBUFF+1];
  char tmpstr[TBUFF+1];
  char template[TBUFF+1];
  char **ustrg;

  syslog(LOG_DEBUG, "complete_rd: begin of execution");
  time(&dataset_date);
  /*
    parse data in complete mode
  */
  syslog(LOG_DEBUG,"complete_rd: data: '%s'\n", completedata);

  /* open sqlite db file */
  if ( ( err = sqlite3_open( umeterstation.config.dbfile, &umeterdb))) {
    syslog(LOG_ALERT, "statdb: Failed to open database %s. Error: %s\n", 
      umeterstation.config.dbfile, sqlite3_errmsg(umeterdb));
    return(err);
  }
  ustrg = malloc(sizeof(char)*TBUFF);
  ustrg[2] = malloc(sizeof(char)*NBUFF);

  /* 1. Wind Speed */
  strncpy(umeterstr, (const char * )(completedata+4), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  windspeed = strtol(umeterstr, NULL, base);
  windspeed = (1.0/36.0)*windspeed; /* 0.1 kph to ms-1 */
  syslog(LOG_DEBUG, "windspeed: %f\n", windspeed);
  //measval_db( "WindSensor", "Wind Speed", dataset_date, (float)windspeed, umeterdb);

  /* 2. Wind direction */
  strncpy(umeterstr, (const char *)(completedata+8), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  winddir = strtol(umeterstr, NULL, base);
  winddir = (360.0/255.0)*winddir; /* 0-255 to 0-360 deg */
  syslog(LOG_DEBUG, "winddir: %f\n", winddir);
  //measval_db( "WindSensor", "Wind Direction", dataset_date, (float)winddir, umeterdb);

  /* 3. 5 min Windspeed Peak */
  strncpy(umeterstr, (const char *)(completedata+12), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  windspeed_5min_peak = strtol(umeterstr, NULL, base);
  windspeed_5min_peak = (1.0/36.0)*windspeed_5min_peak; /* kph to ms-1 */
  syslog(LOG_DEBUG, "windspeed_5min_peak: %f\n", windspeed_5min_peak);

  /* 4. 5min Winddirection Peak */
  strncpy(umeterstr, (const char *)(completedata+16), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  winddir_5min_peak = strtol(umeterstr, NULL, base);
  winddir_5min_peak = (360.0/255.0)*winddir_5min_peak; /* 0-255 to 0-360 deg */
  syslog(LOG_DEBUG, "winddir_5min_peak: %f\n", winddir_5min_peak);

  /* 5. Wind Chill */
  strncpy(umeterstr, (const char *)(completedata+20), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  windchill = strtol(umeterstr, NULL, base);
  windchill = ((1.0/10.0)*windchill -32.0)*5.0/9.0; /* 0.1 degF to degC */
  syslog(LOG_DEBUG, "windchill: %f\n", windchill);

  /* 6. Outdoor Temperature */
  strncpy(umeterstr, (const char *)(completedata+24), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  temp_out = strtol(umeterstr, NULL, base);
  temp_out = ((1.0/10.0)*temp_out -32.0)*5.0/9.0; /* 0.1 degF to degC */
  syslog(LOG_DEBUG, "temp_out: %f\n", temp_out);

  /* 7. Rain Total for today */
  strncpy(umeterstr, (const char *)(completedata+28), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  rain_total_today = strtol(umeterstr, NULL, base);
  rain_total_today = (25.4/10.0)*rain_total_today; /* 0.1in to mm */
  syslog(LOG_DEBUG, "rain_total_today: %f\n", rain_total_today);

  /* 8. Barometer */
  strncpy(umeterstr, (const char *)(completedata+32), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  baro = strtol(umeterstr, NULL, base);
  baro = baro/10.0; /* 0.1mbar to mbar */
  syslog(LOG_DEBUG, "baro: %f\n", baro);

  /* 9. Barometer 3-hour Pressure Change */
  strncpy(umeterstr, (const char *)(completedata+36), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  baro_3hr_chg = strtol(umeterstr, NULL, base);
  baro_3hr_chg = baro_3hr_chg/10.0; /* 0.1 mbar to mbar */
  syslog(LOG_DEBUG, "baro_3hr_chg: %f\n", baro_3hr_chg);

  /* 10./11. Barometer Correction Factor LSW/MSW */
  strncpy(umeterlstr, (const char *)(completedata+40), 9); 
  umeterlstr[8] = 0;
  baro_corr = strtol(umeterlstr, NULL, base);
  syslog(LOG_DEBUG, "packet_rd: baro_corr: %d\n", baro_corr);

  /* 12. Indoor Temperature */
  strncpy(umeterstr, (const char *)(completedata+48), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  temp_in = strtol(umeterstr, NULL, base);
  temp_in = ((1.0/10.0)*temp_in -32.0)*5.0/9.0; /* 0.1 degF to degC */
  syslog(LOG_DEBUG, "temp_in: %f\n", temp_in);

  /* 13. Outdoor humidity */
  strncpy(umeterstr, (const char *)(completedata+52), 5); 
  umeterstr[4] = 0;
  err = strncmp( umeterstr, "----", 4);
  /* Temperature Sensor installed */
  if ( err == 0 ) {
    syslog(LOG_INFO, "packet_rd: No Outdoor Humidity/Temperature Sensor found");
  } else /* Outdoor Humidity Temperature sensor installed */ {
    out_th_present = 1;
    humid_out = (float)strtol(umeterstr, NULL, base);
    humid_out = (1.0/10.0)*humid_out; /* 0.1% to %rel.hum. */
    syslog(LOG_DEBUG, "packet_rd: humid_out: %f\n", humid_out); 
  }

  free(ustrg[2]);
  free(ustrg);

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
  static unsigned char data[SBUFF+1];

  const char dataloghd[3]  = "!!";
  const char packethd[6]   = "$ULTW";
  const char complethd[5] = "&CR&";
  int dataloglen  = 51; /* NL is ignored */
                               /*  thus record size is 48 hex bytes + 2 header bytes + carriage return */
  int packetlen   = 58; /* NL is ignored */
                               /* thus record size is 52 hex bytes + 5 header bytes + carriage */
  int completlen  = 457;  /* NL is ignored */
                               /* thus record size is 452 hex bytes + 4 header bytes + carriage */

  printf("umeter_rd: start of execution\n");

  for (;;) {
    ndat = read(rfd,data,SBUFF); 
    data[ndat-1]=0;

    /* check data logger mode */
    if ( ( strncmp( (const char *)data, (const char *)dataloghd, 2) == 0) 
          && ( ndat == dataloglen) ) {
      syslog(LOG_DEBUG, "umeter_rd: datalogger mode\n");
      datalogger_rd( data, ndat);
      /* check packet mode */
    } else if ( ( strncmp( (const char *)data, (const char *)packethd, 5) == 0) 
          && ( ndat ==packetlen )) {
      syslog(LOG_DEBUG, "umeter_rd: packet mode\n");
      packet_rd( data, ndat);
      /* check complete record mode */
    } else if ( ( strncmp( (const char *)data, (const char *)complethd, 4) == 0) 
          && ( ndat ==completlen )) {
      syslog(LOG_DEBUG, "umeter_rd: complete record mode\n");
      complete_rd( data, ndat);
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
  //newtio->c_iflag = IGNPAR | IGNCR | ICRNL;
  newtio->c_iflag = IGNPAR | IGNCR; 
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

