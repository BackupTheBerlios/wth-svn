/*
 * testpg.c
 *
 *      Test the C version of libpq, the PostgreSQL frontend library.
 *
 * compile command
 *       gcc -Wall -lpq -o testpg testpg.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <pgsql/libpq-fe.h>

static void
exit_nicely(PGconn *conn)
{
    PQfinish(conn);
    exit(1);
}

int
main(int argc, char **argv)
{
  const char      *conninfo;
  PGconn         *conn;
  PGresult       *res;
  PQprintOpt     options = {0};
  ExecStatusType status;

  int            max_sens_meas;
  int            i;
  char           query[1024];
  /*
   * If the user supplies a parameter on the command line, use it as the
   * conninfo string; otherwise default to setting dbname=postgres and using
   * environment variables or defaults for all other connection parameters.
   */
  if (argc > 1)
      conninfo = argv[1];
  else
      conninfo = "dbname=onewire user=wth";

  /* Make a connection to the database */
  conn = PQconnectdb(conninfo);

  /* Check to see that the backend connection was successfully made */
  if (PQstatus(conn) != CONNECTION_OK)
  {
    fprintf(stderr, "Connection to database failed: %s\n",
            PQerrorMessage(conn));
    exit_nicely(conn);
  }

  /*
   * Our test case here involves using a SELECT statement 
   */
  snprintf(query, 1024, "SELECT COUNT(*) FROM sensorparameters");
  printf("testpg: sql: %s\n", query);

  res = PQexec( conn, query);
  status = PQresultStatus(res);
  printf("testpg: PQresultStatus: %s\n", PQresStatus( status));

  if ( res == NULL ) {
    fprintf( stderr, "testpg: SQL failed: %s\n", PQerrorMessage(conn));
    exit_nicely(conn);
  }


/*
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    fprintf(stderr, "testpg: error: %s\n", PQerrorMessage(conn));
    exit_nicely(conn);
  }
*/

  printf("testpg: output looping over PQntuples and print PQgetvalue\n");
  for ( i = 0; i < PQntuples(res); i++) {
    max_sens_meas = (int )atoi(PQgetvalue(res, i, 0));
    printf("testpg: max_sens_meas: %d\n", max_sens_meas);
  }

  printf("testpg: output using PQprint function\n");

  options.header   = 1;
  options.align    = 1;
  options.fieldSep = "|";
  PQprint( stdout, res, &options );
  PQclear(res);

  /* close the connection to the database and cleanup */
  PQfinish(conn);
  printf("testpg: testcase finished OK\n");
  return 0;
}

