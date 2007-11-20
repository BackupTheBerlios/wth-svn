/*
  threaded version to write rrd

*/

#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <rrd.h>


int main ( int argc, char **argv) {
  //struct rrd_context *wctx;
  long *pdp_step;
  int rargc;
  char *template = "Temp1:Humidity1:Rain:Windspeed:Winddir:Winddev:PressureI";
  //char **rargv = "887457267:22.5:50.0:1000:30.0:135.0:22.5:978";

  //wctx = rrd_get_context();
  rargc = 1;
     
  //rrd_update_r( "PressureI.rrd", template, argc, argv);
  rrd_update(argc, argv);
  if ( rrd_test_error()) {
    printf("RRD Error: %s\n", rrd_get_error());
    return(-1); 
  }
  return 0;
}

/*
  compile command:
  gcc -L/usr/local/lib -I/usr/local/include -I/usr/include -lpthread -lrrd_th -o wthrrd_th wthrrd_th.c

  sample usage:
  ./wthrrd_th PressureI.rrd 1200000000:22.7:50.0:1000:30.0:234:22.5:967

*/
