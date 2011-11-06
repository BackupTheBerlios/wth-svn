/* 
  umeter.c

  ULTIMETER weatherstation handler implemented as POSIX thread

  $Id$
  $Revision$

  Copyright (C) 2010,2011 Volker Jahns <volker@thalreit.de>

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
  int err;
  syslog( LOG_DEBUG, "umeter_hd: start of execution");

  /* read ULTIMETER weatherstation */
  err = umeter_rd();

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
  float rain_longterm_total;
  float baro;
  float humid_out;
  float rain_today_total;
  float windspeed_1min_avg;
  float windspeed;
  float winddir;
  float temp_in;
  float humid_in;
  int day_of_year;
  int min_of_day;

  struct tm *ptm;
  char buf[TBUFF+1];
  time_t dataset_date;
  int tdiff;

  syslog(LOG_DEBUG, "datalog_rd: data: %s\n", datalogdata);
  time(&dataset_date);

  /* open sqlite db file */

  if ( sqlite3_open( (const char *)umeterstation.config.dbfile, &umeterdb) != SQLITE_OK ) {
    return(1);
  }

  /* Wind Speed */
  strncpy(umeterstr, (const char * )(datalogdata+2), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  windspeed = strtol(umeterstr, NULL, base);
  windspeed = (1.0/36.0)*windspeed; /* 0.1 kph to ms-1 */
  syslog(LOG_DEBUG, "windspeed: %f\n", windspeed);
  measval_db( "windsensor", "windspeed", dataset_date, (float)windspeed, umeterdb);

  /* Wind direction */
  strncpy(umeterstr, (const char *)(datalogdata+6), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  winddir = strtol(umeterstr, NULL, base);
  winddir = (360.0/255.0)*winddir; /* 0-255 to 0-360 deg */
  syslog(LOG_DEBUG, "winddir: %f\n", winddir);
  measval_db( "windsensor", "winddirection", dataset_date, (float)winddir, umeterdb);

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
  rain_longterm_total = strtol(umeterstr, NULL, base);
  rain_longterm_total = (25.4/10.0)*rain_longterm_total; /* 0.1in to mm */
  syslog(LOG_DEBUG, "rain_longterm_total: %f\n", rain_longterm_total);
  measval_db( "raingauge", "rain_total", dataset_date, (float)rain_longterm_total, umeterdb);

  /* Barometer */
  strncpy(umeterstr, (const char *)(datalogdata+18), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  baro = strtol(umeterstr, NULL, base);
  baro = baro/10.0; /* 0.1mbar to mbar */
  syslog(LOG_DEBUG, "baro: %f\n", baro);
  measval_db( "tp_in_sensor", "pressure", 
	      dataset_date, (float)baro, umeterdb);

  /* Indoor Temperature */
  strncpy(umeterstr, (const char *)(datalogdata+22), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  temp_in = strtol(umeterstr, NULL, base);
  temp_in = ((1.0/10.0)*temp_in -32.0)*5.0/9.0; /* 0.1 degF to degC */
  syslog(LOG_DEBUG, "temp_in: %f\n", temp_in);
  measval_db( "tp_in_sensor", "indoor_temp", 
	      dataset_date, (float)temp_in, umeterdb);

  /* Outdoor humdity - if sensor is installed */
  strncpy(umeterstr, (const char *)(datalogdata+26), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  err = strncmp( umeterstr, "----", 4);
  /* Temperature Sensor installed */
  if ( err == 0 ) {
    syslog(LOG_INFO, "No Outdoor Humidity/Temperature Sensor found");
    measval_db( "t_out_sensor", "outdoor_temp", 
		dataset_date, (float)temp_out, umeterdb);
  } else /* Outdoor Humidity Temperature sensor installed */ {
    out_th_present = 1;
    humid_out = (float)strtol(umeterstr, NULL, base);
    humid_out = (1.0/10.0)*humid_out; /* 0.1% to %rel.hum. */
    measval_db( "th_out_sensor", "outdoor_temp", 
		dataset_date, (float)temp_out, umeterdb);
    measval_db( "th_out_sensor", "outdoor_hum", 
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
    measval_db( "h_in_sensor", "indoor_hum", dataset_date, (float)humid_in, umeterdb);
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

  /* Ultimeter clock deviation from PC clock */
  ptm = localtime(&dataset_date);
  strftime( buf, sizeof(buf), "%j", ptm);
  syslog(LOG_DEBUG,"complete_rd: dataset_date day_of_year :  %s", buf);
  tdiff = day_of_year - atoi(buf);
  if ( tdiff != 0 )
    syslog(LOG_ALERT, 
      "complete_rd: time difference Ultimeter clock  - PC clock: %d days", 
      tdiff);

  /* Today's rain total */
  strncpy(umeterstr, (const char *)(datalogdata+42), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  rain_today_total = strtol(umeterstr, NULL, base);
  rain_today_total = (25.4/10.0)*rain_today_total; /* 0.1in to mm */
  syslog(LOG_DEBUG, "rain_today_total: %f\n", rain_today_total);
  measval_db( "raingauge", "Rain Today Total", dataset_date, (float)rain_today_total, umeterdb);

  /* 1 Minute windspeed average */
  strncpy(umeterstr, (const char *)(datalogdata+46), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "umeterstr: %s\n", umeterstr);
  windspeed_1min_avg = strtol(umeterstr, NULL, base);
  windspeed_1min_avg = (1.0/36.0)*windspeed_1min_avg; /* 0.1 kph to ms-1 */
  syslog(LOG_DEBUG, "windspeed_1min_avg: %f\n", windspeed_1min_avg);
  measval_db( "windsensor", "windspeed_1min_avg", 
	      dataset_date, (float)windspeed_1min_avg, umeterdb);

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
  float windspeed_5min_peak;
  float winddir_peak;
  float temp_out;
  float rain_longterm_total;
  float baro;
  float baro_chg;
  unsigned int baro_corr; // 8 bytes
  float humid_out;
  float rain_today_total;
  float windspeed_5min_avg;

  int day_of_year;
  int min_of_day;

  struct tm *ptm;
  char buf[TBUFF+1];
  time_t dataset_date;
  int tdiff;

  /*
    parse data in packetmode
  */
  syslog(LOG_DEBUG, "packetdata: '%s'", packetdata);
  time(&dataset_date);

  /* open sqlite db file */
  if ( sqlite3_open( (const char *)umeterstation.config.dbfile, &umeterdb) != SQLITE_OK ) {
    return(1);
  }


  /* Wind Speed 5 Minute Peak */
  strncpy(umeterstr, (const char *)(packetdata+5), 5); 
  umeterstr[4] = 0;
  windspeed_5min_peak = (float)strtol(umeterstr, NULL, base);
  windspeed_5min_peak = (1.0/36.0)*windspeed_5min_peak; /* 0.1 kph to ms-1 */
  syslog(LOG_DEBUG, "packet_rd: windspeed_5min_peak: %f\n", windspeed_5min_peak);
  measval_db( "windsensor", "windspeed_5min_peak", 
	      dataset_date, (float)windspeed_5min_peak, umeterdb);

  /* Wind Direction of Wind Speed Peak*/
  strncpy(umeterstr, (const char *)(packetdata+9), 5); 
  umeterstr[4] = 0;
  winddir_peak = (float)strtol(umeterstr, NULL, base);
  winddir_peak = (360.0/255.0)*winddir_peak; /* 0-255 to 0-360 deg */
  measval_db( "windsensor", "winddirection_peak", dataset_date, (float)winddir_peak, umeterdb);
  syslog(LOG_DEBUG, "packet_rd: winddir_peak: %f\n", winddir_peak);

  /* Current Outdoor Temperature */
  strncpy(umeterstr, (const char *)(packetdata+13), 5); 
  umeterstr[4] = 0;
  temp_out = (float)strtol(umeterstr, NULL, base);
  temp_out = ((1.0/10.0)*temp_out -32.0)*5.0/9.0; /* 0.1 degF to degC */
  /* temperature data will not be written to database until it is clear that no TH sensor is installed */
  syslog(LOG_DEBUG, "packet_rd: temp_out: %f\n", temp_out);

  /* Rain Long Term Total */
  strncpy(umeterstr, (const char *)(packetdata+17), 5); 
  umeterstr[4] = 0;
  rain_longterm_total = (float)strtol(umeterstr, NULL, base);
  rain_longterm_total = (25.4/10.0)*rain_longterm_total; /* 0.1in to mm */
  measval_db( "raingauge", "rain_total", dataset_date, (float)rain_longterm_total, umeterdb);
  syslog(LOG_DEBUG, "packet_rd: rain_longterm_total: %f\n", rain_longterm_total);

  /* Current barometer */
  strncpy(umeterstr, (const char *)(packetdata+21), 5); 
  umeterstr[4] = 0;
  baro = (float)strtol(umeterstr, NULL, base);
  baro = baro/10.0; /* 0.1mbar to mbar */
  measval_db( "tp_in_sensor", "pressure", dataset_date, (float)baro, umeterdb);
  syslog(LOG_DEBUG, "packet_rd: baro: %f\n", baro);

  /* Barometer Delta value */
  strncpy(umeterstr, (const char *)(packetdata+25), 5); 
  umeterstr[4] = 0;
  baro_chg = (float)strtol(umeterstr, NULL, base); 
  baro_chg = baro_chg/10.0; /* 0.1mbar to mbar */
  measval_db( "tp_in_sensor", "pressure_delta", dataset_date, (float)baro_chg, umeterdb);
  syslog(LOG_DEBUG, "packet_rd: baro_chg: %f\n", baro_chg);

  /* Barometer Correction factor */
  strncpy(umeterlstr, (const char *)(packetdata+29), 9); 
  umeterlstr[8] = 0;
  baro_corr = strtol(umeterlstr, NULL, base);
  statval_db( "tp_in_sensor", "pressure_corr", 
      dataset_date, (float)baro_corr, umeterdb);
  syslog(LOG_DEBUG, "packet_rd: baro_corr: %d\n", baro_corr);

  /* Current Outdoor Humidity */
  strncpy(umeterstr, (const char *)(packetdata+37), 5); 
  umeterstr[4] = 0;
  err = strncmp( umeterstr, "----", 4);
  /* Temperature Sensor installed */
  if ( err == 0 ) {
    syslog(LOG_INFO, "packet_rd: No Outdoor Humidity/Temperature Sensor found");
    measval_db( "t_out_sensor", "outdoor_temp", 
		dataset_date, (float)temp_out, umeterdb);
  } else /* Outdoor Humidity Temperature sensor installed */ {
    out_th_present = 1;
    humid_out = (float)strtol(umeterstr, NULL, base);
    humid_out = (1.0/10.0)*humid_out; /* 0.1% to %rel.hum. */
    measval_db( "th_out_sensor", "outdoor_temp", 
		dataset_date, (float)temp_out, umeterdb);
    measval_db( "th_out_sensor", "outdoor_hum", 
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
  ptm = localtime(&dataset_date);
  strftime( buf, sizeof(buf), "%j", ptm);
  syslog(LOG_DEBUG,"complete_rd: dataset_date day_of_year :  %s", buf);
  tdiff = day_of_year - atoi(buf);
  if ( tdiff != 0 )
    syslog(LOG_ALERT, 
      "complete_rd: time difference Ultimeter clock  - PC clock: %d days", 
      tdiff);

  /* today's rain total */
  strncpy(umeterstr, (const char *)(packetdata+49), 5); 
  umeterstr[4] = 0;
  rain_today_total = (float)strtol(umeterstr, NULL, base);
  rain_today_total = (25.4/10.0)*rain_today_total; /* 0.1in to mm */
  measval_db( "raingauge", "rain_today", 
	      dataset_date, (float)rain_today_total, umeterdb);
  syslog(LOG_DEBUG, "packet_rd: rain_today_total: %f\n", rain_today_total);

  /* 5 Minute windspeed average */
  strncpy(umeterstr, (const char *)(packetdata+53), 5); 
  umeterstr[4] = 0;
  windspeed_5min_avg = (float)strtol(umeterstr, NULL, base);
  windspeed_5min_avg = (1.0/36.0)*windspeed_5min_avg; /* 0.1 kph to ms-1 */
  measval_db( "windsensor", "windspeed_5min_avg", 
	      dataset_date, (float)windspeed_5min_avg, umeterdb);
  syslog(LOG_DEBUG, "packet_rd: windspeed_avg: %f\n", windspeed_5min_avg);

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

  int base = 16;
  char umeterstr[5];
  char umeterlstr[9];
  int out_th_present = 0;
  int baro_ok = 0;

  /* parameters in complete record mode */
  float datafield;
  float temp_out;
  int statusfield;
  int timeval;

  int day_of_year;
  int min_of_day;
  struct tm *ptm;
  char buf[TBUFF+1];
  int tdiff;


  syslog(LOG_DEBUG, "complete_rd: begin of execution");
  time(&dataset_date);
  /*
    parse data in complete mode
  */
  syslog(LOG_DEBUG,"complete_rd: data: '%s'\n", completedata);

  /* open sqlite db file */
  if ( sqlite3_open( (const char *)umeterstation.config.dbfile, &umeterdb) != SQLITE_OK ) {
    return(1);
  }

  /* 1. Wind Speed */
  strncpy(umeterstr, (const char * )(completedata+4), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "1. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (1.0/36.0)*datafield; /* 0.1 kph to ms-1 */
  syslog(LOG_DEBUG, "1. Wind Speed: %f\n", datafield);
  measval_db( "windsensor", "windspeed", dataset_date, (float)datafield, umeterdb);

  /* 2. Wind direction */
  strncpy(umeterstr, (const char *)(completedata+8), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "2. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (360.0/255.0)*datafield; /* 0-255 to 0-360 deg */
  syslog(LOG_DEBUG, "2. Current Wind Direction: %f\n", datafield);
  measval_db( "windsensor", "winddirection", dataset_date, (float)datafield, umeterdb);

  /* 3. 5 min Windspeed Peak */
  strncpy(umeterstr, (const char *)(completedata+12), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "3. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (1.0/36.0)*datafield; /* kph to ms-1 */
  syslog(LOG_DEBUG, "3. 5 minute Wind Speed Peak: %f\n", datafield);
  measval_db( "windsensor", "windspeed_5min_peak", dataset_date, (float)datafield, umeterdb);

  /* 4. 5min Winddirection Peak */
  strncpy(umeterstr, (const char *)(completedata+16), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "4. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (360.0/255.0)*datafield; /* 0-255 to 0-360 deg */
  syslog(LOG_DEBUG, "4. Wind Direction of 5 Minute Wind Speed Peak: %f\n", datafield);
  measval_db( "windsensor", "winddirection_peak", dataset_date, (float)datafield, umeterdb);

  /* 5. Wind Chill */
  strncpy(umeterstr, (const char *)(completedata+20), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "5. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = ((1.0/10.0)*datafield -32.0)*5.0/9.0; /* 0.1 degF to degC */
  syslog(LOG_DEBUG, "5. Wind Chill: %f\n", datafield);
  syslog(LOG_INFO, "complete_rd: caculation of Wind Chill unclear");
  measval_db( "windsensor", "windchill", dataset_date, (float)datafield, umeterdb);

  /* 6. Outdoor Temperature */
  strncpy(umeterstr, (const char *)(completedata+24), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "6. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = ((1.0/10.0)*datafield -32.0)*5.0/9.0; /* 0.1 degF to degC */
  temp_out = datafield;
  syslog(LOG_DEBUG, "6. Outdoor Temperature: %f\n", temp_out);
  /* this parameter is written to database when it is clear if T-sensor or HT-sensor is installed */

  /* 7. Rain Total for today */
  strncpy(umeterstr, (const char *)(completedata+28), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "7. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (25.4/10.0)*datafield; /* 0.1in to mm */
  syslog(LOG_DEBUG, "7. Rain Total for today: %f\n", datafield);
  measval_db( "raingauge", "rain_today", dataset_date, (float)datafield, umeterdb);

  /* 8. Barometer */
  strncpy(umeterstr, (const char *)(completedata+32), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "8. umeterstr: %s\n", umeterstr);
  err = strncmp( umeterstr, "----", 4);
  /* Barometer Sensor configured */
  if ( err == 0 ) {
    syslog(LOG_INFO, "complete_rd: Barometer not configured: no Barometer data available.");
  } else /* Barometer configured */ {
    baro_ok = 1;
    datafield = strtol(umeterstr, NULL, base);
    datafield = datafield/10.0; /* 0.1mbar to mbar */
    syslog(LOG_DEBUG, "8. Barometer: %f\n", datafield);
    measval_db( "tp_in_sensor", "pressure", 
      dataset_date, (float)datafield, umeterdb);
  }

  /* 9. Barometer 3-hour Pressure Change */
  strncpy(umeterstr, (const char *)(completedata+36), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "9. umeterstr: %s\n", umeterstr);
  if ( baro_ok == 1) {
    datafield = strtol(umeterstr, NULL, base);
    datafield = datafield/10.0; /* 0.1 mbar to mbar */
    syslog(LOG_DEBUG, "9. Barometer 3-Hour Pressure Change: %f\n", datafield);
    measval_db( "tp_in_sensor", "pressure_3hr_chg", 
      dataset_date, (float)datafield, umeterdb);
  } else {
    syslog(LOG_INFO, "complete_rd: Barometer not configured: no Barometer 3-hour Pressure Change data available.");
  }

  /* 10./11. Barometer Correction Factor LSW/MSW */
  strncpy(umeterlstr, (const char *)(completedata+40), 9); 
  umeterlstr[8] = 0;
  syslog(LOG_DEBUG, "10./11. umeterlstr: %s\n", umeterlstr);
  if ( baro_ok == 1) {
    statusfield = strtol(umeterlstr, NULL, base);
    syslog(LOG_DEBUG, "10./11. Barometer Correction factor: %d\n", statusfield);
    statval_db( "tp_in_sensor", "pressure_corr", 
      dataset_date, (float)statusfield, umeterdb);
  } else {
    syslog(LOG_INFO, "complete_rd: Barometer not configured: no Barometer Correction Factor data available.");
  }


  /* 12. Indoor Temperature */
  strncpy(umeterstr, (const char *)(completedata+48), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "12. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = ((1.0/10.0)*datafield -32.0)*5.0/9.0; /* 0.1 degF to degC */
  syslog(LOG_DEBUG, "12. Indoor Temperature: %f\n", datafield);
  measval_db( "tp_in_sensor", "indoor_temp", 
    dataset_date, (float)datafield, umeterdb);

  /* 13. Outdoor humidity */
  strncpy(umeterstr, (const char *)(completedata+52), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "13. umeterstr: %s\n", umeterstr);
  err = strncmp( umeterstr, "----", 4);
  /* Temperature Sensor installed */
  if ( err == 0 ) {
    syslog(LOG_INFO, "complete_rd: No Outdoor Humidity/Temperature Sensor found");
    measval_db( "t_out_sensor", "outdoor_temp", 
		dataset_date, (float)temp_out, umeterdb); 
  } else /* Outdoor Humidity Temperature sensor installed */ {
    out_th_present = 1;
    datafield = (float)strtol(umeterstr, NULL, base);
    datafield = (1.0/10.0)*datafield; /* 0.1% to %rel.hum. */
    measval_db( "th_out_sensor", "outdoor_temp", 
		dataset_date, (float)temp_out, umeterdb);
    measval_db( "th_out_sensor", "outdoor_hum", 
		dataset_date, (float)datafield, umeterdb);
    syslog(LOG_DEBUG, "13. Outdoor Humidity: %f\n", datafield); 
  }

  /* 14. Indoor Humidity */
  strncpy(umeterstr, (const char *)(completedata+56), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "14. umeterstr: %s\n", umeterstr);
  err = strncmp( umeterstr, "----", 4);
  /* Indoor Humidity Sensor installed */
  if ( err == 0 ) {
    syslog(LOG_INFO, "complete_rd: No Indoor HumiditySensor found");
  } else {
    datafield = strtol(umeterstr, NULL, base);
    datafield = (1.0/10.0)*datafield; /* 0.1% to %rel.hum. */
    syslog(LOG_DEBUG, "14. Indoor Humidity: %f\n", datafield);
    measval_db( "h_in_sensor", "indoor_hum", dataset_date, (float)datafield, umeterdb); 
  }

  /* 15. Dew Point */
  strncpy(umeterstr, (const char *)(completedata+60), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "15. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  err = strncmp( umeterstr, "----", 4);
  /* unclear which sensors are used to calculate dew point */
  if ( err == 0 ) {
    syslog(LOG_INFO, "complete_rd: dew point calculation unclear");
  } else {
    datafield = strtol(umeterstr, NULL, base);
    datafield = ((1.0/10.0)*datafield -32.0)*5.0/9.0; /* 0.1 degF to degC */
    syslog(LOG_INFO, "complete_rd: dew point calculation unclear");
    syslog(LOG_DEBUG, "15. Dew Point: %f\n", datafield);
  }

  /* 16. day of year */
  strncpy(umeterstr, (const char *)(completedata+64), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "16. umeterstr: %s\n", umeterstr);
  day_of_year = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "16. day_of_year: %d\n", day_of_year);

  /* 17. minute of day */
  strncpy(umeterstr, (const char *)(completedata+68), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "17. umeterstr: %s\n", umeterstr);
  min_of_day = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "17. min_of_day: %d\n", min_of_day);

  /* Ultimeter clock deviation from PC clock */
  ptm = localtime(&dataset_date);
  strftime( buf, sizeof(buf), "%j", ptm);
  syslog(LOG_DEBUG,"complete_rd: dataset_date day_of_year :  %s", buf);
  tdiff = day_of_year - atoi(buf);
  if ( tdiff != 0 )
    syslog(LOG_ALERT, 
      "complete_rd: time difference Ultimeter clock  - PC clock: %d days", 
      tdiff);

  /* 18. Today's Low Chill Value */
  strncpy(umeterstr, (const char *)(completedata+72), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "18. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = ((1.0/10.0)*datafield -32.0)*5.0/9.0; /* 0.1 degF to degC */
  syslog(LOG_DEBUG, "18. Today's Low Chill Value: %f\n", datafield);

  /* 19. Today's Low Chill Time */
  strncpy(umeterstr, (const char *)(completedata+76), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "19. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "19. Today's Low Chill Time: %d\n", timeval);

  /* 20. Yesterday's Low Chill Value */
  strncpy(umeterstr, (const char *)(completedata+80), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "20. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = ((1.0/10.0)*datafield -32.0)*5.0/9.0; /* 0.1 degF to degC */
  syslog(LOG_DEBUG, "20. Yesterday's Low Chill Value: %f\n", datafield);

  /* 21. Yesterday's Low Chill Time */
  strncpy(umeterstr, (const char *)(completedata+84), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "21. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "21. Yesterday's Low Chill Time: %d\n", timeval);


  /* 22. Longterm Low Chill Date  */
  strncpy(umeterstr, (const char *)(completedata+88), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "22. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "22. Longterm Low Chill date: %d\n", timeval);

  /* 23. Longterm Low Chill Value */
  strncpy(umeterstr, (const char *)(completedata+92), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "23. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = ((1.0/10.0)*datafield -32.0)*5.0/9.0; /* 0.1 degF to degC */
  syslog(LOG_DEBUG, "23. Longterm Low Chill Value: %f\n", datafield);

  /* 24. Longterm Low Chill Time */
  strncpy(umeterstr, (const char *)(completedata+96), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "24. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "24. Longterm Low Chill Time: %d\n", timeval);

  /* 25. Today's Low Outdoor Temperature Value */
  strncpy(umeterstr, (const char *)(completedata+100), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "25. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = ((1.0/10.0)*datafield -32.0)*5.0/9.0; /* 0.1 degF to degC */
  syslog(LOG_DEBUG, "25. Today's Low Outdoor Temperature Value: %f\n", datafield);

  /* 26. Today's Low Outdoor Temperature Time */
  strncpy(umeterstr, (const char *)(completedata+104), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "26. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "26. Today's Low Outdoor Temperature Time: %d\n", timeval);

  /* 27. Yesterday's Low Outdoor Temperature Value */
  strncpy(umeterstr, (const char *)(completedata+108), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "27. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = ((1.0/10.0)*datafield -32.0)*5.0/9.0; /* 0.1 degF to degC */
  syslog(LOG_DEBUG, "27. Yesterday's Low Outdoor Temperature Value: %f\n", datafield);

  /* 28. Yesterday's Low Outdoor Temperature Time */
  strncpy(umeterstr, (const char *)(completedata+112), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "28. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "28. Yesterday's Low Outdoor Temperature Time: %d\n", timeval);

  /* 29. Longterm Low Outdoor Temperature Date  */
  strncpy(umeterstr, (const char *)(completedata+116), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "29. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "29. Longterm Low Outdoor temperature date: %d\n", timeval);

  /* 30. Longterm Low Outdoor Temperature Value */
  strncpy(umeterstr, (const char *)(completedata+120), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "30. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = ((1.0/10.0)*datafield -32.0)*5.0/9.0; /* 0.1 degF to degC */
  syslog(LOG_DEBUG, "30. Longterm Low Outdoor Temperature Value: %f\n", datafield);

  /* 31. Longterm Low Outdoor Temperature Time */
  strncpy(umeterstr, (const char *)(completedata+124), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "31. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "31. Longterm Low Outdoor Temperature Time: %d\n", timeval);

  /* 32. Today's Low Barometer Value */
  strncpy(umeterstr, (const char *)(completedata+128), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "32. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = datafield/10.0; /* 0.1mbar to mbar */
  syslog(LOG_DEBUG, "32. Today's Low Barometer Value: %f\n", datafield);

  /* 33. Today's Low Barometer Time */
  strncpy(umeterstr, (const char *)(completedata+132), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "33. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "33. Today's Low Barometer Time: %d\n", timeval);

  /* 34. Wind Speed */
  strncpy(umeterstr, (const char * )(completedata+136), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "34. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (1.0/36.0)*datafield; /* 0.1 kph to ms-1 */
  syslog(LOG_DEBUG, "34. Wind Speed: %f\n", datafield);

  /* 35. Wind direction */
  strncpy(umeterstr, (const char *)(completedata+140), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "35. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (360.0/255.0)*datafield; /* 0-255 to 0-360 deg */
  syslog(LOG_DEBUG, "35. Current Wind Direction: %f\n", datafield);

  /* 36. Yesterday's Low Barometer Value */
  strncpy(umeterstr, (const char *)(completedata+144), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "36. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = datafield/10.0; /* 0.1mbar to mbar */
  syslog(LOG_DEBUG, "36. Yesterday's Low Barometer Value: %f\n", datafield);

  /* 37. Yesterday's Low Barometer Time */
  strncpy(umeterstr, (const char *)(completedata+148), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "37. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "37. Yesterday's Low Barometer Time: %d\n", timeval);

  /* 38. Longterm Low Barometer Date  */
  strncpy(umeterstr, (const char *)(completedata+152), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "38. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "38. Longterm Low Barometer date: %d\n", timeval);

  /* 39. Longterm Low Barometer Value */
  strncpy(umeterstr, (const char *)(completedata+156), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "39. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = datafield/10.0; /* 0.1mbar to mbar */
  syslog(LOG_DEBUG, "39. Longterm Low Barometer Value: %f\n", datafield);

  /* 40. Longterm Low Barometer Time */
  strncpy(umeterstr, (const char *)(completedata+160), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "40. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "40. Longterm Low Barometer Time: %d\n", timeval);

  /* 41. Today's Low Indoor Temperature Value */
  strncpy(umeterstr, (const char *)(completedata+164), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "41. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = ((1.0/10.0)*datafield -32.0)*5.0/9.0; /* 0.1 degF to degC */
  syslog(LOG_DEBUG, "41. Today's Low Indoor Temperature Value: %f\n", datafield);

  /* 42. Today's Low Indoor Temperature Time */
  strncpy(umeterstr, (const char *)(completedata+168), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "42. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "42. Today's Low Indoor Temperature Time: %d\n", timeval);

  /* 43. Yesterday's Low Indoor Temperature Value */
  strncpy(umeterstr, (const char *)(completedata+172), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "43. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = ((1.0/10.0)*datafield -32.0)*5.0/9.0; /* 0.1 degF to degC */
  syslog(LOG_DEBUG, "43. Yesterday's Low Indoor Temperature Value: %f\n", datafield);

  /* 44. Yesterday's Low Indoor Temperature Time */
  strncpy(umeterstr, (const char *)(completedata+176), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "44. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "44. Yesterday's Low Indoor Temperature Time: %d\n", timeval);

  /* 45. Longterm Low Indoor Temperature Date  */
  strncpy(umeterstr, (const char *)(completedata+180), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "45. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "45. Longterm Low Indoor temperature date: %d\n", timeval);

  /* 46. Longterm Low Indoor Temperature Value */
  strncpy(umeterstr, (const char *)(completedata+184), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "46. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = ((1.0/10.0)*datafield -32.0)*5.0/9.0; /* 0.1 degF to degC */
  syslog(LOG_DEBUG, "46. Longterm Low Indoor Temperature Value: %f\n", datafield);

  /* 47. Longterm Low Indoor Temperature Time */
  strncpy(umeterstr, (const char *)(completedata+188), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "47. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "47. Longterm Low Indoor Temperature Time: %d\n", timeval);

  /* 48. Today's Low Outdoor Humidity Value*/
  strncpy(umeterstr, (const char *)(completedata+192), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "48. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (1.0/10.0)*datafield; /* 0.1% to %rel.hum. */
  syslog(LOG_DEBUG, "48. Today's Low Outdoor Humidity Value: %f\n", datafield);

  /* 49. Today's Low Outdoor Humidity Time */
  strncpy(umeterstr, (const char *)(completedata+196), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "49. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "49. Today's Low Outdoor Humidity Time: %d\n", timeval);

  /* 50. Yesterday's Low Outdoor Humidity Value*/
  strncpy(umeterstr, (const char *)(completedata+200), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "50. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (1.0/10.0)*datafield; /* 0.1% to %rel.hum. */
  syslog(LOG_DEBUG, "50. Yesterday's Low Outdoor Humidity Value: %f\n", datafield);

  /* 51. Yesterday's Low Outdoor Humidity Time */
  strncpy(umeterstr, (const char *)(completedata+204), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "51. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "51. Yesterday's Low Outdoor Humidity Time: %d\n", timeval);

  /* 52. Longterm Low Outdoor Humidity Date  */
  strncpy(umeterstr, (const char *)(completedata+208), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "52. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "52. Longterm Low Outdoor Humidity date: %d\n", timeval);

  /* 53. Longterm Low Outdoor Humidity Value*/
  strncpy(umeterstr, (const char *)(completedata+212), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "53. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (1.0/10.0)*datafield; /* 0.1% to %rel.hum. */
  syslog(LOG_DEBUG, "53. Longterm Low Outdoor Humidity Value: %f\n", datafield);

  /* 54. Longterm Low Outdoor Humidity Time */
  strncpy(umeterstr, (const char *)(completedata+216), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "54. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "54. Longterm Low Outdoor Humidity Time: %d\n", timeval);

  /* 55. Today's Low Indoor Humidity Value*/
  strncpy(umeterstr, (const char *)(completedata+220), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "55. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (1.0/10.0)*datafield; /* 0.1% to %rel.hum. */
  syslog(LOG_DEBUG, "55. Today's Low Indoor Humidity Value: %f\n", datafield);

  /* 56. Today's Low Indoor Humidity Time */
  strncpy(umeterstr, (const char *)(completedata+224), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "56. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "56. Today's Low Indoor Humidity Time: %d\n", timeval);

  /* 57. Yesterday's Low Indoor Humidity Value*/
  strncpy(umeterstr, (const char *)(completedata+228), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "57. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (1.0/10.0)*datafield; /* 0.1% to %rel.hum. */
  syslog(LOG_DEBUG, "57. Yesterday's Low Indoor Humidity Value: %f\n", datafield);

  /* 58. Yesterday's Low Indoor Humidity Time */
  strncpy(umeterstr, (const char *)(completedata+232), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "58. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "58. Yesterday's Low Indoor Humidity Time: %d\n", timeval);

  /* 59. Longterm Low Indoor Humidity Date  */
  strncpy(umeterstr, (const char *)(completedata+236), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "59. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "59. Longterm Low Indoor Humidity date: %d\n", timeval);

  /* 60. Longterm Low Indoor Humidity Value*/
  strncpy(umeterstr, (const char *)(completedata+240), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "60. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (1.0/10.0)*datafield; /* 0.1% to %rel.hum. */
  syslog(LOG_DEBUG, "60. Longterm Low Indoor Humidity Value: %f\n", datafield);

  /* 61. Longterm Low Indoor Humidity Time */
  strncpy(umeterstr, (const char *)(completedata+244), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "61. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "61. Longterm Low Indoor Humidity Time: %d\n", timeval);

  /* 62. Today's Wind Speed Value*/
  strncpy(umeterstr, (const char * )(completedata+248), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "62. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (1.0/36.0)*datafield; /* 0.1 kph to ms-1 */
  syslog(LOG_DEBUG, "62. Today's Wind Speed Value: %f\n", datafield);

  /* 63. Today's Wind Speed Time */
  strncpy(umeterstr, (const char *)(completedata+252), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "63. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "63. Today's Wind Speed Time: %d\n", timeval);

  /* 64. Yesterday's Wind Speed Value*/
  strncpy(umeterstr, (const char * )(completedata+256), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "64. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (1.0/36.0)*datafield; /* 0.1 kph to ms-1 */
  syslog(LOG_DEBUG, "64. Yesterday's Wind Speed Value: %f\n", datafield);

  /* 65. Yesterday's Wind Speed Time */
  strncpy(umeterstr, (const char *)(completedata+260), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "65. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "65. Yesterday's Wind Speed Time: %d\n", timeval);

  /* 66. Longterm Wind Speed Date  */
  strncpy(umeterstr, (const char *)(completedata+264), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "66. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "66. Longterm Wind Speed date: %d\n", timeval);

  /* 67. Longterm Wind Speed Value*/
  strncpy(umeterstr, (const char * )(completedata+268), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "67. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (1.0/36.0)*datafield; /* 0.1 kph to ms-1 */
  syslog(LOG_DEBUG, "67. Longterm Wind Speed Value: %f\n", datafield);

  /* 68. Longterm Wind Speed Time */
  strncpy(umeterstr, (const char *)(completedata+272), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "68. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "68. Longterm Wind Speed Time: %d\n", timeval);

  /* 69. Today's High Outdoor Temperature Value */
  strncpy(umeterstr, (const char *)(completedata+276), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "69. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = ((1.0/10.0)*datafield -32.0)*5.0/9.0; /* 0.1 degF to degC */
  syslog(LOG_DEBUG, "69. Today's High Outdoor Temperature Value: %f\n", datafield);

  /* 70. Today's High Outdoor Temperature Time */
  strncpy(umeterstr, (const char *)(completedata+280), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "70. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "70. Today's High Outdoor Temperature Time: %d\n", timeval);

  /* 71. Wind Speed */
  strncpy(umeterstr, (const char * )(completedata+284), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "71. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (1.0/36.0)*datafield; /* 0.1 kph to ms-1 */
  syslog(LOG_DEBUG, "71. Wind Speed: %f\n", datafield);

  /* 72. Wind direction */
  strncpy(umeterstr, (const char *)(completedata+288), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "72. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (360.0/255.0)*datafield; /* 0-255 to 0-360 deg */
  syslog(LOG_DEBUG, "72. Current Wind Direction: %f\n", datafield);

  /* 73. Yesterday's High Outdoor Temperature Value */
  strncpy(umeterstr, (const char *)(completedata+292), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "73. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = ((1.0/10.0)*datafield -32.0)*5.0/9.0; /* 0.1 degF to degC */
  syslog(LOG_DEBUG, "73. Yesterday's High Outdoor Temperature Value: %f\n", datafield);

  /* 74. Yesterday's High Outdoor Temperature Time */
  strncpy(umeterstr, (const char *)(completedata+296), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "74. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "74. Yesterday's High Outdoor Temperature Time: %d\n", timeval);

  /* 75. Longterm High Outdoor Temperature Date  */
  strncpy(umeterstr, (const char *)(completedata+300), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "75. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "75. Longterm High Outdoor temperature date: %d\n", timeval);

  /* 76. Longterm High Outdoor Temperature Value */
  strncpy(umeterstr, (const char *)(completedata+304), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "76. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = ((1.0/10.0)*datafield -32.0)*5.0/9.0; /* 0.1 degF to degC */
  syslog(LOG_DEBUG, "76. Longterm High Outdoor Temperature Value: %f\n", datafield);

  /* 77. Longterm High Outdoor Temperature Time */
  strncpy(umeterstr, (const char *)(completedata+308), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "77. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "77. Longterm High Outdoor Temperature Time: %d\n", timeval);

  /* 78. Today's High Barometer Value */
  strncpy(umeterstr, (const char *)(completedata+312), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "78. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = datafield/10.0; /* 0.1mbar to mbar */
  syslog(LOG_DEBUG, "78. Today's High Barometer Value: %f\n", datafield);

  /* 79. Today's High Barometer Time */
  strncpy(umeterstr, (const char *)(completedata+316), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "79. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "79. Today's High Barometer Time: %d\n", timeval);

  /* 80. Yesterday's High Barometer Value */
  strncpy(umeterstr, (const char *)(completedata+320), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "80. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = datafield/10.0; /* 0.1mbar to mbar */
  syslog(LOG_DEBUG, "80. Yesterday's High Barometer Value: %f\n", datafield);

  /* 81. Yesterday's High Barometer Time */
  strncpy(umeterstr, (const char *)(completedata+324), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "81. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "81. Yesterday's High Barometer Time: %d\n", timeval);

  /* 82. Longterm High Barometer Date  */
  strncpy(umeterstr, (const char *)(completedata+328), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "82. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "82. Longterm High Barometer date: %d\n", timeval);

  /* 83. Longterm High Barometer Value */
  strncpy(umeterstr, (const char *)(completedata+332), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "83. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = datafield/10.0; /* 0.1mbar to mbar */
  syslog(LOG_DEBUG, "83. Longterm High Barometer Value: %f\n", datafield);

  /* 84. Longterm High Barometer Time */
  strncpy(umeterstr, (const char *)(completedata+336), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "84. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "84. Longterm High Barometer Time: %d\n", timeval);

  /* 85. Today's High Indoor Temperature Value */
  strncpy(umeterstr, (const char *)(completedata+340), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "85. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = ((1.0/10.0)*datafield -32.0)*5.0/9.0; /* 0.1 degF to degC */
  syslog(LOG_DEBUG, "85. Today's High Indoor Temperature Value: %f\n", datafield);

  /* 86. Today's High Indoor Temperature Time */
  strncpy(umeterstr, (const char *)(completedata+344), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "86. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "86. Today's High Indoor Temperature Time: %d\n", timeval);

  /* 87. Yesterday's High Indoor Temperature Value */
  strncpy(umeterstr, (const char *)(completedata+348), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "87. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = ((1.0/10.0)*datafield -32.0)*5.0/9.0; /* 0.1 degF to degC */
  syslog(LOG_DEBUG, "87. Yesterday's High Indoor Temperature Value: %f\n", datafield);

  /* 88. Yesterday's High Indoor Temperature Time */
  strncpy(umeterstr, (const char *)(completedata+352), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "88. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "88. Yesterday's High Indoor Temperature Time: %d\n", timeval);

  /* 89. Longterm High Indoor Temperature Date  */
  strncpy(umeterstr, (const char *)(completedata+356), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "89. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "89. Longterm High Indoor temperature date: %d\n", timeval);

  /* 90. Longterm High Indoor Temperature Value */
  strncpy(umeterstr, (const char *)(completedata+360), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "90. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = ((1.0/10.0)*datafield -32.0)*5.0/9.0; /* 0.1 degF to degC */
  syslog(LOG_DEBUG, "90. Longterm High Indoor Temperature Value: %f\n", datafield);

  /* 91. Longterm High Indoor Temperature Time */
  strncpy(umeterstr, (const char *)(completedata+364), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "91. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "91. Longterm High Indoor Temperature Time: %d\n", timeval);

  /* 92. Today's High Outdoor Humidity Value*/
  strncpy(umeterstr, (const char *)(completedata+368), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "92. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (1.0/10.0)*datafield; /* 0.1% to %rel.hum. */
  syslog(LOG_DEBUG, "92. Today's High Outdoor Humidity Value: %f\n", datafield);

  /* 93. Today's High Outdoor Humidity Time */
  strncpy(umeterstr, (const char *)(completedata+372), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "93. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "93. Today's High Outdoor Humidity Time: %d\n", timeval);

  /* 94. Yesterday's High Outdoor Humidity Value*/
  strncpy(umeterstr, (const char *)(completedata+376), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "94. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (1.0/10.0)*datafield; /* 0.1% to %rel.hum. */
  syslog(LOG_DEBUG, "94. Yesterday's High Outdoor Humidity Value: %f\n", datafield);

  /* 95. Yesterday's High Outdoor Humidity Time */
  strncpy(umeterstr, (const char *)(completedata+380), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "95. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "95. Yesterday's High Outdoor Humidity Time: %d\n", timeval);

  /* 96. Longterm High Outdoor Humidity Date  */
  strncpy(umeterstr, (const char *)(completedata+384), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "96. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "96. Longterm High Outdoor Humidity date: %d\n", timeval);

  /* 97. Longterm High Outdoor Humidity Value*/
  strncpy(umeterstr, (const char *)(completedata+388), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "97. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (1.0/10.0)*datafield; /* 0.1% to %rel.hum. */
  syslog(LOG_DEBUG, "97. Longterm High Outdoor Humidity Value: %f\n", datafield);

  /* 98. Longterm High Outdoor Humidity Time */
  strncpy(umeterstr, (const char *)(completedata+392), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "98. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "98. Longterm High Outdoor Humidity Time: %d\n", timeval);

  /* 99. Today's High Indoor Humidity Value*/
  strncpy(umeterstr, (const char *)(completedata+396), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "99. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (1.0/10.0)*datafield; /* 0.1% to %rel.hum. */
  syslog(LOG_DEBUG, "99. Today's High Indoor Humidity Value: %f\n", datafield);

  /* 100. Today's High Indoor Humidity Time */
  strncpy(umeterstr, (const char *)(completedata+400), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "100. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "100. Today's High Indoor Humidity Time: %d\n", timeval);

  /* 101. Yesterday's High Indoor Humidity Value*/
  strncpy(umeterstr, (const char *)(completedata+404), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "101. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (1.0/10.0)*datafield; /* 0.1% to %rel.hum. */
  syslog(LOG_DEBUG, "101. Yesterday's High Indoor Humidity Value: %f\n", datafield);

  /* 102. Yesterday's High Indoor Humidity Time */
  strncpy(umeterstr, (const char *)(completedata+408), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "102. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "102. Yesterday's High Indoor Humidity Time: %d\n", timeval);

  /* 103. Longterm High Indoor Humidity Date  */
  strncpy(umeterstr, (const char *)(completedata+412), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "103. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "103. Longterm High Indoor Humidity date: %d\n", timeval);

  /* 104. Longterm High Indoor Humidity Value*/
  strncpy(umeterstr, (const char *)(completedata+416), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "104. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (1.0/10.0)*datafield; /* 0.1% to %rel.hum. */
  syslog(LOG_DEBUG, "104. Longterm High Indoor Humidity Value: %f\n", datafield);

  /* 105. Longterm High Indoor Humidity Time */
  strncpy(umeterstr, (const char *)(completedata+420), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "105. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "105. Longterm High Indoor Humidity Time: %d\n", timeval);

 /* 106. Yesterday's Rain Total */
  strncpy(umeterstr, (const char *)(completedata+424), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "106. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (25.4/10.0)*datafield; /* 0.1in to mm */
  syslog(LOG_DEBUG, "106. Yesterday's Rain Total: %f\n", datafield);

  /* 107. Longterm Rain Date  */
  strncpy(umeterstr, (const char *)(completedata+428), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "107. umeterstr: %s\n", umeterstr);
  timeval = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "107. Longterm Rain Date: %d\n", timeval);

  /* 108. Longterm Rain Total */
  strncpy(umeterstr, (const char *)(completedata+432), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "108. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (25.4/10.0)*datafield; /* 0.1in to mm */
  syslog(LOG_DEBUG, "108. Longterm Rain Total: %f\n", datafield);
  measval_db( "raingauge", "rain_total", dataset_date, (float)datafield, umeterdb);

  /* 109. Leap Year */
  strncpy(umeterstr, (const char *)(completedata+436), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "109. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "109. Leap Year: %f\n", datafield);

  /* 110. WDCF Value */
  strncpy(umeterstr, (const char *)(completedata+440), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "110. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "110. WDCF Value: %f\n", datafield);

  /* 111. Today's High Wind Direction */
  strncpy(umeterstr, (const char *)(completedata+444), 3); 
  umeterstr[2] = 0;
  syslog(LOG_DEBUG, "111. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (360.0/255.0)*datafield; /* 0-255 to 0-360 deg */
  syslog(LOG_DEBUG, "111. Today's High Wind Direction: %f\n", datafield);

  /* 112. Yesterday's High Wind Direction */
  strncpy(umeterstr, (const char *)(completedata+446), 3); 
  umeterstr[2] = 0;
  syslog(LOG_DEBUG, "112. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  datafield = (360.0/255.0)*datafield; /* 0-255 to 0-360 deg */
  syslog(LOG_DEBUG, "112. Yesterday's High Wind Direction: %f\n", datafield);

  /* 113. Spare */
  strncpy(umeterstr, (const char *)(completedata+448), 3); 
  umeterstr[2] = 0;
  syslog(LOG_DEBUG, "113. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "113. Spare: %f\n", datafield);

  /* 114. Longterm High Wind Direction */
  strncpy(umeterstr, (const char *)(completedata+450), 3); 
  umeterstr[2] = 0;
  syslog(LOG_DEBUG, "114. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "114. Longterm High Wind Direction: %f\n", datafield);

  /* 115. 1 Minute Windspeed Average */
  strncpy(umeterstr, (const char *)(completedata+452), 5); 
  umeterstr[4] = 0;
  syslog(LOG_DEBUG, "115. umeterstr: %s\n", umeterstr);
  datafield = strtol(umeterstr, NULL, base);
  syslog(LOG_DEBUG, "115. 1 Minute Windspeed Average: %f\n", datafield);

  sqlite3_close( umeterdb);

  return(0);
}

/*
  umeter_rd

  reading data records from serial port
  data acquisition to database

*/
int 
umeter_rd( ) {
  int pfd;
  struct termios tio;

  int err = -1;
  int ndat;
  unsigned char data[SBUFF+1];

  const char dataloghd[3]  = "!!";
  const char packethd[6]   = "$ULTW";
  const char complethd[5] = "&CR&";
  int dataloglen  = 51; /* NL is ignored */
  /*  thus record size is 48 hex bytes + 2 header bytes + carriage return */
  int packetlen   = 58; /* NL is ignored */
  /* thus record size is 52 hex bytes + 5 header bytes + carriage */
  int completlen  = 457;  /* NL is ignored */
  /* thus record size is 452 hex bytes + 4 header bytes + carriage */

  memset(&tio,0,sizeof(tio));
  tio.c_iflag=IGNPAR|ICRNL;
  tio.c_oflag=0;
  tio.c_cflag=CS8|CREAD|CLOCAL;
  tio.c_lflag=ICANON;

  tio.c_cc[VINTR]    = 0;     /* Ctrl-c */ 
  tio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
  tio.c_cc[VERASE]   = 0;     /* del */
  tio.c_cc[VKILL]    = 0;     /* @ */
  tio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
  tio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
  tio.c_cc[VSTART]   = 0;     /* Ctrl-q */ 
  tio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
  tio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
  tio.c_cc[VEOL]     = 0;     /* '\0' */
  tio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
  tio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
  tio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
  tio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
  tio.c_cc[VEOL2]    = 0;     /* '\0' */

  pfd = open(umeterstation.config.device, O_RDWR | O_NOCTTY);
  cfsetospeed(&tio,B2400);
  cfsetispeed(&tio,B2400);
  tcflush(pfd, TCIFLUSH);
  tcsetattr(pfd,TCSANOW,&tio);

  umeterstation.status.is_present = 1;

  for (;;) {
    tcflush(pfd, TCIFLUSH);
    memset(&data,0,sizeof(data));
    if (( ndat = read(pfd,&data,SBUFF)) > 0) { 
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
        syslog(LOG_DEBUG, "umeter_rd: data (garbage): '%s' : %d\n", data, ndat);
      }
    } else {
      syslog(LOG_WARNING,"umeter_rd: read ndat < 0");
      sleep(1);
    }
  }
  close(pfd);
  return(err); 
}

