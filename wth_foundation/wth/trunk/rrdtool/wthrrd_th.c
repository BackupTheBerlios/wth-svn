/*
  threaded version to write rrd

*/

#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <rrd.h>


int main ( int argc, char **argv) {
  struct rrd_context *wctx;
  long *pdp_step;
  char *template = "Temp1:Humidity1:Rain:Windspeed:Winddir:Winddev:PressureI";

  wctx = rrd_get_context();

  rrd_update_r( "PressureI.rrd", template, argc, argv);
  if ( rrd_test_error()) {
    printf("RRD Error: %s\n", rrd_get_error());
    return(-1); 
  }
  return 0;
}

/*
  compile command:
  gcc -L/usr/local/lib -I/usr/local/include -I/usr/include -lpthread -lrrd_th -o wthrrd_th wthrrd_th.c
*/
