/*
  wthdb.c

  read and write wth database data 

  $Id$
  $Revision$

  Copyright (C) Volker Jahns <volker@thalreit.de>

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
#include <sys/resource.h>

/*
  datadb - insert measured values

*/
int
datadb( long dataset_date, int sensor_meas_no, float meas_value,
  sqlite3 *pcwsrdb )
{
  int err;
  int querylen = MAXQUERYLEN;
  char query[MAXQUERYLEN];
  char *errmsg;

  /* insert data values */
  snprintf(query, querylen, 
    "INSERT INTO sensordata VALUES ( NULL, %lu, %d, %f)",
    dataset_date, sensor_meas_no, meas_value); 
  err = sqlite3_exec( pcwsrdb, query, NULL, NULL, &errmsg);
  if ( err) { 
    syslog(LOG_DEBUG,
      "datadb: error: insert sensor data: err: %d : sqlite_errmsg: %s\n", 
      err, sqlite3_errmsg(pcwsrdb));
  }
  if ( errmsg != NULL) {
    sqlite3_free( errmsg);
    errmsg = NULL;
  }

  /* insert date when last data has been updated */
  snprintf(query, querylen, 
    "UPDATE sensorupdate SET last_update = %lu WHERE sensor_meas_no = %d",
    dataset_date, sensor_meas_no); 
  err = sqlite3_exec( pcwsrdb, query, NULL, NULL, &errmsg);
  if ( err) { 
    syslog(LOG_DEBUG, 
      "datadb: error: update sensor date: err: %d : sqlite_errmsg: %s\n", 
      err, sqlite3_errmsg(pcwsrdb));
  }
  syslog( LOG_DEBUG, "datadb: query: %s", query);
  if ( errmsg != NULL) {
    sqlite3_free( errmsg);
    errmsg = NULL;
  }

  return(err);
}


/*
  statdb - insert status values

*/
int
statdb( int sensor_status[], time_t statusset_date) 
{
  int i, err;
  int querylen = MAXQUERYLEN;
  char query[MAXQUERYLEN];

  char *errmsg;

  char tstrg[TBUFF+1];
  char rrdfile[TBUFF+1];
  char tmpstr[TBUFF];
  char template[TBUFF];
  char **ustrg;

  //time_t pstatime;
  //struct tm *pstatm;
  struct rusage pstat;
    
  //time(&pstatime); pstatm = gmtime(&pstatime);
  err = getrusage( RUSAGE_SELF, &pstat);
  syslog(LOG_DEBUG, "statdb_i: memory check: "
         "maxrss: %ld : ixrss: %ld idrss: %ld isrss : %ld\n",
         pstat.ru_maxrss,
         pstat.ru_ixrss, pstat.ru_idrss,pstat.ru_isrss);
  err = 0;

  ustrg = malloc(sizeof(char)*TBUFF+1);
  ustrg[2] = malloc(sizeof(char)*NBUFF+1);
  snprintf(tstrg,MAXMSGLEN, "%lu", (long int) statusset_date);

  /* open sqlite db file */
  if ( ( err = sqlite3_open( ws2000station.config.dbfile, &ws2000db))) {
    syslog(LOG_ALERT, "statdb: Failed to open database %s. Error: %s\n", 
      ws2000station.config.dbfile, sqlite3_errmsg(ws2000db));
    return(err);
  }

  for ( i = 1; i <= 18; i++) {
    snprintf(query, querylen, 
     "INSERT INTO sensorstatus VALUES ( NULL, %lu, %d, %d)",
     (long unsigned int) statusset_date, i, sensor_status[i]); 
    err = sqlite3_exec( ws2000db, query, NULL, NULL, &errmsg);
    if ( err) { 
      syslog(LOG_DEBUG,
        "statdb: error: insert sensor status: err: %d : sqlite_errmsg: %s", 
        err, sqlite3_errmsg(ws2000db));
    }
    if ( errmsg != NULL) {
      sqlite3_free( errmsg);
      errmsg = NULL;
    }

    snprintf(template,MAXMSGLEN,"%d", sensor_status[i]);
    strncat(tstrg, ":", 1);
    strncat(tstrg, template, strlen(template));
  }

  /* cleanup and close */
  sqlite3_close( ws2000db);

  snprintf( rrdfile, TBUFF, "%s", ws2000station.config.rrdpath);
  snprintf( tmpstr, MAXMSGLEN, "%s", ws2000station.config.monitor);
  strncat( rrdfile, tmpstr, 2*MAXMSGLEN+1);
  syslog(LOG_DEBUG, "statdb: rrdfile: %s", rrdfile);
  syslog(LOG_DEBUG, "statdb: rrd update string: %s", tstrg);
  
  snprintf(ustrg[2], MAXMSGLEN-2, "%s", tstrg);
  rrd_clear_error();
  rrd_get_context();
  rrd_update_r( rrdfile, NULL, 1, (const char **)(ustrg + 2));
  if ( ( err = rrd_test_error())) {
    syslog( LOG_ALERT, "statdb: RRD return code: %d\n",
            rrd_test_error());
  }
  free(ustrg[2]);
  free(ustrg);

  err = getrusage( RUSAGE_SELF, &pstat);
  syslog(LOG_DEBUG, "statdb_f: memory check: "
         "maxrss: %ld : ixrss: %ld idrss: %ld isrss : %ld\n",
         pstat.ru_maxrss,
         pstat.ru_ixrss, pstat.ru_idrss,pstat.ru_isrss);

  return(err);
}


/*
  newdb - insert new flag values

*/
int
newdb( long statusset_date, int sensor_no, int new_flag) 
{
  int err;
  int querylen = MAXQUERYLEN;
  char query[MAXQUERYLEN];
  char *errmsg;

  err = sqlite3_open( ws2000station.config.dbfile, &ws2000db);
  syslog(LOG_DEBUG, 
    "newdb: sqlite3_open %s return value: %d : sqlite_errmsg: %s\n", 
    ws2000station.config.dbfile,
    err, sqlite3_errmsg(ws2000db));
  if ( err) {
    syslog( LOG_ALERT, "newdb: failed to open database %s. error: %s\n", 
    ws2000station.config.dbfile, sqlite3_errmsg(ws2000db));
    free( errmsg);
    return (-1);
  } else {
    syslog(LOG_DEBUG, "newdb: sqlite3_open: OK\n");
  }

  snprintf(query, querylen, 
	   "INSERT INTO sensornewflag VALUES ( NULL, %lu, %d, %d)",
	   statusset_date, sensor_no, new_flag); 
  err = sqlite3_exec( ws2000db, query, NULL, NULL, NULL);
  if ( err) { 
    syslog(LOG_ALERT,
	   "newdb: error: insert sensor status: err: %d : sqlite_errmsg: %s\n", 
	   err, sqlite3_errmsg(ws2000db));
    free(errmsg);
  }
  /* cleanup and close */
  sqlite3_close( ws2000db);
  syslog(LOG_DEBUG,"newdb: sqlite3_close ws2000db done\n");

  return(0);
}


/*
  senspardb - retrieve sensor number and parameter name
*/
int
senspardb( int sensor_meas_no, senspar_t *sspar, sqlite3 *wthdb)
{
  int err, rowcnt;
  char query[SBUFF+1];
  sqlite3_stmt *qcomp;

  snprintf(query, SBUFF, 
    "SELECT sp.sensor_no, sn.sensorname, pn.parameter_name "
    "FROM sensorparameters AS sp, sensornames AS sn, parameternames "
    "AS pn WHERE sp.parameter_no = pn.parameter_no "
    "AND sp.sensor_no = sn.sensor_no AND sp.sensor_meas_no = %d", 
    sensor_meas_no);

  err = sqlite3_prepare( wthdb, query, -1, &qcomp, 0); 
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
	    "Error: select parametername: err: %d : sqlite_errmsg: %s\n", 
	    err, sqlite3_errmsg(wthdb));
    return(1);
  }

  rowcnt = 0;
  while( SQLITE_ROW == sqlite3_step(qcomp)) {
    syslog(LOG_DEBUG, "senspardb  : sensor_meas_no: %d : sensor_no: %d",
	   sensor_meas_no, sqlite3_column_int(qcomp, 0));
    syslog(LOG_DEBUG, "senspardb  : sensorname: %s : parametername: %s",
	   (char *)sqlite3_column_text(qcomp, 1), (char *)sqlite3_column_text(qcomp, 2));
    sspar->sensor_no   = sqlite3_column_int(qcomp,0); 
    sspar->sensor_name = malloc(sizeof(char)*TBUFF+1); 
    sspar->par_name    = malloc(sizeof(char)*TBUFF+1);
    strncpy(sspar->sensor_name, (char *)sqlite3_column_text(qcomp,1), TBUFF);  
    strncpy(sspar->par_name, (char *)sqlite3_column_text(qcomp,2), TBUFF);  
    rowcnt++;
  }
  err = sqlite3_finalize(qcomp);
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
	    "Error: select parametername: err: %d : sqlite_errmsg: %s\n", 
	    err, sqlite3_errmsg(wthdb));
    return(1);
  }
  if ( rowcnt == 0) {
    syslog( LOG_ALERT, "Error: no configuration data in database");
    return(1);
  }
  return(0);
}


/*
  sensdevpar - retrieve sensor number , devicetyp and parameter name
*/
int
sensdevpar( char *parname, char *serialnum, sensdevpar_t *ssdp, sqlite3 *wthdb)
{
  int err, rowcnt;
  char query[SBUFF+1];
  sqlite3_stmt *qcomp;

  snprintf(query, SBUFF, 
    "SELECT sdp.sensor_meas_no, sn.sensor_name, pn.parameter_name, dt.devicetyp, dt.familycode, dt.serialnum "
    "FROM sensordevparameters AS sdp, sensorname AS sn, parametername AS pn, devicetyp AS dt "
    "WHERE sdp.parameter_no = pn.parameter_no "
    "AND sdp.sensor_no = sn.sensor_no "
    "AND sdp.device_no = dt.device_no "
    "AND dt.serialnum = '%s' " 
    "AND pn.parameter_name = '%s' ", 
    serialnum, parname);
  //syslog(LOG_DEBUG, "sensdevpar: sql: %s", query);

  err = sqlite3_prepare( wthdb, query, -1, &qcomp, 0); 
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
	    "Error: select sensdevpar: err: %d : sqlite_errmsg: %s\n", 
	    err, sqlite3_errmsg(wthdb));
    return(1);
  }

  rowcnt = 0;
  while( SQLITE_ROW == sqlite3_step(qcomp)) {
    /*
    syslog(LOG_DEBUG, "sensdevpar  : sensorname: %s : parametername: %s : sensor_meas_no: %d",
	   (char *)sqlite3_column_text(qcomp, 1), 
           (char *)sqlite3_column_text(qcomp, 2),
           sqlite3_column_int(qcomp, 0));
    syslog(LOG_DEBUG, "sensdevpar  : devicetyp: %s : familycode: %s : serialnum: %s",
	   (char *)sqlite3_column_text(qcomp, 3), 
           (char *)sqlite3_column_text(qcomp, 4), 
           (char *)sqlite3_column_text(qcomp, 5));
    */
    /*
    ssdp->sensorname     = malloc(sizeof(char)*TBUFF+1); 
    ssdp->par_name       = malloc(sizeof(char)*TBUFF+1);
    ssdp->devicetyp      = malloc(sizeof(char)*TBUFF+1);
    ssdp->familycode     = malloc(sizeof(char)*TBUFF+1);
    ssdp->serialnum      = malloc(sizeof(char)*TBUFF+1);
    */
    ssdp->sensor_meas_no = sqlite3_column_int(qcomp, 0);
    strncpy(ssdp->sensorname, (char *)sqlite3_column_text(qcomp,1), TBUFF);  
    strncpy(ssdp->par_name, (char *)sqlite3_column_text(qcomp,2), TBUFF);  
    strncpy(ssdp->devicetyp, (char *)sqlite3_column_text(qcomp,3), TBUFF);  
    strncpy(ssdp->familycode, (char *)sqlite3_column_text(qcomp,4), TBUFF);  
    strncpy(ssdp->serialnum, (char *)sqlite3_column_text(qcomp,5), TBUFF);  
    rowcnt++;
  }
  err = sqlite3_finalize(qcomp);
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
	    "Error: select parametername: err: %d : sqlite_errmsg: %s\n", 
	    err, sqlite3_errmsg(wthdb));
    return(1);
  }
  if ( rowcnt == 0) {
    //syslog( LOG_ALERT, "Error: no configuration data in database");
    return(1);
  }
  return(0);
}

int
writedb( int sensor_no, int nval, int sensor_meas_no[], time_t dataset_date, 
         float meas_value[] ) {
  int i, err;
  char tstrg[TBUFF+1];
  char rrdfile[TBUFF+1];
  char tmpstr[TBUFF+1];
  char template[TBUFF+1];
  char **ustrg;
  senspar_t spar;
  char *errmsg;

  err = 0;        
  ustrg = malloc(sizeof(char)*TBUFF);
  ustrg[2] = malloc(sizeof(char)*NBUFF);

  snprintf(tstrg,MAXMSGLEN, "%lu", (long int)dataset_date);

  err = sqlite3_open( ws2000station.config.dbfile, &ws2000db);
  syslog(LOG_DEBUG, 
    "writedb: sqlite3_open %s return value: %d : sqlite_errmsg: %s\n", 
    ws2000station.config.dbfile,
    err, sqlite3_errmsg(ws2000db));
  if ( err) {
    syslog( LOG_ALERT, "writedb: failed to open database %s. error: %s\n", 
    ws2000station.config.dbfile, sqlite3_errmsg(ws2000db));
    free( errmsg);
    return (-1);
  } else {
    syslog(LOG_DEBUG, "writedb: sqlite3_open: OK\n");
  }

  /* database and rrd handling */
  for ( i = 0; i < nval; i++) {
    datadb( dataset_date, sensor_meas_no[i], meas_value[i], ws2000db);
    /* handling rrd */
    /* fetch names from database */
    if ( ( err = senspardb( sensor_meas_no[i], &spar, ws2000db)) != 0 ) {
      syslog(LOG_DEBUG,"writedb: senspardb returned error: %d\n", err);
    }
    if ( spar.sensor_no != sensor_no ) {
      syslog(LOG_WARNING, 
	     "writedb: sensor_no mismatch: sensor_no: %d : "
	     "spar.sensor_no: %d\n", sensor_no, spar.sensor_no);
      break;
    }
    syslog(LOG_DEBUG, 
	   "writedb: sensor_meas_no: %d : spar.sensor_no: %d",
	   sensor_meas_no[i], spar.sensor_no);
    syslog(LOG_DEBUG, 
	   "writedb: spar.sensor_name: %s: spar.par_name: %s", 
	   spar.sensor_name, spar.par_name);
    syslog(LOG_DEBUG, 
	   "writedb: %lu : sensor: %s : parameter: %s: %f",
	   (long int)dataset_date, spar.sensor_name, 
	   spar.par_name, meas_value[i]);
    snprintf(template,MAXMSGLEN,"%f", meas_value[i]);
    strncat(tstrg, ":", 1);
    strncat(tstrg, template, strlen(template));
  }
  snprintf( rrdfile, MAXMSGLEN, "%s", ws2000station.config.rrdpath);
  snprintf( tmpstr, MAXMSGLEN, "%s.rrd", spar.sensor_name);
  strncat( rrdfile, tmpstr, 2*MAXMSGLEN+1);
  syslog(LOG_DEBUG, "writedb: rrdfile: %s", rrdfile);
  syslog(LOG_DEBUG, "writedb: update string: %s", tstrg);
  snprintf(ustrg[2], MAXMSGLEN-2, "%s", tstrg);
  rrd_clear_error();
  rrd_get_context();
  rrd_update_r( rrdfile, NULL, 1, (const char **)(ustrg + 2));
  if ( ( err = rrd_test_error())) {
    syslog( LOG_ALERT, "writedb: RRD return code: %d\n",
            rrd_test_error());
  }

  free(ustrg[2]);
  free(ustrg);

  /* cleanup and close */
  sqlite3_close( ws2000db);
  syslog(LOG_DEBUG,"readstat: sqlite3_close ws2000db done\n");


  return(err);
}

/*
  read sensor data
*/
char *
readdb( char *wstation) {
  int err;
  char *errmsg;
  static char rbuf[NBUFF+1];;
  char s[TBUFF+1], buf[TBUFF+1], query[SBUFF+1];
  sqlite3_stmt *qcomp;
  struct tm *tm;
  time_t meastim;

  snprintf(rbuf, NBUFF, "");

  /* ---------------------------------- */
  /* read data of ws2000 weatherstation */
  /* ---------------------------------- */
  if ( ( err = strncmp( wstation,"ws2000",5)) == 0 ) {
    /* open sqlite db file */
    err = sqlite3_open( ws2000station.config.dbfile, &ws2000db);
    syslog(LOG_DEBUG, 
	 "readdb: sqlite3_open %s return value: %d : sqlite_errmsg: %s\n", 
	 ws2000station.config.dbfile,
	 err, sqlite3_errmsg(ws2000db));
    if ( err) {
      syslog( LOG_ALERT, "readdb: failed to open database %s. error: %s\n", 
	    ws2000station.config.dbfile, sqlite3_errmsg(ws2000db));
      free( errmsg);
      return (NULL);
    } else {
      syslog(LOG_DEBUG, "readdb: sqlite3_open: OK\n");
    }

    snprintf( query, SBUFF, 
      "SELECT DISTINCT sensornames.sensorname,parameternames.parameter_name, "
      "sensorupdate.last_update, sensordata.meas_value, "
      "parameternames.parameter_unit "
      "FROM " 
        "sensorupdate, sensordata,sensornames,sensorparameters,parameternames "
      "WHERE sensorupdate.last_update = sensordata.dataset_date "
      "AND "
        "sensorupdate.sensor_meas_no = sensordata.sensor_meas_no "
      "AND "
        "sensornames.sensor_no = sensorparameters.sensor_no "
      "AND "
        "sensorupdate.sensor_meas_no = sensorparameters.sensor_meas_no "
      "AND "
        "parameternames.parameter_no = sensorparameters.parameter_no"
      );
    err = sqlite3_prepare( ws2000db, query, -1, &qcomp, 0); 
    if ( err != SQLITE_OK ) {
      snprintf( rbuf, TBUFF, 
        "Error: readdb: select ws2000 data: err: %d : sqlite_errmsg: %s\n", 
        err, sqlite3_errmsg(ws2000db));
      syslog( LOG_ALERT, rbuf);
      snprintf( rbuf, TBUFF, "database error: please check installation\n");
      return(rbuf);
    }

    while( SQLITE_ROW == sqlite3_step(qcomp)) {
      meastim = (time_t )sqlite3_column_int( qcomp, 2); 
      tm = gmtime(&meastim);
      strftime(buf, sizeof(buf), "%b %e, %Y %H:%M:%S %Z", tm);
      snprintf(s, SBUFF, "%-12s %-18s %8.2f %-8s %-12s\n",
	(char *)sqlite3_column_text( qcomp, 0),
        (char *)sqlite3_column_text( qcomp, 1),
        (float)sqlite3_column_double( qcomp, 3),
        (char *)sqlite3_column_text( qcomp, 4),
        buf
      );
      strncat(rbuf,s,strlen(s));
    }
    err = sqlite3_finalize(qcomp);
    if ( err != SQLITE_OK ) {
      snprintf(rbuf, TBUFF, 
        "Error: readdb: select parametername: err: %d : sqlite_errmsg: %s\n", 
	err, sqlite3_errmsg(ws2000db));
      syslog( LOG_ALERT, rbuf);
      snprintf(rbuf, TBUFF, "database error: please check installation\n");
      return(rbuf);
    }

    /* cleanup and close */
    sqlite3_close( ws2000db);
    syslog(LOG_DEBUG,"readdb: sqlite3_close ws2000db done\n");

  /* --------------------------------- */
  /* read data of pcwsr weatherstation */      
  /* --------------------------------- */
  } else if ( ( err = strncmp( wstation,"pcwsr",5)) == 0 ) {
    printf("readdb: pcwsr : %s\n", wstation);
    /* open sqlite db file */
    err = sqlite3_open( pcwsrstation.config.dbfile, &pcwsrdb);
    syslog(LOG_DEBUG, 
	 "readdb: sqlite3_open %s return value: %d : sqlite_errmsg: %s", 
	 pcwsrstation.config.dbfile,
	 err, sqlite3_errmsg(pcwsrdb));
    if ( err) {
      syslog( LOG_ALERT, "readdb: failed to open database %s. error: %s", 
	    pcwsrstation.config.dbfile, sqlite3_errmsg(pcwsrdb));
      free( errmsg);
      return (NULL);
    } else {
      syslog(LOG_DEBUG, "readdb: sqlite3_open: OK\n");
    }

    snprintf( query, SBUFF, 
      "SELECT DISTINCT sensornames.sensorname, sensornames.sensor_no, "
      "parameternames.parameter_name, "
      "sensorupdate.last_update, sensordata.meas_value, "
      "parameternames.parameter_unit "
      "FROM " 
        "sensorupdate, sensordata,sensornames,sensorparameters,parameternames "
      "WHERE sensorupdate.last_update = sensordata.dataset_date "
      "AND "
        "sensorupdate.sensor_meas_no = sensordata.sensor_meas_no "
      "AND "
        "sensornames.sensor_no = sensorparameters.sensor_no "
      "AND "
        "sensorupdate.sensor_meas_no = sensorparameters.sensor_meas_no "
      "AND "
        "parameternames.parameter_no = sensorparameters.parameter_no"
      );
    err = sqlite3_prepare( pcwsrdb, query, -1, &qcomp, 0); 
    if ( err != SQLITE_OK ) {
      snprintf( rbuf, TBUFF,
        "Error: readdb: select pcwsr data: err: %d : sqlite_errmsg: %s\n", 
        err, sqlite3_errmsg(pcwsrdb));
      syslog( LOG_ALERT, rbuf);
      snprintf( rbuf, TBUFF, "database error: please check installation\n");
      return(rbuf);
    }

    snprintf( s, TBUFF, 
      "    sensorname parameter             value unit     dataset date\n"
      "-------------- -----------   ------------- -------- -------------------------\n");
    strncat(rbuf, s, strlen(s));
    while( SQLITE_ROW == sqlite3_step(qcomp)) {
      meastim = (time_t )sqlite3_column_int( qcomp, 3); 
      tm = gmtime(&meastim);
      strftime(buf, sizeof(buf), "%b %e, %Y %H:%M:%S %Z", tm);
      snprintf( s, TBUFF, "%15s%d %-18s %8.2f %-8s %-12s\n",
	(char *)sqlite3_column_text( qcomp, 0),
        (int)sqlite3_column_int( qcomp, 1),
        (char *)sqlite3_column_text( qcomp, 2),
        (float)sqlite3_column_double( qcomp, 4),
        (char *)sqlite3_column_text( qcomp, 5),
        buf
      );
      strncat(rbuf,s,strlen(s));
    }
    err = sqlite3_finalize(qcomp);
    if ( err != SQLITE_OK ) {
      snprintf( rbuf, TBUFF, 
        "Error: readdb: select parametername: err: %d : sqlite_errmsg: %s\n", 
	err, sqlite3_errmsg(pcwsrdb));
      syslog( LOG_ALERT, rbuf);
      snprintf( rbuf, TBUFF, "database error: please check installation");
      return(rbuf);
    }

    /* cleanup and close */
    sqlite3_close( pcwsrdb);
    syslog(LOG_DEBUG,"readdb: sqlite3_close pcwsrdb done\n");      

  /* ----------------------------------- */
  /* read data of onewire weatherstation */
  /* ----------------------------------- */
  } else if ( ( err = strncmp( wstation,"onewire",5)) == 0 ) {

    printf("readdb: onewire : %s\n", wstation);
    /* open sqlite db file */
    err = sqlite3_open( onewirestation.config.dbfile, &onewiredb);
    syslog(LOG_DEBUG, 
	 "readdb: sqlite3_open %s return value: %d : sqlite_errmsg: %s\n", 
	 onewirestation.config.dbfile,
	 err, sqlite3_errmsg(onewiredb));
    if ( err) {
      syslog( LOG_ALERT, "readdb: failed to open database %s. error: %s\n", 
	    onewirestation.config.dbfile, sqlite3_errmsg(onewiredb));
      free( errmsg);
      return (NULL);
    } else {
      syslog(LOG_DEBUG, "readdb: sqlite3_open: OK\n");
    }

    snprintf( query, SBUFF, 
      "SELECT DISTINCT sensorname.sensor_name, sensorname.sensor_no, "
      "parametername.parameter_name, "
      "sensorupdate.last_update, sensordata.meas_value, "
      "parametername.unit "
      "FROM " 
        "sensorupdate, sensordata,sensorname,sensordevparameters,parametername "
      "WHERE sensorupdate.last_update = sensordata.dataset_date "
      "AND "
        "sensorupdate.sensor_meas_no = sensordata.sensor_meas_no "
      "AND "
        "sensorname.sensor_no = sensordevparameters.sensor_no "
      "AND "
        "sensorupdate.sensor_meas_no = sensordevparameters.sensor_meas_no "
      "AND "
        "parametername.parameter_no = sensordevparameters.parameter_no"
      );
    err = sqlite3_prepare( onewiredb, query, -1, &qcomp, 0); 
    if ( err != SQLITE_OK ) {
      snprintf( rbuf, TBUFF,
        "Error: readdb: select pcwsr data: err: %d : sqlite_errmsg: %s\n", 
        err, sqlite3_errmsg(onewiredb));
      syslog( LOG_ALERT, rbuf);
      snprintf( rbuf, TBUFF, "database error: please check installation");
      return(rbuf);
    }

    snprintf( s, TBUFF, 
      "      sensorname parameter             value unit     dataset date\n"
      "---------------- -----------   ------------- -------- -------------------------\n");
    strncat(rbuf, s, strlen(s));
    while( SQLITE_ROW == sqlite3_step(qcomp)) {
      meastim = (time_t )sqlite3_column_int( qcomp, 3); 
      tm = gmtime(&meastim);
      strftime(buf, sizeof(buf), "%b %e, %Y %H:%M:%S %Z", tm);
      snprintf( s, TBUFF, " %15s %-18s %8.2f %-8s %-12s\n",
	(char *)sqlite3_column_text( qcomp, 0),
        (char *)sqlite3_column_text( qcomp, 2),
        (float)sqlite3_column_double( qcomp, 4),
        (char *)sqlite3_column_text( qcomp, 5),
        buf
      );
      strncat(rbuf,s,strlen(s));
    }

    err = sqlite3_finalize(qcomp);
    if ( err != SQLITE_OK ) {
      snprintf( rbuf, TBUFF, 
        "Error: readdb: select parametername: err: %d : sqlite_errmsg: %s\n", 
	err, sqlite3_errmsg(onewiredb));
      syslog( LOG_ALERT, rbuf);
      snprintf( rbuf, TBUFF, "database error: please check installation");
      return(rbuf);
    }

    /* cleanup and close */
    sqlite3_close( onewiredb);
    syslog(LOG_DEBUG,"readdb: sqlite3_close onewiredb done\n");      

  } else {
      return(NULL);
  }
  return(rbuf);
}

/*
  read 1hr old data 
*/
int 
readpar( time_t *meastim, float *measval, 
  int sensor_no, int sensor_meas_no, time_t timedif, char *wstation)
{
  int err, num;
  char query[SBUFF+1];
  char rbuf[NBUFF+1];
  char *errmsg;
  sqlite3_stmt *qcomp;

  /* handle ws2000 weatherstation */
  if ( ( err = strncmp( wstation,"ws2000",5)) == 0 ) {
    /* open sqlite db file */
    err = sqlite3_open( ws2000station.config.dbfile, &ws2000db);
    syslog(LOG_DEBUG, 
	 "readpar: sqlite3_open %s return value: %d : sqlite_errmsg: %s\n", 
	 ws2000station.config.dbfile,
	 err, sqlite3_errmsg(ws2000db));
    if ( err) {
      syslog( LOG_ALERT, "readpar: failed to open database %s. error: %s\n", 
	    ws2000station.config.dbfile, sqlite3_errmsg(ws2000db));
      free( errmsg);
      return(err);
    } else {
      syslog(LOG_DEBUG, "readpar: sqlite3_open: OK\n");
    }

    snprintf ( query, SBUFF,
      "SELECT DISTINCT sensornames.sensorname,parameternames.parameter_name, "
      "sensordata.dataset_date, sensordata.meas_value, "
      "parameternames.parameter_unit "
      "FROM " 
        "sensorupdate, sensordata,sensornames,sensorparameters,parameternames "
      "WHERE sensordata.dataset_date >= sensorupdate.last_update - 3600 "
      "AND "
        "sensorupdate.sensor_meas_no = sensordata.sensor_meas_no "
      "AND "
        "sensornames.sensor_no = sensorparameters.sensor_no "
      "AND "
        "sensorupdate.sensor_meas_no = sensorparameters.sensor_meas_no "
      "AND "
        "parameternames.parameter_no = sensorparameters.parameter_no "
      "AND "
        "sensornames.sensor_no = %d "
      "AND "
        "sensordata.sensor_meas_no = %d",
      sensor_no, sensor_meas_no
      );
    err = sqlite3_prepare( ws2000db, query, -1, &qcomp, 0); 
    if ( err != SQLITE_OK ) {
      syslog( LOG_ALERT, 
        "Error: readpar: select ws2000 data: err: %d : sqlite_errmsg: %s\n", 
        err, sqlite3_errmsg(ws2000db));
      return(err);
    }

    num=0;
    while( SQLITE_ROW == sqlite3_step(qcomp)) {
      if ( num == 0 ) {
        *meastim = (time_t )sqlite3_column_int( qcomp, 2); 
        *measval = (float )sqlite3_column_double( qcomp, 3); 
        printf("readpar: meastim: %lu measval %f\n", 
          (long int)*meastim, *measval);
        num++;
      }
    }

    err = sqlite3_finalize(qcomp);
    if ( err != SQLITE_OK ) {
      snprintf(rbuf, TBUFF, 
        "Error: readpar: select parametername: err: %d : sqlite_errmsg: %s\n", 
	err, sqlite3_errmsg(ws2000db));
      syslog( LOG_ALERT, rbuf);
      return(err);
    }

    /* cleanup and close */
    sqlite3_close( ws2000db);
    syslog(LOG_DEBUG,"readpar: sqlite3_close ws2000db done\n");

    /* handle pcwsr weatherstation */
  } else if ( ( err = strncmp( wstation,"pcwsr",5)) == 0 ) {
    /* open sqlite db file */
    err = sqlite3_open( pcwsrstation.config.dbfile, &pcwsrdb);
    syslog(LOG_DEBUG, 
	 "readpar: sqlite3_open %s return value: %d : sqlite_errmsg: %s\n", 
	 pcwsrstation.config.dbfile,
	 err, sqlite3_errmsg(pcwsrdb));
    if ( err) {
      syslog( LOG_ALERT, "readpar: failed to open database %s. error: %s\n", 
	    pcwsrstation.config.dbfile, sqlite3_errmsg(pcwsrdb));
      free( errmsg);
      return(err);
    } else {
      syslog(LOG_DEBUG, "readpar: sqlite3_open: OK\n");
    }

    snprintf( query, SBUFF,
      "SELECT DISTINCT sensornames.sensorname,parameternames.parameter_name, "
      "sensordata.dataset_date, sensordata.meas_value, "
      "parameternames.parameter_unit "
      "FROM " 
        "sensorupdate, sensordata,sensornames,sensorparameters,parameternames "
      "WHERE sensordata.dataset_date >= sensorupdate.last_update - 3600 "
      "AND "
        "sensorupdate.sensor_meas_no = sensordata.sensor_meas_no "
      "AND "
        "sensornames.sensor_no = sensorparameters.sensor_no "
      "AND "
        "sensorupdate.sensor_meas_no = sensorparameters.sensor_meas_no "
      "AND "
        "parameternames.parameter_no = sensorparameters.parameter_no "
      "AND "
        "sensornames.sensor_no = %d "
      "AND "
        "sensordata.sensor_meas_no = %d",
      sensor_no, sensor_meas_no
      );
    err = sqlite3_prepare( pcwsrdb, query, -1, &qcomp, 0); 
    if ( err != SQLITE_OK ) {
      syslog( LOG_ALERT, 
        "Error: readpar: select pcwsr data: err: %d : sqlite_errmsg: %s\n", 
        err, sqlite3_errmsg(pcwsrdb));
      return(err);
    }

    num=0;
    while( SQLITE_ROW == sqlite3_step(qcomp)) {
      if ( num == 0 ) {
        *meastim = (time_t )sqlite3_column_int( qcomp, 2); 
        *measval = (float )sqlite3_column_double( qcomp, 3); 
        printf("readpar: meastim: %lu measval %f\n", 
          (long int)*meastim, *measval);
        num++;
      }
    }
    err = sqlite3_finalize(qcomp);
    if ( err != SQLITE_OK ) {
      snprintf( rbuf, TBUFF, 
        "Error: readpar: select parametername: err: %d : sqlite_errmsg: %s\n", 
	err, sqlite3_errmsg(pcwsrdb));
      syslog( LOG_ALERT, rbuf);
      return(err);
    }

    /* cleanup and close */
    sqlite3_close( pcwsrdb);
    syslog(LOG_DEBUG,"readpar: sqlite3_close pcwsrdb done\n");      

  } else if ( ( err = strncmp( wstation,"1wire",5)) == 0 ) {
     return(err);
  } else {
     return(err);
  }
  return(err);
}

/*
  read sensor status data
*/
char *
readstat ( char *wstation) {
  int i, err, sensor_no, sensoraddr;
  char *errmsg;
  static char rbuf[NBUFF+1], s[TBUFF+1], buf[TBUFF+1];
  char query[SBUFF+1], sensorvers[TBUFF+1], sensorname[TBUFF+1];
  char devicetyp[TBUFF+1], serialnum[TBUFF+1], parametername[TBUFF+1];
  char unit[TBUFF+1], gain[TBUFF+1], offset[TBUFF+1];
  int familycode, lastseen;
  sqlite3_stmt *qcomp;
  struct tm *tm;
  time_t lastread, statread;

  snprintf(rbuf, NBUFF, "");
  /* WS2000 weatherstation handling */
  if ( ( err = strncmp( wstation,"ws2000",6)) == 0) {

    err = sqlite3_open( ws2000station.config.dbfile, &ws2000db);
    syslog(LOG_DEBUG, 
      "readstat: sqlite3_open %s return value: %d : sqlite_errmsg: %s\n", 
      ws2000station.config.dbfile,
       err, sqlite3_errmsg(ws2000db));

    if ( err) {
      syslog( LOG_ALERT, "readstat: failed to open database %s. error: %s\n", 
	ws2000station.config.dbfile, sqlite3_errmsg(ws2000db));
      free( errmsg);
      snprintf(rbuf, SBUFF, "WS2000 database problem");
      return NULL;
    } else {
      syslog(LOG_DEBUG, "readstat: sqlite3_open: OK\n");
    }

    snprintf( query, SBUFF, 
      "select sensor_no, max(statusset_date), sensor_status "
      "from sensorstatus group by sensor_no");

    err = sqlite3_prepare( ws2000db, query, -1, &qcomp, 0); 
    if ( err != SQLITE_OK ) {
      syslog( LOG_ALERT,
        "Error: readstat: select parametername: err: %d : sqlite_errmsg: %s\n", 
        err, sqlite3_errmsg(ws2000db));
      snprintf(rbuf, SBUFF, 
        "WS2000 database problem. Please check installation");

      /* cleanup and close */
      sqlite3_close( ws2000db);
      syslog(LOG_DEBUG,"readstat: sqlite3_close ws2000 done\n");
      return(rbuf);

    }

    while( SQLITE_ROW == sqlite3_step(qcomp)) {
      sensor_no   = sqlite3_column_int(qcomp,0);  
      ws2000station.sensor[sensor_no].lastseen = sqlite3_column_int(qcomp,1);  
      ws2000station.sensor[sensor_no].status   = sqlite3_column_int(qcomp,2);  
    }
   
    err = sqlite3_finalize(qcomp);
    if ( err != SQLITE_OK ) {
      syslog( LOG_ALERT,
        "Error: readstat: select parametername: err: %d : sqlite_errmsg: %s\n", 
        err, sqlite3_errmsg(ws2000db));
      snprintf(rbuf, SBUFF, 
        "WS2000 Database problem. Please check installation");

      /* cleanup and close */
      sqlite3_close( ws2000db);
      syslog(LOG_DEBUG,"readstat: sqlite3_close ws2000 done\n");

      return(rbuf);
    }

    /* database cleanup and close */
    sqlite3_close( ws2000db);
    syslog(LOG_DEBUG,"readstat: sqlite3_close ws2000db done\n");

    /* prepare status response WS2000 */
    snprintf( rbuf , TBUFF, "WS2000 weatherstation status\n"
      "version\t\t:\t%x\nmeasure interval:\t%lu (min)\n",
      ws2000station.status.version,
      (long int)ws2000station.status.interval
      );

    if ( ws2000station.status.DCFstat == 1 ) {
      snprintf( s, TBUFF, "DCF status\t:\t%d (DCF receiver present)\n",
		ws2000station.status.DCFstat);
    }
    else {
      snprintf( s, TBUFF,"DCF status\t:\t%d (no DCF receiver found)\n",
               ws2000station.status.DCFstat);
    }
    strncat( rbuf, s, strlen(s));
    if ( ws2000station.status.DCFsync == 1 ) {
      snprintf( s, TBUFF, "DCF sync.\t:\t%d (DCF synchronized)\n",
		ws2000station.status.DCFsync);
    }
    else {
      snprintf ( s, TBUFF,"DCF sync.\t:\t%d (DCF NOT synchronized)\n",
		 ws2000station.status.DCFsync);
    }
    strncat( rbuf, s, strlen(s));
    if ( ws2000station.status.HFstat == 1 ) {
      snprintf( s, TBUFF, "HF status\t:\t%d (with HF)\n",
		ws2000station.status.HFstat);
    }
    else {
      snprintf( s, TBUFF, "HF status\t:\t%d (without HF)\n",
		ws2000station.status.HFstat);
    }
    strncat( rbuf, s, strlen(s));
    snprintf( s, TBUFF, "battery status\t:\t%d\n",
	      ws2000station.status.Battstat);
    strncat( rbuf, s, strlen(s));
    
    snprintf ( s, TBUFF, "sensor number\t:\t%d (sensors)\n",
	       ws2000station.status.numsens);
    strncat( rbuf, s, strlen(s));
      
    snprintf( s, TBUFF, "sensorname\tsensorstatus\tlastseen\n"
	      "------------\t------------\t----------\n"
	      );
    strncat( rbuf, s, strlen(s));
    time(&statread);
    lastread = statread - ws2000station.status.lastread;
    
    for ( i = 1; i <=ws2000station.status.numsens; i++) {
      tm = gmtime(&ws2000station.sensor[i].lastseen);
      strftime( buf, sizeof(buf), "%b %e, %Y %H:%M:%S %Z", tm);
      snprintf ( s , TBUFF, "%12s\t%d\t\t%s\n",
		 ws2000station.sensor[i].sensorname,
		 ws2000station.sensor[i].status,
		 buf
		 );
      strncat( rbuf, s, strlen(s));
    }

    /* function returns here for WS2000 weatherstation */
    return(rbuf);

  /* PCWSR weatherstation status handling */
  } else if ( ( err = strncmp( wstation,"pcwsr",5)) == 0) {
    err = sqlite3_open( pcwsrstation.config.dbfile, &pcwsrdb);
    syslog(LOG_DEBUG, 
      "readstat: sqlite3_open %s return value: %d : sqlite_errmsg: %s\n", 
      pcwsrstation.config.dbfile,
       err, sqlite3_errmsg(pcwsrdb));
    if ( err) {
      syslog( LOG_ALERT, "readstat: failed to open database %s. error: %s\n", 
	pcwsrstation.config.dbfile, sqlite3_errmsg(pcwsrdb));
      free( errmsg);
      snprintf(rbuf, SBUFF, "PCWSR database problem.");
      return NULL;
    } else {
      syslog(LOG_DEBUG, "readstat: sqlite3_open: OK\n");
    }

    snprintf ( query, SBUFF, 
      "SELECT DISTINCT sensorparameters.sensor_no, sensornames.sensorname, "
      "sensornames.sensoraddr, sensornames.sensorvers, "
      "sensorupdate.last_update "
      "FROM sensorupdate,sensorparameters,sensornames "
      "WHERE sensorparameters.sensor_meas_no = sensorupdate.sensor_meas_no "
      "AND sensorparameters.sensor_no = sensornames.sensor_no "
      "AND last_update > 0");

    err = sqlite3_prepare( pcwsrdb, query, -1, &qcomp, 0); 
    if ( err != SQLITE_OK ) {
      syslog( LOG_ALERT,
        "Error: readstat: select parametername: err: %d : sqlite_errmsg: %s\n", 
        err, sqlite3_errmsg(pcwsrdb));
      snprintf(rbuf, SBUFF, "PCWSR database problem.");

      /* cleanup and close */
      sqlite3_close( pcwsrdb);
      syslog(LOG_DEBUG,"readstat: sqlite3_close pcwsrdb done\n");
      return(rbuf);
    }

    /* prepare PCWSR status response */
    snprintf( rbuf, NBUFF, "PCWSR weatherstation status\n");
    
    snprintf( s, TBUFF, "sensorname\taddress\tversion\tlastseen\n"
	      "--------------\t-------\t-------\t----------\n"
	      );
    strncat( rbuf, s, strlen(s));

    while( SQLITE_ROW == sqlite3_step(qcomp)) {
      sensor_no   = sqlite3_column_int(qcomp,0); 
      snprintf( sensorname, SBUFF, (char *)sqlite3_column_text(qcomp,1));
      sensoraddr  = sqlite3_column_int(qcomp,2);
      snprintf( sensorvers, SBUFF, (char *)sqlite3_column_text(qcomp,3)); 
      pcwsrstation.sensor[sensor_no].lastseen = sqlite3_column_int(qcomp,4);  
      tm = gmtime(&pcwsrstation.sensor[sensor_no].lastseen);
      strftime(buf, sizeof(buf), "%b %e, %Y %H:%M:%S %Z", tm);
      snprintf ( s , TBUFF, "%12s%d\t0x%d\t%s\t%s\n",
		 sensorname,sensor_no,
		 sensoraddr, sensorvers,
		 buf
        );
      strncat( rbuf, s, strlen(s));
    }

    printf("readstat: response: %s\n", rbuf);

    err = sqlite3_finalize(qcomp);
    if ( err != SQLITE_OK ) {
      syslog( LOG_ALERT,
        "Error: readstat: select parametername: err: %d : sqlite_errmsg: %s\n", 
        err, sqlite3_errmsg(pcwsrdb));
      snprintf( rbuf, SBUFF, "PCWSR database problem.");
      /* cleanup and close */
      sqlite3_close( pcwsrdb);
      syslog(LOG_DEBUG,"readstat: sqlite3_close pcwsrdb done\n");
      
      return(rbuf);
    }

    /* cleanup and close */
    sqlite3_close( pcwsrdb);
    syslog(LOG_DEBUG,"readstat: sqlite3_close pcwsrdb done\n");

    /* function returns here if PCWSR station */
    return(rbuf);

  /* ONEWIRE weatherstation status handling */
  } else if ( ( err = strncmp( wstation,"onewire",6)) == 0) {
    err = sqlite3_open( onewirestation.config.dbfile, &onewiredb);
    syslog(LOG_DEBUG, 
      "readstat: sqlite3_open %s return value: %d : sqlite_errmsg: %s\n", 
      onewirestation.config.dbfile,
       err, sqlite3_errmsg(onewiredb));
    if ( err) {
      syslog( LOG_ALERT, "readstat: failed to open database %s. error: %s\n", 
	onewirestation.config.dbfile, sqlite3_errmsg(onewiredb));
      free( errmsg);
      snprintf(rbuf, SBUFF, "ONEWIRE database problem.\n");
      return NULL;
    } else {
      syslog(LOG_DEBUG, "readstat: sqlite3_open: OK\n");
    }

    snprintf ( query, SBUFF, 
      "SELECT DISTINCT sn.sensor_no, sn.sensor_name, "
         "dt.devicetyp, dt.familycode, dt.serialnum, "
         "pn.parameter_name, pn.unit, pn.gain, pn.offset, su.last_update "
      "FROM sensorname as sn, sensorupdate as su, devicetyp as dt, parametername as pn, sensordevparameters as sdp "
      "WHERE sn.sensor_no = sdp.sensor_no "
      "AND dt.device_no = sdp.device_no "
      "AND pn.parameter_no = sdp.parameter_no "
      "AND su.sensor_meas_no = sdp.sensor_meas_no "
      "AND last_update > 0");

    err = sqlite3_prepare( onewiredb, query, -1, &qcomp, 0); 
    if ( err != SQLITE_OK ) {
      syslog( LOG_ALERT,
        "Error: readstat: select parametername: err: %d : sqlite_errmsg: %s\n", 
        err, sqlite3_errmsg(onewiredb));
      snprintf(rbuf, SBUFF, "ONEWIRE database problem.\n");

      /* cleanup and close */
      sqlite3_close( onewiredb);
      syslog(LOG_DEBUG,"readstat: sqlite3_close onewiredb done\n");
      return(rbuf);
    }

    /* prepare ONEWIRE status response */
    snprintf( rbuf, NBUFF, "ONEWIRE weatherstation status\n");
    
    snprintf( s, TBUFF, " #     sensorname  device        family serial number\tparameter\tunit\tgain\toffset\tlast seen\n"
	      "-- --------------- ------------- ------\t---------------- ------------ ------ ------  ------\t----------------------- \n"
	      );
    strncat( rbuf, s, strlen(s));

    while( SQLITE_ROW == sqlite3_step(qcomp)) {
      sensor_no   = sqlite3_column_int(qcomp,0); 
      snprintf( sensorname, SBUFF, (char *)sqlite3_column_text(qcomp,1));
      snprintf( devicetyp, SBUFF, (char *)sqlite3_column_text(qcomp,2));
      familycode  = sqlite3_column_int(qcomp,3);
      snprintf( serialnum, SBUFF, (char *)sqlite3_column_text(qcomp,4)); 
      snprintf( parametername, SBUFF, (char *)sqlite3_column_text(qcomp,5)); 
      snprintf( unit, SBUFF, (char *)sqlite3_column_text(qcomp,6)); 
      snprintf( gain, SBUFF, (char *)sqlite3_column_text(qcomp,7)); 
      snprintf( offset, SBUFF, (char *)sqlite3_column_text(qcomp,8)); 
      lastseen = sqlite3_column_int(qcomp,9);  
      tm = gmtime(&lastseen);
      strftime(buf, sizeof(buf), "%b %e, %Y %H:%M:%S %Z", tm);
      snprintf ( s , TBUFF, "%2d %15s %13s %2d\t%s %-12s %-6s\t%s\t%s\t%s\n",
		 sensor_no, sensorname, devicetyp, familycode,
		 serialnum, parametername, unit, gain, offset, buf
        );
      strncat( rbuf, s, strlen(s));
    }

    printf("readstat: response: %s\n", rbuf);

    err = sqlite3_finalize(qcomp);
    if ( err != SQLITE_OK ) {
      syslog( LOG_ALERT,
        "Error: readstat: select parametername: err: %d : sqlite_errmsg: %s\n", 
        err, sqlite3_errmsg(onewiredb));
      snprintf( rbuf, SBUFF, "ONEWIRE database problem.\n");
      /* cleanup and close */
      sqlite3_close( onewiredb);
      syslog(LOG_DEBUG,"readstat: sqlite3_close onewiredb done\n");
      
      return(rbuf);
    }

    /* cleanup and close */
    sqlite3_close( onewiredb);
    syslog(LOG_DEBUG,"readstat: sqlite3_close onewire done\n");

    /* function returns here if ONEWIRE station */
    return(rbuf);
  } else {
    snprintf(rbuf, SBUFF, "Unsupported database: %s.", wstation);
    return(rbuf);
  }
  return(NULL);
}

int
maxsensmeas( sqlite3 *onewiredb) {
  int max_sens_meas;
  int err, rowcnt;
  char query[SBUFF+1];
  sqlite3_stmt *qcomp;

  snprintf(query, SBUFF, "SELECT COUNT(*) "
    "FROM sensordevparameters");
  syslog(LOG_DEBUG, "maxsensmeas: sql: %s", query);

  err = sqlite3_prepare( onewiredb, query, -1, &qcomp, 0);
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
            "Error: select maxsensmeas: err: %d : sqlite_errmsg: %s\n",
            err, sqlite3_errmsg(onewiredb));
    return(1);
  }

  while( SQLITE_ROW == sqlite3_step(qcomp)) {
    syslog(LOG_DEBUG, "maxsensmeas  : max_sens_meas: %d",
           sqlite3_column_int(qcomp, 0));
    max_sens_meas = sqlite3_column_int(qcomp, 0);
    rowcnt++;
  }

  err = sqlite3_finalize(qcomp);
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
            "Error: select parametername: err: %d : sqlite_errmsg: %s\n",
            err, sqlite3_errmsg(onewiredb));
    return(1);
  }

  if ( rowcnt == 0) {
    syslog( LOG_ALERT, "Error: cant find relation sensor - device - parameter");
    return(1);
  }

  return(max_sens_meas);
}
