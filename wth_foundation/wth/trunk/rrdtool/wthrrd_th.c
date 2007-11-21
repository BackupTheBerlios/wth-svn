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
  int rargc;
  char *filename = "Humidity1.rrd";
  char *template = "Humidity1";

  wctx = rrd_get_context();
  printf("wthrrd_th: argc: %d\n", argc);
  printf("wthrrd_th: argv[0]: %s\n", argv[0]);
  printf("wthrrd_th: argv[1]: %s\n", argv[1]);
  printf("wthrrd_th: argv[2]: %s\n", argv[2]);

  rrd_update_r( filename, template, argc-2, (const char **)(argv + 2));
  //rrd_update(argc, argv);
  if ( rrd_test_error()) {
    printf("RRD Error: %s\n", rrd_get_error());
    return(-1); 
  }
  return 0;
}

/*
  compile command:
  gcc -L/usr/local/lib -I/usr/local/include -I/usr/include -lpthread \
  -lrrd_th -o wthrrd_th wthrrd_th.c

  sample usage:
  ./wthrrd_th Humidity1.rrd 973709210:50.0
 
  create a sample rrd to work with:
  rrdtool create Humidity1.rrd  --start 973709200 --step 300 \
  DS:Humidity1:GAUGE:600:10:100 \
  RRA:AVERAGE:0.5:1:600 \
  RRA:AVERAGE:0.5:6:700 \
  RRA:AVERAGE:0.5:24:775 \
  RRA:AVERAGE:0.5:288:797

*/
