/*
  wthdb.c
  reading writing database data 

*/
#include "wth.h"

/*
  datadb - insert measured values

*/
int
datadb( long dataset_date, int sensor_param, float meas_value, 
        sqlite3 *wthdb) 
{
  int err;
  int querylen = MAXQUERYLEN;
  char query[MAXQUERYLEN];

  /* insert data values */
  snprintf(query, querylen, 
           "INSERT INTO sensordata VALUES ( NULL, %lu, %d, %f)",
           dataset_date, sensor_param, meas_value); 
  err = sqlite3_exec( wthdb, query, NULL, NULL, NULL);
  if ( err) { 
    syslog(LOG_DEBUG,
	   "datadb: error: insert sensor data: err: %d : sqlite_errmsg: %s\n", 
	   err, sqlite3_errmsg(wthdb));
  }

  /* insert date when last data has been updated */
  snprintf(query, querylen, 
	   "UPDATE sensorupdate SET last_update = %lu WHERE sensor_meas_no = %d",
	   dataset_date, sensor_param); 
  err = sqlite3_exec( wthdb, query, NULL, NULL, NULL);
  if ( err) { 
    syslog(LOG_DEBUG, 
	   "datadb: error: update sensor date: err: %d : sqlite_errmsg: %s\n", 
	   err, sqlite3_errmsg(wthdb));
  }
  return(err);
}
/*
  statdb - insert status values

*/
int
statdb( int sensor_status[], time_t statusset_date, sqlite3 *wthdb) 
{
  int i, err;
  int querylen = MAXQUERYLEN;
  char query[MAXQUERYLEN];
  char tstrg[MAXMSGLEN];
  char *rrdfile;
  char *tmpstr;
  char *template;
  char **ustrg;

  err = 0;
  if (( rrdfile = malloc(MAXMSGLEN)) == NULL ) return(-1);
  if (( tmpstr = malloc(MAXMSGLEN)) == NULL ) return(-1);
  if (( template = malloc(MAXMSGLEN)) == NULL) return(-1);

  ustrg = malloc(sizeof(char)*MAXMSGLEN);
  ustrg[2] = malloc(sizeof(char)*MAXBUFF);
  snprintf(tstrg,MAXMSGLEN, "%d", statusset_date);
 
  for ( i = 1; i <= 18; i++) {
    snprintf(query, querylen, 
     "INSERT INTO sensorstatus VALUES ( NULL, %lu, %d, %d)",
     (long unsigned int) statusset_date, i, sensor_status[i]); 
    err = sqlite3_exec( wthdb, query, NULL, NULL, NULL);
    if ( err) { 
      syslog(LOG_DEBUG,
        "statdb: error: insert sensor status: err: %d : sqlite_errmsg: %s", 
        err, sqlite3_errmsg(wthdb));
    } 
    snprintf(template,MAXMSGLEN,"%d", sensor_status[i]);
    strncat(tstrg, ":", 1);
    strncat(tstrg, template, strlen(template));
  }
  snprintf( rrdfile, MAXMSGLEN, "%s", ws2000station.config.rrdpath);
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
  return(err);

}


/*
  newdb - insert new flag values

*/
int
newdb( long statusset_date, int sensor_no, int new_flag, sqlite3 *wthdb) 
{
  int err;
  int querylen = MAXQUERYLEN;
  char query[MAXQUERYLEN];

  snprintf(query, querylen, 
	   "INSERT INTO sensornewflag VALUES ( NULL, %lu, %d, %d)",
	   statusset_date, sensor_no, new_flag); 
  err = sqlite3_exec( wthdb, query, NULL, NULL, NULL);
  if ( err) { 
    syslog(LOG_ALERT,
	   "newdb: error: insert sensor status: err: %d : sqlite_errmsg: %s\n", 
	   err, sqlite3_errmsg(wthdb));
  } else {
    return(-1);
  }
  return(0);
}


/*
  senspardb - retrieve sensor number and parameter name
*/
int
senspardb( int sensor_meas_no, senspar_t *sspar, sqlite3 *wthdb)
{
  int err;
  const char *query;
  sqlite3_stmt *qcomp;

  query = mkmsg2("SELECT sp.sensor_no, sn.sensorname, pn.parameter_name "
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

  while( SQLITE_ROW == sqlite3_step(qcomp)) {
    syslog(LOG_DEBUG, "senspardb  : sensor_meas_no: %d : sensor_no: %d",
	   sensor_meas_no, sqlite3_column_int(qcomp, 0));
    syslog(LOG_DEBUG, "senspardb  : sensorname: %s : parametername: %s",
	   sqlite3_column_text(qcomp, 1), sqlite3_column_text(qcomp, 2));
    sspar->sensor_no   = sqlite3_column_int(qcomp,0);  
    sspar->sensor_name = (char *)sqlite3_column_text(qcomp,1);  
    sspar->par_name    = (char *)sqlite3_column_text(qcomp,2);  
  }
  err = sqlite3_finalize(qcomp);
  if ( err != SQLITE_OK ) {
    syslog( LOG_ALERT,
	    "Error: select parametername: err: %d : sqlite_errmsg: %s\n", 
	    err, sqlite3_errmsg(wthdb));
    return(1);
  }
  return(0);
}

int
writedb( int sensor_no, int nval, int sensor_meas_no[], time_t dataset_date, 
         float meas_value[], sqlite3 *wthdb ) {
  int i, err;
  char tstrg[MAXMSGLEN];
  char *rrdfile;
  char *tmpstr;
  char *template;
  char **ustrg;
  senspar_t spar;

  err = 0;        
  if (( rrdfile = malloc(MAXMSGLEN)) == NULL )
    return(-1);

  if (( tmpstr = malloc(MAXMSGLEN)) == NULL )
    return(-1);

  if (( template = malloc(MAXMSGLEN)) == NULL)
    return(-1);

  ustrg = malloc(sizeof(char)*MAXMSGLEN);
  ustrg[2] = malloc(sizeof(char)*MAXBUFF);

  snprintf(tstrg,MAXMSGLEN, "%d", dataset_date);
  /* database and rrd handling */
  for ( i = 0; i < nval; i++) {
    datadb( dataset_date, sensor_meas_no[i], meas_value[i], wthdb);
    /* handling rrd */
    /* fetch names from database */
    if ( ( err = senspardb( sensor_meas_no[i], &spar, wthdb)) != 0 ) {
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
  printf("snprintf ustrg done \n");
  rrd_clear_error();
  rrd_get_context();
  rrd_update_r( rrdfile, NULL, 1, (const char **)(ustrg + 2));
  if ( ( err = rrd_test_error())) {
    syslog( LOG_ALERT, "writedb: RRD return code: %d\n",
            rrd_test_error());
  }
  return(err);
}

/*
  read sensor data
*/
char *
readdb( char *wstation) {
  int err;
  char *errmsg;
  char *rbuf, *s;
  const char *query;
  sqlite3_stmt *qcomp;
  char buf[MAXLINE];
  struct tm *tm;
  time_t meastim;

  if ( ( rbuf = malloc( MAXBUFF+1)) == NULL )
    return NULL;
  //rbuf = mkmsg2("readdb called to show data");
  /* handle ws2000 weatherstation */
  if ( ( err = strncmp( wstation,"ws2000",5)) == 0 ) {
    printf("readdb: ws2000 : %s\n", wstation);
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
      syslog(LOG_DEBUG, "readdb: sqlite3_open: no error: OK\n");
    }

    query = mkmsg2(
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
      rbuf = mkmsg2(
        "Error: readdb: select ws2000 data: err: %d : sqlite_errmsg: %s\n", 
        err, sqlite3_errmsg(ws2000db));
      syslog( LOG_ALERT, rbuf);
      rbuf = mkmsg2("database error: please check installation");
      return(rbuf);
    }

    while( SQLITE_ROW == sqlite3_step(qcomp)) {
      meastim = (time_t )sqlite3_column_int( qcomp, 2); 
      tm = gmtime(&meastim);
      strftime(buf, sizeof(buf), "%b %e, %Y %H:%M:%S %Z", tm);
      s = mkmsg2("%-12s %-18s %8.2f %-8s %-12s\n",
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
      rbuf = mkmsg2(
        "Error: readdb: select parametername: err: %d : sqlite_errmsg: %s\n", 
	err, sqlite3_errmsg(ws2000db));
      syslog( LOG_ALERT, rbuf);
      rbuf = mkmsg2("database error: please check installation");
      return(rbuf);
    }

    /* cleanup and close */
    sqlite3_close( ws2000db);
    syslog(LOG_DEBUG,"readdb: sqlite3_close ws2000db done\n");
      
  } else {
      return(rbuf);
  }
  return(rbuf);
}

/*
  read 1hr old data 
*/
int 
readpar( time_t *meastim, float *measval, int sensor_no, int sensor_meas_no, time_t timedif, char *wstation)
{
  int err, num;
  const char *query;
  sqlite3_stmt *qcomp;

  /* handle ws2000 weatherstation */
  if ( ( err = strncmp( wstation,"ws2000",5)) == 0 ) {
    /* open sqlite db file must be done in calling function */

    query = mkmsg2(
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
        printf("readpar: meastim: %d measval %f\n", *meastim, *measval);
        num++;
      }
    }
  } else {
      return(err);
  }
  return(err);
}

/*
  read sensor status data
*/
int readstat ( char *wstation) {
  int err, sensor_no;
  char *errmsg;
  const char *query;
  sqlite3_stmt *qcomp;

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
      return -1;
    } else {
      syslog(LOG_DEBUG, "readstat: sqlite3_open: no error: OK\n");
    }

    query = mkmsg2("select sensor_no, max(statusset_date), sensor_status "
                   "from sensorstatus group by sensor_no");

    err = sqlite3_prepare( ws2000db, query, -1, &qcomp, 0); 
    if ( err != SQLITE_OK ) {
      syslog( LOG_ALERT,
        "Error: readstat: select parametername: err: %d : sqlite_errmsg: %s\n", 
        err, sqlite3_errmsg(ws2000db));
       return(1);
    }

    while( SQLITE_ROW == sqlite3_step(qcomp)) {
      syslog(LOG_DEBUG, "readstat  : sensor_no: %d : lastseen: %d : "
           "status: %d\n",
	   sqlite3_column_int(qcomp, 0),
	   sqlite3_column_int(qcomp, 1),
	   sqlite3_column_int(qcomp, 2));
      sensor_no   = sqlite3_column_int(qcomp,0);  
      ws2000station.sensor[sensor_no].lastseen = sqlite3_column_int(qcomp,1);  
      ws2000station.sensor[sensor_no].status   = sqlite3_column_int(qcomp,2);  
    }
    err = sqlite3_finalize(qcomp);
    if ( err != SQLITE_OK ) {
      syslog( LOG_ALERT,
	    "Error: readstat: select parametername: err: %d : sqlite_errmsg: %s\n", 
	    err, sqlite3_errmsg(ws2000db));
      return(1);
    }

    /* cleanup and close */
    sqlite3_close( ws2000db);
    syslog(LOG_DEBUG,"readstat: sqlite3_close ws2000db done\n");
  } else {
    return(err);
  }
  return(err);
}

