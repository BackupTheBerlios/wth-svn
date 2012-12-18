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



sensdevpar_t
pgsql_get_sensorparams( char *sensorname, char*parametername,
                         int stationtype)
{
  int          i;
  int          rowcnt             = 0;
  char         query[SBUFF+1]     = "\0";
  char         serialnum[TBUFF+1] = "\0";
  double        offset;
  double        gain;
  sensdevpar_t sqsenspar;
  PGconn       *wth_pgconn;
  PGresult     *res;
   
  switch(stationtype) {
    case ONEWIRE:
      syslog(LOG_DEBUG, "pgsql_get_sensorparams: stationtype is "
        "ONEWIRE\n");
      wth_pgconn = onewire_pgconn;
      strncpy( serialnum, sensorname, strlen( sensorname));
      snprintf(query, SBUFF,
        "SELECT sp.sensor_meas_no, sn.sensor_name, "
        "pn.param_name, pn.param_offset, pn.param_gain, "
        "dt.devicetyp, dt.familycode, dt.serialnum "
        "FROM sensorparameters AS sp, sensornames AS sn, "
        "parameternames AS pn, devicetyp AS dt "
        "WHERE sp.param_no = pn.param_no "
        "AND sp.sensor_no = sn.sensor_no "
        "AND sp.device_no = dt.device_no "
        "AND dt.serialnum = '%s' "
        "AND pn.param_name = '%s' ",
        serialnum, parametername);
      break;
    case WMR9X8:
      syslog(LOG_DEBUG, "pgsql_get_sensorparams: stationtype is "
        "WMR9X8\n");
      wth_pgconn= wmr9x8_pgconn;
      snprintf(query, SBUFF,
        "SELECT sp.sensor_meas_no, sn.sensor_name, "
        "pn.param_name, pn.param_offset, pn.param_gain "
        "FROM sensorparameters AS sp, sensornames AS sn, "
        "parameternames AS pn "
        "WHERE sp.param_no = pn.param_no "
        "AND sp.sensor_no = sn.sensor_no "
        "AND sn.sensor_name = '%s' "
        "AND pn.param_name  = '%s' ",
        sensorname, parametername);
      break;
    case UMETER:
      syslog(LOG_DEBUG, "pgsql_get_sensorparams: stationtype is "
        "UMETER\n");
      wth_pgconn= umeter_pgconn;
      snprintf(query, SBUFF,
        "SELECT sp.sensor_meas_no, sn.sensor_name, "
        "pn.param_name, pn.param_offset, pn.param_gain "
        "FROM sensorparameters AS sp, sensornames AS sn, "
        "parameternames AS pn "
        "WHERE sp.param_no = pn.param_no "
        "AND sp.sensor_no = sn.sensor_no "
        "AND sn.sensor_name = '%s' "
        "AND pn.param_name  = '%s' ",
        sensorname, parametername);
      break;
    case WS2000:
      syslog(LOG_DEBUG, "pgsql_get_sensorparams: stationtype is "
        "WS2000\n");
      wth_pgconn= ws2000_pgconn;
      snprintf(query, SBUFF,
        "SELECT sp.sensor_meas_no, sn.sensor_name, "
        "pn.param_name, pn.param_offset, pn.param_gain "
        "FROM sensorparameters AS sp, sensornames AS sn, "
        "parameternames AS pn "
        "WHERE sp.param_no = pn.param_no "
        "AND sp.sensor_no = sn.sensor_no "
        "AND sn.sensor_name = '%s' "
        "AND pn.param_name  = '%s' ",
        sensorname, parametername);
      break;
    default:
      syslog(LOG_ALERT, "pgsql_get_sensorparams: error: unknown stationtype\n");
      sqsenspar.sensor_meas_no = -1;
      return(sqsenspar);
  }

  //syslog(LOG_DEBUG, "pgsql_get_sensorparams: query %s\n", query);
  if (PQstatus(wth_pgconn) != CONNECTION_OK) {
    syslog( LOG_ALERT,"pgsql_get_sensorparams: Connection to database failed: %s",
            PQerrorMessage(wth_pgconn));
    PQfinish(wth_pgconn);
    switch(stationtype) {
      case ONEWIRE:
        syslog(LOG_DEBUG, "pgsql_get_sensorparams: stationtype is "
          "ONEWIRE\n");
        onewire_pgconn = PQconnectdb(onewirestation.config.dbconn);
        wth_pgconn = onewire_pgconn;
        break;
      case WMR9X8:
        syslog(LOG_DEBUG, "pgsql_get_sensorparams: stationtype is "
          "WMR9X8\n");
        wmr9x8_pgconn= PQconnectdb(wmr9x8station.config.dbconn);
        wth_pgconn= wmr9x8_pgconn;
        break;
      case UMETER:
        syslog(LOG_DEBUG, "pgsql_get_sensorparams: stationtype is "
          "UMETER\n");
        umeter_pgconn= PQconnectdb(umeterstation.config.dbconn);
        wth_pgconn= umeter_pgconn;
        break;
     case WS2000:
        syslog(LOG_DEBUG, "pgsql_get_sensorparams: stationtype is "
          "WS2000\n");
        ws2000_pgconn= PQconnectdb(ws2000station.config.dbconn);
        wth_pgconn= ws2000_pgconn;
        break;
     default:
       syslog(LOG_ALERT, "pgsql_get_sensorparams: error: unknown stationtype\n");
       sqsenspar.sensor_meas_no = -1;
       return(sqsenspar);
    }
    if (PQstatus(wth_pgconn) != CONNECTION_OK) {
      syslog( LOG_ALERT,"pgsql_get_sensorparams: Reconnection to database failed: %s",
              PQerrorMessage(wth_pgconn));
      sqsenspar.sensor_meas_no = -1;
      return(sqsenspar);
    } else {
      syslog(LOG_INFO, "pgsql_getsensorparams: Reconnect to database success");
    }
  }

  res = PQexec( wth_pgconn, query);
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    syslog( LOG_ALERT,
            "pgsql_get_sensorparams: error: select:  %s\n",
             PQerrorMessage(wth_pgconn));
    PQclear(res);
    sqsenspar.sensor_meas_no = -1;
    return(sqsenspar);
  }

  rowcnt =  PQntuples(res);
  for ( i = 0; i < PQntuples(res); i++) {
    sqsenspar.sensor_meas_no = atoi(PQgetvalue( res, i, 0));
    strncpy(sqsenspar.sensorname, 
      PQgetvalue( res, i, 1), TBUFF);
    strncpy(sqsenspar.par_name, 
      PQgetvalue( res, i, 2), TBUFF);
    offset = atof((PQgetvalue( res, i, 3)));
    sqsenspar.offset = offset;
    gain   = atof((PQgetvalue( res, i, 4)));
    sqsenspar.gain   = gain;
    if ( stationtype == ONEWIRE ) {				  
      strncpy(sqsenspar.devicetyp, 
        PQgetvalue( res, i, 5), TBUFF);
      strncpy(sqsenspar.familycode, 
        PQgetvalue( res, i, 6), TBUFF);
      strncpy(sqsenspar.serialnum, 
        PQgetvalue( res, i, 7), TBUFF);
    } else {
      strncpy(sqsenspar.devicetyp, 
        "n.a.", TBUFF);
      strncpy(sqsenspar.familycode, 
        "n.a.", TBUFF);
      strncpy(sqsenspar.serialnum, 
        "n.a.", TBUFF);
    }
  }

  syslog(LOG_DEBUG, "pgsql_get_sensorparams: sensor_meas_no : %d\n", 
                    sqsenspar.sensor_meas_no);
  if ( rowcnt == 0) {
    syslog( LOG_DEBUG, "pgsql_get_sensorparams: "
                       "no configuration data in database");
    sqsenspar.sensor_meas_no = -1;
    return(sqsenspar);
  }

  PQclear(res);
  return(sqsenspar);
}


sensflag_t
pgsql_get_sensorflags( char *sensorname, char *flagname,
                         int stationtype)
{
  int          i;
  int          rowcnt = 0;
  char         query[SBUFF+1] = "\0";
  sensflag_t   sqsensflag;
  PGconn       *wth_pgconn;
  PGresult     *res;
   
  switch(stationtype) {
    case ONEWIRE:
      syslog(LOG_DEBUG, "pgsql_get_sensorflags: no flags available"
                        " for stationtype ONEWIRE\n");
      sqsensflag.sensor_flag_no = -1;
      return(sqsensflag);
      break;
    case WMR9X8:
      syslog(LOG_DEBUG, "pgsql_get_sensorflags: stationtype is "
                        "WMR9X8\n");
      wth_pgconn = wmr9x8_pgconn;
      snprintf(query, SBUFF,
        "SELECT sf.sensor_flag_no, sn.sensor_no, fn.flag_no, sn.sensor_name, "
        "fn.flag_name "
        "FROM sensorflags AS sf, sensornames AS sn, "
        "flagnames AS fn "
        "WHERE sf.flag_no = fn.flag_no "
        "AND sf.sensor_no = sn.sensor_no "
        "AND sn.sensor_name = '%s' "
        "AND fn.flag_name  = '%s' ",
        sensorname, flagname);
      syslog(LOG_DEBUG, "pgsql_get_sensorflags: query: %s\n",
                        query);
      break;
    case UMETER:
      syslog(LOG_DEBUG, "pgsql_get_sensorflags: stationtype is "
                        "UMETER\n");
      wth_pgconn = umeter_pgconn;
      snprintf(query, SBUFF,
        "SELECT sf.sensor_flag_no, sn.sensor_no, fn.flag_no, sn.sensor_name, "
        "fn.flag_name "
        "FROM sensorflags AS sf, sensornames AS sn, "
        "flagnames AS fn "
        "WHERE sf.flag_no = fn.flag_no "
        "AND sf.sensor_no = sn.sensor_no "
        "AND sn.sensor_name = '%s' "
        "AND fn.flag_name  = '%s' ",
        sensorname, flagname);
      syslog(LOG_DEBUG, "pgsql_get_sensorflags: query: %s\n",
                        query);
      break;
    default:
      syslog(LOG_ALERT, "pgsql_get_sensorflags: error: "
                        "unknown stationtype\n");
      sqsensflag.sensor_flag_no = -1;
      return(sqsensflag);
  }
  if (PQstatus(wth_pgconn) != CONNECTION_OK) {
    syslog( LOG_ALERT,"Connection to database failed: %s",
            PQerrorMessage(wth_pgconn));
  }


  res = PQexec( wth_pgconn, query);
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    syslog( LOG_ALERT,
            "pgsql_get_sensorflags: error: select:  %s\n",
             PQerrorMessage(wth_pgconn));
    PQclear(res);
    sqsensflag.sensor_flag_no = -1;
    return(sqsensflag);
  }

  rowcnt =  PQntuples(res);
  for ( i = 0; i < PQntuples(res); i++) {
    sqsensflag.sensor_flag_no = atoi(PQgetvalue( res, i, 0));
    sqsensflag.sensor_no      = atoi(PQgetvalue( res, i, 1));
    sqsensflag.flag_no        = atoi(PQgetvalue( res, i, 1));
    strncpy(sqsensflag.sensorname, 
      PQgetvalue( res, i, 3), TBUFF);
    strncpy(sqsensflag.flagname, 
      PQgetvalue( res, i, 4), TBUFF);
    strncpy(sqsensflag.devicetyp, 
      "n.a.", TBUFF);
    strncpy(sqsensflag.familycode, 
      "n.a.", TBUFF);
    strncpy(sqsensflag.serialnum, 
      "n.a.", TBUFF);
  }
  syslog(LOG_DEBUG, "pgsql_get_sensorflags: sensor_flag_no : %d\n", 
                    sqsensflag.sensor_flag_no);

  if ( rowcnt == 0) {
    syslog( LOG_DEBUG, "pgsql_get_sensorflags: "
                       "no configuration data in database");
    sqsensflag.sensor_flag_no = -1;
    return(sqsensflag);
  }

  PQclear(res);
  return(sqsensflag);
}

/*
  pgsql_datadbn 

  insert measured data values for use in 
    - 1-Wire
    - WS2000 
    - PCWSR 
  database

*/
int
pgsql_datadbn( long dataset_date, int sensor_meas_no, float meas_value,
  int stationtype )
{
  int          err = 0;
  int          querylen = MAXQUERYLEN;
  char         query[MAXQUERYLEN];
  PGconn       *wth_pgconn;
  PGresult     *res;

  switch(stationtype) {
    case ONEWIRE:
      syslog(LOG_DEBUG, "pgsql_datadbn: stationtype is "
        "ONEWIRE\n");
      wth_pgconn = onewire_pgconn;
      break;
    case UMETER:
      syslog(LOG_DEBUG, "pgsql_datadbn: stationtype is "
        "UMETER\n");
      wth_pgconn = umeter_pgconn;
      break;
    case WMR9X8:
      syslog(LOG_DEBUG, "pgsql_datadbn: stationtype is "
        "WMR9X8\n");
      wth_pgconn = wmr9x8_pgconn;
      break;
    case WS2000:
      syslog(LOG_DEBUG, "pgsql_datadbn: stationtype is "
        "WS2000\n");
      wth_pgconn = ws2000_pgconn;
      break;
    default:
      syslog(LOG_ALERT, "pgsql_datadbn: error: unknown stationtype\n");
      return(1);
  }  

  /* insert data values */
  syslog(LOG_DEBUG, "pgsql_datadbn: dataset_date: %ld, "
                    "sensor_meas_no: %d, meas_val: %f", 
                    dataset_date, sensor_meas_no, meas_value);
  snprintf(query, querylen, 
    "INSERT INTO sensordata VALUES ( nextval('sensordata_dataset_no_seq'), to_timestamp(%lu), %d, %f)",
    dataset_date, sensor_meas_no, meas_value); 
  syslog(LOG_DEBUG, "pgsql_datadbn: query: %s", query);
  if (PQstatus(wth_pgconn) != CONNECTION_OK) {
    syslog( LOG_ALERT,"Connection to database failed: %s",
            PQerrorMessage(wth_pgconn));
  }


  res = PQexec( wth_pgconn, query);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    syslog(LOG_ALERT,
      "pgsql_datadbn: error: insert sensor status: %s",
      PQerrorMessage(wth_pgconn));
    PQclear(res);
    err = 1;  
  }
  PQclear(res);

  /* insert date when last data has been updated */
  snprintf(query, querylen, 
    "UPDATE sensorupdate SET last_update = to_timestamp(%lu) WHERE sensor_meas_no = %d",
    dataset_date, sensor_meas_no); 
  syslog(LOG_DEBUG, "pgsql_datadbn: query: %s", query);
  if (PQstatus(wth_pgconn) != CONNECTION_OK) {
    syslog( LOG_ALERT,"Connection to database failed: %s",
            PQerrorMessage(wth_pgconn));
  }

  res = PQexec( wth_pgconn, query);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    syslog(LOG_ALERT,
      "pgsql_datadbn: error: update sensorupdate: %s",
      PQerrorMessage(wth_pgconn));
    PQclear(res);
    err = 1;  
  }
  PQclear(res);

  return(err);
}

/*
  sqlite_statdbn 

  insert measured status values for use in 
    - 1-Wire
    - WS2000 
    - PCWSR 
  database

*/
int
pgsql_statdbn( long statusset_date, int sensor_flag_no, 
                long unsigned int stat_value, int stationtype )
{
  int          err;
  int          querylen = MAXQUERYLEN;
  char         query[MAXQUERYLEN];
  PGconn       *wth_pgconn;
  PGresult     *res;

  switch(stationtype) {
    case ONEWIRE:
      syslog(LOG_DEBUG, "pgsql_statdbn: stationtype is "
        "ONEWIRE\n");
      wth_pgconn = onewire_pgconn;
      break;
    case UMETER:
      syslog(LOG_DEBUG, "pgsql_statdbn: stationtype is "
        "UMETER\n");
      wth_pgconn = umeter_pgconn;
      break;
    case WMR9X8:
      syslog(LOG_DEBUG, "pgsql_statdbn: stationtype is "
        "WMR9X8\n");
      wth_pgconn = wmr9x8_pgconn;
      break;
    default:
      syslog(LOG_ALERT, "pgsql_statdbn: error: unknown stationtype\n");
      return(1);
  }  


  /* insert data values */
  syslog(LOG_DEBUG, "pgsql_statdbn: dataset_date: %ld, "
                    "sensor_flag_no: %d, stat_val: %lu", 
                    statusset_date, sensor_flag_no, stat_value);
  snprintf(query, querylen, 
    "INSERT INTO statusdata VALUES ( nextval('statusdata_statusset_no'), to_timestamp(%lu), %d, %lu)",
    statusset_date, sensor_flag_no, stat_value); 
  syslog(LOG_DEBUG, "pgsql_statdbn: query: %s", query);

  if (PQstatus(wth_pgconn) != CONNECTION_OK) {
    syslog( LOG_ALERT,"Connection to database failed: %s",
            PQerrorMessage(wth_pgconn));
  }

  res = PQexec( wth_pgconn, query);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    syslog(LOG_ALERT,
      "pgsql_statdbn: error: insert sensor status: %s",
      PQerrorMessage(wth_pgconn));
    PQclear(res);
    err = 1;  
  }
  PQclear(res);

  return(err);
}

/*
  WS2000 legacy

  pgsql_stat_ws2000db - insert status values of WS2000 weatherstation

*/
int
pgsql_stat_ws2000db( int sensor_status[], time_t statusset_date) 
{
  PGresult *res;
  int i, err;
  int querylen = MAXQUERYLEN;
  char query[MAXQUERYLEN];

  err = 0;
  for ( i = 1; i <= 18; i++) {
    snprintf(query, querylen, 
     "INSERT INTO sensorstatus (statusset_no, statusset_date, sensor_no, sensor_status) VALUES ( nextval('sensorstatus_statusset_no_seq'), to_timestamp(%lu), %d, %d)",
     (long unsigned int) statusset_date, i, sensor_status[i]); 

    if (PQstatus(ws2000_pgconn) != CONNECTION_OK) {
      syslog( LOG_ALERT,"Connection to database failed: %s",
              PQerrorMessage(ws2000_pgconn));
    }
    res = PQexec( ws2000_pgconn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
      syslog(LOG_ALERT,
        "pgsql_stat_ws2000db: error: insert sensor status: %s",
        PQerrorMessage(ws2000_pgconn));
      PQclear(res);
      err = 1;  
    }
    PQclear(res);
  }
  PQclear(res);
  return(err);
}


/*
  legacy WS2000

  pgsql_new_ws2000db - insert new flag values of WS2000 weatherstation

  new flag is read which each datarecord read command ( 1 2)
  status value is read with status command ( 5)

  there is a small time offset as both commands occur at different time
  new flag is added to table sensorstatus with the same time value as
  the sensorstatus value has been added with the status command before
  using SQL update

*/
int
pgsql_new_ws2000db( int sensor_no, int new_flag) 
{
  PGresult *res;
  int err = 0;
  int querylen = MAXQUERYLEN;
  char query[MAXQUERYLEN];

  snprintf(query, querylen, 
	   "UPDATE sensorstatus SET new_flag = %d WHERE sensor_no = %d AND statusset_date =  ( select MAX(statusset_date) from sensorstatus where sensor_no = %d )",
	   new_flag, sensor_no, sensor_no); 
  syslog(LOG_DEBUG, "pgsql_new_ws2000db: query: %s", query);
  if (PQstatus(ws2000_pgconn) != CONNECTION_OK) {
    syslog( LOG_ALERT,"Connection to database failed: %s",
            PQerrorMessage(ws2000_pgconn));
  }
  res = PQexec( ws2000_pgconn, query);
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    syslog(LOG_ALERT,
      "pgsql_new_ws2000db: error: insert sensor status: %s",
      PQerrorMessage(ws2000_pgconn));
    PQclear(res);
    err = 1;
  }
  PQclear(res);
  syslog( LOG_DEBUG, "pgsql_new_ws2000db: INSERT query: %s", query);

  return(err);
}



/*
  legacy WS2000

  pgsql_is_ws2000sens
  
  check if status value of sensor is not equal to zero 
  only works fpr WS2000 weatherstation 
  
*/
int
pgsql_is_ws2000sens( int sensor_no)
{
  int i;
  PGresult *res;
  time_t   stattim;
  int      statval = 0;
  char     query[SBUFF+1];

  snprintf(query, SBUFF, 
    "SELECT sensor_status, EXTRACT (EPOCH FROM statusset_date) FROM sensorstatus WHERE sensor_no = %d",
    sensor_no);

  syslog(LOG_DEBUG, "pgsql_is_ws2000sens: query %s\n", query);
  if (PQstatus(ws2000_pgconn) != CONNECTION_OK) {
    syslog( LOG_ALERT,"Connection to database failed: %s",
            PQerrorMessage(ws2000_pgconn));
  }
  res = PQexec( ws2000_pgconn, query);
  if (PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    syslog( LOG_ALERT,
            "pgsql_is_ws2000sens: error: select:  %s\n",
             PQerrorMessage(ws2000_pgconn));
    PQclear(res);
    statval = 0;
    return(statval);
  }

  for ( i = 0; i < PQntuples(res); i++) {
    statval = atoi(PQgetvalue( res, i, 0));
    stattim = (time_t) atol(PQgetvalue( res, i, 1));
    syslog(LOG_DEBUG, "pgsql_is_ws2000sens: statval: %d stattim: %lu",
            statval, stattim);
  }

  if ( PQntuples(res) == 0 ) {
    syslog(LOG_ALERT,"pgsql_is_ws2000sens: no statusdatae in database");
    statval = 0;
  }
  PQclear(res);

  return(statval); 
}



/*
  PCWSR legacy

  pgsql_datadb - insert measured values for use in 
    - PCWSR 
  database

*/
int
pgsql_datadb( long dataset_date, int sensor_meas_no, float meas_value,
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
  if (PQstatus(pg_conn) != CONNECTION_OK) {
    syslog( LOG_ALERT,"Connection to database failed: %s",
            PQerrorMessage(pg_conn));
  }
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
  if (PQstatus(pg_conn) != CONNECTION_OK) {
    syslog( LOG_ALERT,"Connection to database failed: %s",
            PQerrorMessage(pg_conn));
  }
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

