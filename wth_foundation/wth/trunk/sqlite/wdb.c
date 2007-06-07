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
 
  /* insert sensor data */
  snprintf(query, querylen, 
           "INSERT INTO sensordata VALUES ( NULL, %d, %d, %f)",
           1181248994, 3, 11.7); 

  printf("query: \"%s\"\n", query);
  err = sqlite3_exec( ws2000db, query, NULL, NULL, NULL);
  printf("err: %d : sqlite_errmsg: %s\n", err, sqlite3_errmsg(ws2000db));
 
  /* cleanup and close */
  sqlite3_close( ws2000db);
  printf("WS2000 sqlite done\n");
  return 0;
}

/*
  compile command:
  gcc -I/usr/local/include -L/usr/local/lib -o wdb wdb.c -Wall -lsqlite
*/
