#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <sys/resource.h>
#include <pthread.h>

#define MAXLEN 256

int
main(void)
{

  int querylen = MAXLEN;
  char *errmsg;
  char query[MAXLEN];
  time_t date;
  int status[MAXLEN];  
  sqlite3        *db = NULL;
  int             i, ret;
  struct rusage pstat;

  time(&date);
  ret = getrusage( RUSAGE_SELF, &pstat);
  printf("memory check: "
	 "maxrss: %ld : ixrss: %ld idrss: %ld isrss : %ld\n",
	 pstat.ru_maxrss,
	 pstat.ru_ixrss, pstat.ru_idrss,pstat.ru_isrss);
  ret = 0;

  printf("OPEN ");
  ret = sqlite3_open("/var/tmp/test.db", &db);
  if (ret != SQLITE_OK) {
    printf("error[%d]: %s\n", ret, sqlite3_errmsg(db));
    return (EXIT_FAILURE);
  }
  printf("OK\n");


  for ( i = 1; i <= 18; i++) {
    status[i] = i;
    snprintf(query, querylen, 
	     "INSERT INTO sensorstatus VALUES ( NULL, %lu, %d, %d)",
	     (long unsigned int) date, i, status[i]); 
    ret = sqlite3_exec( db, query, NULL, NULL, &errmsg);
    if ( ret) { 
      printf("error: insert sensor status: ret: %d : sqlite_errmsg: %s", 
	     ret, sqlite3_errmsg(db));
    }
    if ( errmsg) {
      printf("errmsg: %s\n", errmsg);
      sqlite3_free(errmsg);
    }
  }

  printf("CLOSE ");
  ret = sqlite3_close(db);
  if (ret != SQLITE_OK) {
    printf("error[%d]: %s\n", ret, sqlite3_errmsg(db));
    return (EXIT_FAILURE);
  }
  printf("OK\n");
  ret = getrusage( RUSAGE_SELF, &pstat);
  printf("memory check: "
	 "maxrss: %ld : ixrss: %ld idrss: %ld isrss : %ld\n",
	 pstat.ru_maxrss,
	 pstat.ru_ixrss, pstat.ru_idrss,pstat.ru_isrss);
  ret = 0;
  return (EXIT_SUCCESS);
}

/*
 *
 * compile command: 
 * gcc -I. -I/usr/include -I/usr/local/include \
   -L/usr/local/lib -lpthread -lsqlite3 -o sqlitemem sqlitemem.c
 *
 */
