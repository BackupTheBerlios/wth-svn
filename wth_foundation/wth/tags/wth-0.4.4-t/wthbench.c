/*

  C code to benchmark data consumption and read/write perfomance of 
  weather data sets


  $Id: wthbench.c,v 1.2 2002/07/04 09:51:42 jahns Exp $

*/

#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXDATA 1024
#define MAXSENSOR 8


typedef struct status_tag {
  int batteryStat;
  int alarmSet;
  int receiveErr;
} status_typ;

typedef struct temphum_tag {
  int sensorID;
  float average, high, low, dewpoint;
  status_typ Status;
} temp_typ, hum_typ;

typedef struct press_tag {
  int sensorID;
  float absolute, sealevel;
  int tendency;
  status_typ Status;
} press_typ;

typedef struct wind_tag {
  int sensorID;
  float avgSpeed, avgDirection, avgMeanDeviation;
  float gustSpeed, gustDirection;
  float lowSpeed, lowDirection;
  float highSpeed, highDirection;
  float chill;
  status_typ Status;
} wind_typ;

typedef struct rain_tag {
  int sensorID;
  float total, rate, yesterday;
  status_typ Status;
} rain_typ;

/* fixed size for sensors */

typedef struct wreport_tag {
  time_t reportTime;
  char *units;
  int stationID;
  char *stationTyp;
  /* fixed size w/ MAXSENSOR 8: sizeof struct wreport_typ : 1936Byte */
  /*
  temp_typ outdoorT[MAXSENSOR];
  hum_typ  outdoorH[MAXSENSOR];
  temp_typ indoorT[MAXSENSOR];
  hum_typ  indoorH[MAXSENSOR];
  press_typ baroP[MAXSENSOR];
  wind_typ  Wind[MAXSENSOR];
  rain_typ  Rain[MAXSENSOR];  
  */
  /* pointers : size of struct wreport_typ: 44 Bytes */
  temp_typ *outdoorT;
  hum_typ  *outdoorH;
  temp_typ *indoorT;
  hum_typ  *indoorH;
  press_typ *baroP;
  wind_typ  *Wind;
  rain_typ  *Rain;  
 
} wreport_typ;



int
main ( int argc, char **argv) {
  int i, fd;
  wreport_typ WReport[MAXDATA];

  /* open file, write data, then close */
  fd = open("test.db", O_TRUNC | O_CREAT | O_WRONLY, 0644);
  
  WReport[0].outdoorT = (temp_typ *) malloc(sizeof(temp_typ));

  WReport[0].outdoorT[0].sensorID = 0;
  WReport[0].outdoorT[0].average  = 22.9;
  WReport[0].outdoorT[0].high     = 31.3;
  WReport[0].outdoorT[0].low      = 14.5;
  WReport[0].outdoorT[0].dewpoint = 20.1;
  WReport[0].outdoorT[0].Status.batteryStat = 1;
  WReport[0].outdoorT[0].Status.alarmSet    = 0;
  WReport[0].outdoorT[0].Status.receiveErr  = 3;

  printf("wthbench: sizeof(WReport): %d\n", sizeof(WReport[0]));
  
  for ( i = 0; i < MAXDATA; i++) {
    if ( write(fd, &WReport[0], sizeof(WReport[0])) != sizeof(WReport[0]))
      perror("write WReport record");;
  }

  close(fd);

  /* open file, read data, then close */
  fd = open("test.db", O_RDONLY, 0644);
  for ( i = 0; i < MAXDATA; i++) {
    if ( read(fd, &WReport[0], sizeof(WReport[0])) != sizeof(WReport[0]))
      perror("read WReport record");;
  }

  close(fd);
  memset(WReport[0].outdoorT, 0, sizeof(temp_typ));

  /* echo data */
  printf("sensorID: %d\n",WReport[0].outdoorT[0].sensorID);
  printf("average:  %f\n",WReport[0].outdoorT[0].average);
  printf("high:     %f\n",WReport[0].outdoorT[0].high);
  printf("low:      %f\n",WReport[0].outdoorT[0].low);
  printf("dewpoint: %f\n",WReport[0].outdoorT[0].dewpoint);
  printf("battery:  %d\n",WReport[0].outdoorT[0].Status.batteryStat);
  printf("alarm:    %d\n",WReport[0].outdoorT[0].Status.alarmSet);
  printf("recverr:  %d\n",WReport[0].outdoorT[0].Status.receiveErr);

  return 0;
}


