/*
  pgsql.c

  read and write wth data to postgresql database

  $Id$
  $Revision$

  Copyright (C) 2011-2012 Volker Jahns <volker@thalreit.de>

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
  pg_datadb - insert measured values for use in 
    - 1-Wire
    - WS2000 
    - PCWSR 
  database

*/
int
pg_datadb( long dataset_date, int sensor_meas_no, float meas_value,
  PGconn *pg_conn )
{
  PGresult *res;
  int err;
  int querylen = MAXQUERYLEN;
  char query[MAXQUERYLEN];

  err = 0;
  /* insert data values */
  snprintf(query, querylen, 
    "INSERT INTO sensordata VALUES ( DEFAULT, to_timestamp(%lu), %d, %f)",
    dataset_date, sensor_meas_no, meas_value); 
  syslog( LOG_DEBUG, "datadb: INSERT query: %s", query);
  res = PQexec( pg_conn, query);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    syslog(LOG_DEBUG,
      "pg_datadb: error: insert sensor data: %s", PQerrorMessage(pg_conn));
      PQclear(res);
      err = 1;
  }
  PQclear(res);

  /* insert date when last data has been updated */
  snprintf(query, querylen, 
    "UPDATE sensorupdate SET last_update = to_timestamp(%lu) WHERE sensor_meas_no = %d",
    dataset_date, sensor_meas_no); 
  res = PQexec( pg_conn, query);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    syslog(LOG_DEBUG,
      "pg_datadb: error: update sensor data: %s", PQerrorMessage(pg_conn));
      PQclear(res);
      err = 1;  
  }
  PQclear(res);
  syslog( LOG_DEBUG, "datadb: UPDATE query: %s", query);
  return(err);
}


/*
  pg_stat_ws2000db - insert status values of WS2000 weatherstation

*/
int
pg_stat_ws2000db( int sensor_status[], time_t statusset_date, PGconn *pg_conn) 
{
  PGresult *res;
  int i, err;
  int querylen = MAXQUERYLEN;
  char query[MAXQUERYLEN];

  err = 0;
  for ( i = 1; i <= 18; i++) {
    snprintf(query, querylen, 
     "INSERT INTO sensorstatus (statusset_no, statusset_date, sensor_no, sensor_status) VALUES ( NULL, %lu, %d, %d)",
     (long unsigned int) statusset_date, i, sensor_status[i]); 
    res = PQexec( pg_conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
      syslog(LOG_ALERT,
        "pg_statdb: error: insert sensor status: %s", PQerrorMessage(pg_conn));
      PQclear(res);
      err = 1;  
    }
    PQclear(res);
  }
  PQclear(res);
  return(err);
}


/*
  pg_new_ws2000db - insert new flag values of WS2000 weatherstation

  new flag is read which each datarecord read command ( 1 2)
  status value is read with status command ( 5)

  there is a small time offset as both commands occur at different time
  new flag is added to table sensorstatus with the same time value as
  the sensorstatus value has been added with the status command before
  using SQL update

*/
int
pg_new_ws2000db( long statusset_date, int sensor_no, int new_flag, PGconn *pg_conn) 
{
  PGresult *res;
  int err = 0;
  int querylen = MAXQUERYLEN;
  char query[MAXQUERYLEN];

  snprintf(query, querylen, 
	   "UPDATE sensorstatus SET new_flag = %d WHERE sensor_no = %d AND statusset_date =  ( select MAX(statusset_date) from sensorstatus where sensor_no = %d )",
	   new_flag, sensor_no, sensor_no); 
  syslog(LOG_DEBUG, "newdb: query: %s", query);
  res = PQexec( pg_conn, query);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    syslog(LOG_ALERT,
	   "newdb: error: insert sensor status: %s", PQerrorMessage(pg_conn));
    PQclear(res);
    err = 1;
  }
  PQclear(res);
  syslog( LOG_DEBUG, "datadb: INSERT query: %s", query);

  return(err);
}


/*
  pg_get_onewireinfo
  retrieve
    sensor_meas_no,
    sensor_name,
    param_name,
    param_gain,
    param_offset, 
    sensor number, 
    devicetyp and 
    parameter name of
  1-wire device
  returns these data in structure ssdp
*/
/*
int
pg_get_onewireinfo( char *parname, char *serialnum, sensdevpar_t *ssdp, PGconn *pg_conn)
{
  PGresult *res;
  int i;
  char query[SBUFF+1];

  snprintf(query, SBUFF, 
    "SELECT sp.sensor_meas_no, sn.sensor_name, "
    "pn.param_name, pn.param_offset, pn.param_gain, "
    "dt.devicetyp, dt.familycode, dt.serialnum "
    "FROM sensorparameters AS sp, sensornames AS sn, parameternames AS pn, devicetyp AS dt "
    "WHERE sp.param_no = pn.param_no "
    "AND sp.sensor_no = sn.sensor_no "
    "AND sp.device_no = dt.device_no "
    "AND dt.serialnum = '%s' " 
    "AND pn.param_name = '%s' ", 
    serialnum, parname);
  syslog(LOG_DEBUG, "pg_get_onewireinfo: sql: %s", query);

  res = PQexec( pg_conn, query); 
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    syslog( LOG_ALERT,
	    "pg_get_onewireinfo: error: select sensor parameter info: %s\n", 
	     PQerrorMessage(pg_conn));
    PQclear(res);
    return(1);
  }

  for ( i = 0; i < PQntuples(res); i++) {
    ssdp->sensor_meas_no = (int )atoi(PQgetvalue(res, i, 0));
    strncpy(ssdp->sensorname, PQgetvalue(res, i, 1), TBUFF);  
    strncpy(ssdp->par_name, PQgetvalue(res, i, 2), TBUFF);
    ssdp->offset = (float )atof(PQgetvalue(res, i, 3));
    ssdp->gain   = (float )atof(PQgetvalue(res, i, 4));
    strncpy(ssdp->devicetyp, PQgetvalue(res, i, 5), TBUFF);  
    strncpy(ssdp->familycode, PQgetvalue(res, i, 6), TBUFF);  
    strncpy(ssdp->serialnum, PQgetvalue(res, i, 7), TBUFF);  
  }
  PQclear(res);
  if ( PQntuples(res) == 0) {
    syslog( LOG_DEBUG, "pg_get_onewireinfo: no configuration data in database");
    return(1);
  }
  return(0);
}
*/


/*
  pg_writedb

  writing ws2000 data to sqlite database

 */
int
pg_writedb( int sensor_no, int nval, int sensor_meas_no[], time_t dataset_date, 
         float meas_value[], PGconn *pg_conn ) {
  int i, err;
  
  err = 0;        
  for ( i = 0; i < nval; i++) {
    err = pg_datadb( dataset_date, sensor_meas_no[i], meas_value[i], pg_conn);
  }

  return(err);
}

/*
  pg_is_ws2000sens
  
  check if status value of sensor is not equal to zero 
  only works fpr WS2000 weatherstation 
  
*/
int
pg_is_ws2000sens( int sensor_no, PGconn *pg_conn)
{
  int err;
  time_t stattim;
  int statval;
  char query[SBUFF+1];
  sqlite3_stmt *qcomp;

  snprintf(query, SBUFF, 
    "SELECT sensor_status, max(statusset_date) FROM sensorstatus WHERE sensor_no = %d",
    sensor_no);
  err = sqlite3_prepare( ws2000db, query, -1, &qcomp, 0); 
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT, "Error: issens: select ws2000 status data: err: %d", err);
    return(err);
  }

  while( SQLITE_ROW == sqlite3_step(qcomp)) {
    statval = (int )sqlite3_column_int( qcomp, 0); 
    stattim = (time_t )sqlite3_column_int( qcomp, 1); 
  }

  err = sqlite3_finalize(qcomp);
  if ( err != SQLITE_OK )  {
    syslog( LOG_ALERT,  "Error: issens: sqilte3_finalize");
    return(err);
  } 

  return(statval); 
}

/*
int
pg_maxsensmeas( PGconn *pg_conn) {
  PGresult *res;
  int max_sens_meas;
  int i,rowcnt;
  char query[MAXQUERYLEN];

  snprintf(query, SBUFF, "SELECT COUNT(*) FROM sensorparameters");
  syslog(LOG_DEBUG, "pg_maxsensmeas: sql: %s", query);

  res = PQexec( pg_conn, query);
  if (PQresultStatus(res) != PGRES_TUPLES_OK) {
    syslog(LOG_DEBUG, "pg_maxsensmeas: error: %s", PQerrorMessage(pg_conn));
      PQclear(res);
      return(-1);
  }

  for ( i = 0; i < PQntuples(res); i++) {
    max_sens_meas = (int )atoi(PQgetvalue(res, i, 0));
    rowcnt++;
    syslog(LOG_DEBUG, "maxsensmeas  : max_sens_meas: %d", max_sens_meas);
  }
  PQclear(res);
  if ( rowcnt == 0) {
    syslog( LOG_ALERT, "Error: cant find relation sensor - device - parameter");
    return(-1);
  }
  return(max_sens_meas);
}
*/

/*

  pg_measval_db

  ULTIMETER and WMR928 NX measurement data to postgresql database

 */
int
pg_measval_db( char *sensorname, char *parametername, 
                   time_t dataset_date, float mval, PGconn *pg_conn) {
  PGresult *res;
  int i;
  int sensor_no, parameter_no, sensor_meas_no;
  char query[SBUFF+1];

  syslog(LOG_DEBUG, "pg_measval_db: sensorname: %s parametername: %s", 
                                sensorname, parametername);

  /* select sensor_no */
  snprintf(query, SBUFF, 
    "SELECT sensor_no FROM sensornames WHERE sensor_name = '%s'",
    sensorname);

  syslog(LOG_DEBUG, "pg_measval_sb: query :\'%s\'", query);
  res = PQexec( pg_conn, query);
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    syslog( LOG_ALERT,
            "pg_measval_db: error: select:  %s\n",
             PQerrorMessage(pg_conn));
    PQclear(res);
    return(1);
  }

  for ( i = 0; i < PQntuples(res); i++) {
    syslog(LOG_DEBUG, "pg_measval_db : sensorname: \'%s\' sensor_no: %d", 
              sensorname, atoi(PQgetvalue(res, i, 0)));
    sensor_no   = atoi(PQgetvalue( res, i, 0)); 
  }
  if ( PQntuples(res) == 0 )
    syslog(LOG_ALERT," no sensorname %s\n in database", sensorname);

  PQclear(res);
  /* select parameter_no of parametername */
  snprintf(query, SBUFF, 
    "SELECT param_no FROM parameternames WHERE param_name = '%s'",
    parametername);

  syslog(LOG_DEBUG, "measval_sb: query :\'%s\'", query);
  res = PQexec( pg_conn, query);
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    syslog( LOG_ALERT,
            "pg_measval_db: error: select:  %s\n",
             PQerrorMessage(pg_conn));
    PQclear(res);
    return(1);
  }

  for ( i = 0; i < PQntuples(res); i++) {
    syslog(LOG_DEBUG, "measval_db : parametername: \'%s\' param_no: %d", 
              parametername, atoi(PQgetvalue( res, i, 0)));
    parameter_no   = atoi(PQgetvalue( res, i , 0));
  }
  if ( PQntuples( res) == 0 )
    syslog(LOG_ALERT," no parametername %s\n in database", parametername);

  PQclear(res);

  /* select sens_meas_no of windsensor and wind_direction */
  snprintf(query, SBUFF, 
    "SELECT sensor_meas_no FROM sensorparameters WHERE sensor_no = %d AND param_no = %d",
    sensor_no, parameter_no);
  res = PQexec( pg_conn, query);
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    syslog( LOG_ALERT,
            "pg_measval_db: error: select:  %s\n",
             PQerrorMessage(pg_conn));
    PQclear(res);
    return(1);
  }

  for ( i = 0; i < PQntuples(res); i++) {
    syslog(LOG_DEBUG, "measval_db : sensor_meas_no: %d", 
      atoi(PQgetvalue(res, i, 0)));
    sensor_meas_no   = atoi(PQgetvalue( res, i, 0));
  }
  if ( PQntuples(res) == 0 )
    syslog(LOG_ALERT," no sensor_meas_no in database");

  PQclear(res);

  /* insert data values */
  snprintf(query, SBUFF, 
    "INSERT INTO sensordata VALUES ( NULL, %lu, %d, %f)",
    (long unsigned int)dataset_date, sensor_meas_no, (double)mval); 
  res = PQexec( pg_conn, query);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    syslog(LOG_ALERT, "pg_measval_db: error insert sensor data: %s",
      PQerrorMessage(pg_conn));
  }
  PQclear(res);

  /* insert date when last data has been updated */
  snprintf(query, SBUFF, 
    "UPDATE sensorupdate SET last_update = %lu WHERE sensor_meas_no = %d",
    (long unsigned int)dataset_date, sensor_meas_no); 
  res = PQexec( pg_conn, query);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    syslog(LOG_ALERT, "pg_measval_db: error update sensor data: %s",
      PQerrorMessage(pg_conn));
  }
  PQclear(res);
  return(0);
}

int
pg_statval_db( char *sensorname, char *flagname,
               time_t statusset_date, long unsigned int sval, PGconn *pg_conn) 
{
  PGresult *res;
  int i;
  int sensor_no, flag_no, sensor_flag_no;
  char query[SBUFF+1];

  /* select sensor_no */
  snprintf(query, SBUFF, 
    "SELECT sensor_no FROM sensornames WHERE sensor_name = '%s'",
    sensorname);
  res = PQexec( pg_conn, query);
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    syslog( LOG_ALERT,
            "pg_measval_db: error: select:  %s\n",
             PQerrorMessage(pg_conn));
    PQclear(res);
    return(1);
  }

  for ( i = 0; i < PQntuples(res); i++) {
    syslog(LOG_DEBUG, "pg_statval_db : sensorname: \'%s\' sensor_no: %d",
              sensorname, atoi(PQgetvalue(res, i, 0)));
    sensor_no   = atoi(PQgetvalue( res, i, 0));
  }
  if ( PQntuples(res) == 0 ) {
    syslog(LOG_ALERT," no sensorname %s\n in database", sensorname);
    return(1);
  }

  /* select flag_no of flagnames */
  snprintf(query, SBUFF, 
    "SELECT flag_no FROM flagnames WHERE flag_name = '%s'",
    flagname);
  res = PQexec( pg_conn, query);
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    syslog( LOG_ALERT,
            "pg_flagval_db: error: select:  %s\n",
             PQerrorMessage(pg_conn));
    PQclear(res);
    return(1);
  }
  for ( i = 0; i < PQntuples(res); i++) {
    syslog(LOG_DEBUG, "pg_statval_db : flagname: \'%s\' flag_no: %d",
              flagname, atoi(PQgetvalue(res, i, 0)));
    flag_no   = atoi(PQgetvalue( res, i, 0));
  }
  if ( PQntuples(res) == 0 ) {
    syslog(LOG_ALERT," no flagname %s\n in database", flagname);
    PQclear(res);
    return(1);
  }
  PQclear(res);

  /* select sens_flag_no of sensorname and flagname */
  snprintf(query, SBUFF, 
    "SELECT sensor_flag_no FROM sensorflags WHERE sensor_no = %d AND flag_no = %d",
    sensor_no, flag_no);
  
  res = PQexec( pg_conn, query);
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    syslog( LOG_ALERT,
            "pg_statval_db: error: select sensor_flag_no:  %s\n",
             PQerrorMessage(pg_conn));
    PQclear(res);
    return(1);
  }
  for ( i = 0; i < PQntuples(res); i++) {
    syslog(LOG_DEBUG, "statval_db : sensor_flag_no: %d", 
      atoi(PQgetvalue( res, i, 0)));
    sensor_flag_no   = atoi(PQgetvalue( res, i, 0)); 
  }
  PQclear(res);

  /* insert status values */
  snprintf(query, SBUFF, 
    "INSERT INTO statusdata VALUES ( NULL, %lu, %d, %lu)",
    (long unsigned int)statusset_date, sensor_flag_no, (long unsigned int)sval); 

  res = PQexec( pg_conn, query);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    syslog(LOG_ALERT, "pg_statval_db: error insert status data: %s",
      PQerrorMessage(pg_conn));
  }
  PQclear(res);
  return(0);
}

