/*

  C code to benchmark data consumption and read/write perfomance of 
  weather data sets
  
  this one uses the old data structure wthio

  $Id: wthio_bench.c,v 1.1 2002/07/04 09:51:57 jahns Exp $


*/

#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXDATA 1024
#define MAXSENSOR 8


/* data structures */
struct dataset {
  time_t time;
  int sign;
  float value;
};

struct sensor {
  int status;
  char *type;
  struct dataset mess[MAXDATA];
};

struct DCFstruct {
  int stat;
  int sync;
  time_t time;
};

struct wstatus {
  int nsens;
  int version;
  int intvaltime;
  long ndats;
  char *ebuf;
};


struct wthio {
  struct sensor sens[MAXSENSOR];
  struct DCFstruct DCF;
  struct wstatus wstat;
};  

typedef struct wthio wio_typ;

int
main ( int argc, char **argv) {

  printf("wthio-bench: sizeof(wio_typ): %d\n", sizeof(wio_typ)); 

  return 0;
}










