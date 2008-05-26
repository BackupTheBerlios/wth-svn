/*
  wthdb.c
  reading writing database data 

*/
#include "wthnew.h"

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
statdb( long statusset_date, int sensor_no, int sensor_status, sqlite3 *wthdb) 
{
  int err;
  int querylen = MAXQUERYLEN;
  char query[MAXQUERYLEN];

  snprintf(query, querylen, 
	   "INSERT INTO sensorstatus VALUES ( NULL, %lu, %d, %d)",
	   statusset_date, sensor_no, sensor_status); 
  err = sqlite3_exec( wthdb, query, NULL, NULL, NULL);
  if ( err) { 
    syslog(LOG_DEBUG,
	   "statdb: error: insert sensor status: err: %d : sqlite_errmsg: %s\n", 
	   err, sqlite3_errmsg(wthdb));
  } else {
    return(-1);
  }
  return(0);
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
    syslog(LOG_DEBUG, "senspardb  : sensor_meas_no: %d : sensor_no: %d : "
           "sensorname: %s : parametername: %s\n",
	   sensor_meas_no,
	   sqlite3_column_int(qcomp, 0),
	   sqlite3_column_text(qcomp, 1),
	   sqlite3_column_text(qcomp, 2));
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
  char *template;
  char **ustrg;

  senspar_t spar;
        
  if (( rrdfile = malloc(MAXMSGLEN)) == NULL )
    return(-1);

  if (( template = malloc(MAXMSGLEN)) == NULL)
    return(-1);

  ustrg = (char **)malloc(sizeof(char *));
  ustrg[2] = (char *)malloc(MAXMSGLEN*sizeof(char *));

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
	   "writedb: sensor_meas_no: %d : spar.sensor_no: %d: "
	   "spar.sensor_name: %s: spar.par_name: %s\n", 
	   sensor_meas_no[i], spar.sensor_no, spar.sensor_name,
	   spar.par_name);
    syslog(LOG_INFO, 
	   "writedb: %lu : sensor: %s : parameter: %s: %f\n",
	   (long int)dataset_date, spar.sensor_name, 
	   spar.par_name, meas_value[i]);
    snprintf(template,MAXMSGLEN,"%f", meas_value[i]);
    strncat(tstrg, ":", 1);
    strncat(tstrg, template, strlen(template));
  }

  /*
  snprintf( rrdfile, MAXMSGLEN, "%s%d.rrd", 
	    spar.sensor_name, spar.sensor_no);
  */
  snprintf( rrdfile, MAXMSGLEN, "%s.rrd", 
	    spar.sensor_name);

  syslog(LOG_DEBUG, "writedb: rrdfile: %s: update string: %s\n", 
	 rrdfile, tstrg);
  snprintf(ustrg[2], MAXMSGLEN-2, "%s", tstrg);
  rrd_clear_error();
  rrd_get_context();
  rrd_update_r( rrdfile, NULL, 1, (const char **)(ustrg + 2));
  if ( rrd_test_error()) {
    syslog( LOG_ALERT, "writedb: RRD Error: %s\n", rrd_get_error());
  }
  return(0);
}
