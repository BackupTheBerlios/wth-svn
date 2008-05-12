/*
  simple test program to write/read to sqlite db for
  WS2000 data

  Volker jahns <volker@thalreit.de>
*/
#include "../wthnew.h"
#define MAXQUERYLEN 256

int
main ( int argc, char **argv) {
  int err;
  int querylen =  MAXQUERYLEN;
  char query[MAXQUERYLEN];
  sqlite3 *ws2000db;
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
 
  /* insert sensor data 
     sample: Sensor1 Temperature 11.4
  */
  snprintf(query, querylen, 
           "INSERT INTO sensordata VALUES ( NULL, %d, %d, %f)",
           1181248994, 1, 11.4); 
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

  /* insert sensor data 
     sample: Sensor1 Humidity 67 %
  */
  snprintf(query, querylen, 
           "INSERT INTO sensordata VALUES ( NULL, %d, %d, %f)",
           1181248994, 2, 67.0); 
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

  /* insert sensor data 
     sample: Sensor2 Temperature 11.7
  */
  snprintf(query, querylen, 
           "INSERT INTO sensordata VALUES ( NULL, %d, %d, %f)",
           1181248994, 3, 11.7); 
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

  /* insert sensor data 
     sample rainsensor 1625 ml
  */
  snprintf(query, querylen, 
           "INSERT INTO sensordata VALUES ( NULL, %d, %d, %f)",
           1181248994, 17, 1625.0); 
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

  /* insert sensor data 
     sample: Wind sensor, windspeed 40 km/h
  */
  snprintf(query, querylen, 
           "INSERT INTO sensordata VALUES ( NULL, %d, %d, %f)",
           1181248994, 18, 40.0); 
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

  /* insert sensor data 
     sample: Winddirection Variation, 22.5 deg
  */
  snprintf(query, querylen, 
           "INSERT INTO sensordata VALUES ( NULL, %d, %d, %f)",
           1181248994, 19, 22.5); 
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

  /* insert sensor data 
     sample: Winddirection, 150 deg
  */
  snprintf(query, querylen, 
           "INSERT INTO sensordata VALUES ( NULL, %d, %d, %f)",
           1181248994, 20, 150.0); 
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



  /* cleanup and close */
  sqlite3_close( ws2000db);
  printf("WS2000 sqlite done\n");
  return 0;
}

/*
  compile command:
  gcc -I/usr/local/include -L/usr/local/lib -o wdb wdb.c -Wall -lsqlite3
*/
