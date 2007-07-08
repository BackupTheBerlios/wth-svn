/*
  simple test program to read WS2000 sensordata from sqlite db 

  $Id$
  Volker Jahns <volker@thalreit.de>

*/
#include "../wthnew.h"
#define MAXQUERYLEN 1024

int
getdbdata() {
  int err;
  time_t dataset_date;
  float meas_value;
  int querylen =  MAXQUERYLEN;
  char query[MAXQUERYLEN];
  char *errmsg = 0;
  char *ws2000dbfile = "ws2000.db";
  sqlite3 *ws2000db;
  sqlite3_stmt *wsth;
  const char *sqltail;

  /* open sqlite db file */
  err = sqlite3_open( ws2000dbfile, &ws2000db);
  printf("err: %d : sqlite_errmsg: %s\n", err, sqlite3_errmsg(ws2000db));
  if ( err) {
    fprintf( stderr, "Failed to open databse %s. Error: %s\n", 
                     ws2000dbfile, sqlite3_errmsg(ws2000db));
    free( errmsg);
    return -1;
  } else {
    printf("sqlite3_open: no error: OK\n");
  }

  /* retrieve sensor data */
  /* prepare select statement */
  snprintf(query, querylen, 
           " SELECT sd.dataset_no, sd.dataset_date, "
                    "sn.sensorname, pn.parameter_name," 
                    "sd.meas_value, pn.parameter_unit "
             "FROM sensorparameters AS sp "
             "JOIN sensornames AS sn "
               "ON sp.sensor_no = sn.sensor_no "
             "JOIN parameternames AS pn "
               "ON sp.parameter_no = pn.parameter_no "
             "JOIN sensordata AS sd " 
               "ON sp.sensor_meas_no = sd.sensor_meas_no");
  /* printf("query: \"%s\"\n", query); */

  /* prepare compiled statement handle */
  err = sqlite3_prepare( ws2000db, query, querylen, &wsth, &sqltail);
  if ( err != SQLITE_OK) { 
    fprintf( stderr,
      "Error: sqlite3_prepare failed: err: %d : sqlite_errmsg: %s\n", 
      err, sqlite3_errmsg(ws2000db));
  } 

  do {
    err = sqlite3_step( wsth);
    if ( err == SQLITE_BUSY ) {
      fprintf( stderr,
        "Error: sqlite3_step failed: err: %d : sqlite_errmsg: %s\n", 
        err, sqlite3_errmsg(ws2000db));
    } else if ( err == SQLITE_DONE) {
      break;
    }

    dataset_date = sqlite3_column_int( wsth, 1);
    meas_value   = sqlite3_column_double( wsth, 4);
    printf("dataset_date: %lu;  meas_value: %f\n", 
      (long int)dataset_date, meas_value);  

  } while ( ( err != SQLITE_DONE) && ( err == SQLITE_ROW )) ;
 
  if ( ( err = sqlite3_finalize( wsth))) {
    fprintf( stderr,
      "Error: sqlite3_finalize failed: err: %d : sqlite_errmsg: %s\n", 
      err, sqlite3_errmsg(ws2000db));
  }
 
  /* cleanup and close */
  if ( ( err = sqlite3_close( ws2000db))) {
    fprintf( stderr,
      "Error: sqlite3_close failed: err: %d : sqlite_errmsg: %s\n", 
      err, sqlite3_errmsg(ws2000db));
  }
  printf("getdbdata: WS2000 sqlite done\n");
  return(0);
}


int
main ( int argc, char **argv) {
  int ret;

  ret = getdbdata();

  return 0;
}

/*
  compile command:
  gcc -I/usr/local/include -L/usr/local/lib -o getdbdata getdbdata.c -Wall -lsqlite3
*/
