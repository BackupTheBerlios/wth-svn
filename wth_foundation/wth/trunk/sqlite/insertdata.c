/*
  insertdata.c

  simple test program to write sample data to sqlite db for
  WS2000 data

  Volker jahns <volker@thalreit.de>

*/
#include "../wthnew.h"
sqlite3 *ws2000db;

/*
  dataindb - insert measured values

*/
int
dataindb( long dataset_date, int sensor_param, float meas_value) {
  int err;
  int querylen = MAXQUERYLEN;
  char query[MAXQUERYLEN];

  snprintf(query, querylen,
           "INSERT INTO sensordata VALUES ( NULL, %lu, %d, %f)",
           dataset_date, sensor_param, meas_value);
  printf("query: \"%s\"\n", query);
  err = sqlite3_exec( ws2000db, query, NULL, NULL, NULL);
  if ( err) {
    fprintf( stderr,
             "Error: insert sensor data: err: %d : sqlite_errmsg: %s\n",
             err, sqlite3_errmsg(ws2000db));
  } else {
    fprintf( stderr, "Success: insert sensor data OK: sqlite_errmsg: %s\n",
             sqlite3_errmsg( ws2000db));
  }
  return(0);
}

int
main ( int argc, char **argv) {
  int err, rnd, maxrnd;
  float frnd;
  time_t dataset_date;
  char *errmsg = 0;
  char *ws2000dbfile = "ws2000.db";

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
 
  /* random data values */
  srand( getpid());
  rnd = rand();
  maxrnd = RAND_MAX;
  frnd = (float) rnd / (float) maxrnd;
  printf("rnd: %d | maxrnd: %d | frnd : %f\n", rnd, maxrnd, frnd);
  /* initialize time */
  time( &dataset_date);
  
  /* insert sensor sample: Sensor1 Temperature */
  dataindb( dataset_date, 1, 10.0 + 5.0 * ( (float)rand() /(float)maxrnd)); 

  /* insert sensor sample: Sensor1 Humidity  */
  dataindb( dataset_date, 2, 50.0 + 50.0 * ( (float)rand() /(float)maxrnd)); 

  /* insert sensor sample: Sensor2 Temperature 11.7 degC */
  dataindb( dataset_date, 3, -20.0 + 10.0 * ( (float)rand() /(float)maxrnd));
  /* insert sensor sample rainsensor 1625 ml */
  dataindb( dataset_date, 17, 1625); 

  /* insert sensor sample: Wind sensor, windspeed 40 km/h */
  dataindb( dataset_date, 18, 40.0); 

  /* insert sensor sample: Winddirection Variation, 22.5 deg */
  dataindb( dataset_date, 19, 22.5); 

  /* insert sensor sample: Winddirection, 150 deg */
  dataindb( dataset_date, 20, 150.0); 

  /* finalize stmt handle */

  /* cleanup and close */
  sqlite3_close( ws2000db);
  printf("WS2000 sqlite done\n");
  return 0;
}

/*
  compile command:
  gcc -I/usr/local/include -L/usr/local/lib -o insertdata insertdata.c -Wall -lsqlite3
*/
