/*

  avgdata_tst.c

  average a couple of measurement data
  for general use in wth code

  compile command:
  gcc -Wall -o avgdata_tst avgdata_tst.c


*/

#define MAXSENSMEAS 256

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

struct mset {
  double mtime;
  float mval;
  struct mset *next;
};

int mcycle = 3;

int 
addmdat( struct mset ** mlist_ref, 
         double mtime, 
         double mval) 
{
  struct mset *meas_set;
  if ( ( meas_set = ( struct mset *)malloc ( sizeof ( struct mset))) == NULL ) {
    return 1;
  } else {
    meas_set->mtime = mtime;
    meas_set->mval  = mval;
    meas_set->next  = *mlist_ref;
    *mlist_ref = meas_set;
    return 0;
  }
}

void 
rstmdat( struct mset ** mlist_ref) 
{
  struct mset * current = *mlist_ref;
  struct mset * next;

  while ( current != NULL ) {
    next = current->next;
    free(current);
    current = next;
  }  
  *mlist_ref = NULL;
}


void 
prtmdat( struct mset *mlist_p) 
{
  if ( mlist_p != NULL) {
    prtmdat(mlist_p->next);
    printf("prtmdat: meas_set->mtime: %f, meas_set->mval: %f\n", 
           mlist_p->mtime, mlist_p->mval);
  }
}

void
avgmdat( struct mset ** mlist_ref, 
         int sens_meas_no) 
{
  struct mset *llist_p = *mlist_ref;
  int count = 0;
  double avgtime = 0;
  double avgval  = 0;

  while ( llist_p != NULL) {
    count++;
    avgtime = avgtime + llist_p->mtime;
    avgval = avgval + llist_p->mval;
    llist_p = llist_p->next;
  }
  if ( count != 0 ) {
    avgtime = avgtime / count;
    avgval = avgval / count;
    printf( "avgmdat: sens_meas_no: %d, avgtime: %f, avgval: %f, number: %d\n", 
            sens_meas_no, avgtime, avgval, count); 
  }
}

int measval_db( char *sensorname, char *parametername, 
                   double mtime, long mval) 
{
  static struct mset *mlist_p[MAXSENSMEAS];
  static int cycleno[MAXSENSMEAS];
  int sensor_meas_no;

  if ( strcmp ( parametername, "POSPARAMETER") == 0 ) {
    sensor_meas_no = 0;
  } else if ( strcmp( parametername, "NEGPARAMETER") == 0 ) {
    sensor_meas_no = 1;
  }

  printf("measval_db: sensor_meas_no: %d mtime: %f mval: %ld\n", 
         sensor_meas_no,
         mtime,
         mval);
  printf("measval_db: cycleno[%d]: %d mcycle: %d\n", sensor_meas_no, cycleno[sensor_meas_no], mcycle);
  if ( cycleno[sensor_meas_no] < mcycle ) {
    printf("cycleno < mcycle: adding data\n");
    addmdat( &mlist_p[sensor_meas_no], mtime, mval);
    cycleno[sensor_meas_no]++;
  } else {
    printf("cycleno >= mcycle: averaging data\n");
    avgmdat( &mlist_p[sensor_meas_no], sensor_meas_no);
    rstmdat( &mlist_p[sensor_meas_no]);
    addmdat( &mlist_p[sensor_meas_no], mtime, mval);
    /*
       add code to write data to database
       datadb( sensor_meas_no, ... )

    */
    cycleno[sensor_meas_no] = 1;
  }
  prtmdat( mlist_p[sensor_meas_no]);
  return(0);

}


int 
main (int argc, char **argv)
{
  int i, tcycle;
  double mtime;
  long mval;
  struct timezone tz;
  struct timeval tv;

  tcycle = 20;
  for ( i = 0; i < tcycle; i++ ) {
    gettimeofday( &tv, &tz);
    mtime = tv.tv_sec+1.0e-6*tv.tv_usec; 
    mval  = random();
    printf("avgdata_tst: measurement cycle: %d : %f : %ld\n", i, mtime, mval);
    measval_db( "TESTSENSOR", "POSPARAMETER", mtime, mval);
    mval = -mval;
    printf("avgdata_tst: measurement cycle: %d : %f : %ld\n", i, mtime, mval);
    measval_db( "TESTSENSOR", "NEGPARAMETER", mtime, mval);
    sleep(1);
  }
  return(EXIT_SUCCESS);
}
