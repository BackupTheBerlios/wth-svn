/* 
   wth.h

   global header file for serial weatherstation communication
   for use of 

     WS2000
     PC weathersensor receiver
     1-wire weatherstation

   $Id: wth.h 177 2008-06-10 15:19:08Z vjahns $
   $Revision: 177 $

   Copyright 2009 Volker Jahns, <volker@thalreit.de>
 
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 
*/

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include <netdb.h>
#include <poll.h>	/* for convenience */
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>	/* for S_xxx file mode constants */
#include <sys/select.h>	/* for convenience */ 
#include <sys/socket.h>	/* basic socket definitions */
#include <sys/time.h>	/* timespec{} for pselect() */
#include <sys/uio.h>	/* for iovec{} and readv/writev */
#include <sys/un.h>	/* for Unix domain sockets */
#include <sys/wait.h>
#include <syslog.h>
#include <sqlite3.h>
#include <termios.h>
#include <time.h>	/* timespec{} for pselect() */
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <rrd.h>

#include "ownet.h"
#include "findtype.h"
#include "atod26.h"

#define VERSION     "0.5.0"
#define	MAXLINE     4096	
#define	MAXSOCKADDR 128
#define	BUFFSIZE    8192
#define MAXFD       64
#define MAXBUFF     131072
#define NBUFF       8192
#define SBUFF       1024
#define TBUFF       256
#define MAXSENSORS  24
#define MAXSENSMEAS 128
#define MAXPARAM    8
#define MAXDATA     256
#define PCWSRLEN    8
#define MAXMSGLEN   256
#define MAXQUERYLEN 1024

/* from unp.h */
#define LISTENQ     1024    /* 2nd argument to listen() */

/* Following shortens all the type casts of pointer arguments */
#define	SA	     struct sockaddr
#define max(a,b)     ((a) > (b) ? (a) : (b))

#define WS2000LOCK "/tmp/LCK...wth";

#define SBATTERY_FAM  0x26
#define VSENS 1
#define VACC  2

enum {
  SOH = 0x01,
  STX = 0x02,
  ETX = 0x03,
  EOT = 0x04,
  ENQ = 0x05,
  ACK = 0x06,
  DLE = 0x10,
  DC2 = 0x12,
  DC3 = 0x13,
  NAK = 0x15
};

/*
enum {
  FALSE = 0,
  TRUE  = 1
};
*/

/* weatherstation errors */
enum {
  ESERIAL = -2,
  ECHKSUM = -3,
  ECHKLEN = -4,
  ERCPT   = -5,
  ENET    = -6,
  ESIG    = -7,
  ECMD    = -8,
  EODB    = -9
};

/* sensor parameter assignment */
enum {
  SENSOR1TEMP      = 1,
  SENSOR1HUM       = 2,
  SENSOR2TEMP      = 3,
  SENSOR2HUM       = 4,
  SENSOR3TEMP      = 5,
  SENSOR3HUM       = 6,
  SENSOR4TEMP      = 7,
  SENSOR4HUM       = 8,
  SENSOR5TEMP      = 9,
  SENSOR5HUM       = 10,
  SENSOR6TEMP      = 11,
  SENSOR6HUM       = 12,
  SENSOR7TEMP      = 13,
  SENSOR7HUM       = 14,
  SENSOR8TEMP      = 15,
  SENSOR8HUM       = 16,
  RAINSENSOR       = 17,
  WINDSENSORSPEED  = 18,
  WINDSENSORDIR    = 19,
  WINDSENSORVAR    = 20,
  INDOORPRESS      = 21,
  INDOORTEMP       = 22,
  INDOORHUM        = 23
};

static const int success = 0;
static const int failure = 1;


typedef struct threadinfo {
  int num_active;
  pthread_cond_t thread_exit_cv;
  pthread_mutex_t mutex;
  int received_shutdown_req; /* 0=false, 1=true */
} thread_info_t;

thread_info_t pthread_info;

/* data structures */
typedef struct dataset {
  time_t dataset_time;
  float meas_value;
} dataset_t;

typedef struct param {
  char *paramname;
  float gain;
  float offset;
  dataset_t sample[MAXDATA];
} param_t;

typedef struct sensor {
  int status;
  param_t param[MAXPARAM];
  char sensorname[TBUFF+1];
  char description[TBUFF+1];
  char rrdfile[TBUFF+1];
  int address;
  char version[TBUFF+1];
  float updatefreq;
  time_t lastseen;
} sensor_t;

typedef struct senspar {
  int sensor_no;
  char *sensor_name;
  char *par_name;
} senspar_t;

typedef struct sensdevpar {
  int sensor_meas_no;
  char sensorname[TBUFF+1];
  char par_name[TBUFF+1];
  char devicetyp[TBUFF+1];
  char familycode[TBUFF+1];
  char serialnum[TBUFF+1];
} sensdevpar_t;

typedef struct ws2000stat {
  time_t interval;  /* internal measurement interval WS2000 PC interface */
  time_t lastread;  /* last status read date */
  int version;      /* internal version number */
  int DCFstat;      /* status of DCF receiver */
  int DCFsync;      /* sync bit of DCF receiver */
  time_t DCFtime;   /* DCF time */
  int HFstat;       /* HF status bit */
  int Battstat;     /* battery status */
  int numsens;      /* internal number of sensors */
  int ndats;        /* number of datasets retrieved */
  int is_present;   /* flag if station is present */
  char message[MAXMSGLEN]; /* holds ws2000 response message */
} ws2000stat_t;

typedef struct pcwsrstat {
  time_t lastread;  /* last status read date */
  int numsens;      /* internal number of sensors */
  int ndats;        /* number of datasets retrieved */
  int is_present;   /* flag if station is attached */
} pcwsrstat_t;

typedef struct onewirestat {
  time_t lastread;  /* last status read date */
  int numsens;      /* internal number of sensors */
  int ndats;        /* number of datasets retrieved */
  int is_present;   /* flag if station is attached */
} onewirestat_t;

typedef struct wmr9x8stat {
  time_t lastread;  /* last status read date */
  int numsens;      /* internal number of sensors */
  int ndats;        /* number of datasets retrieved */
  int is_present;   /* flag if station is attached */
  unsigned char wmrrecord[MAXMSGLEN]; /* holds last wmr9x8 record */
} wmr9x8stat_t;

typedef struct wsconf {
  int mcycle;
  char dbfile[TBUFF+1];
  char device[TBUFF+1];
  char dbpath[SBUFF+1];
  char rrdpath[SBUFF+1]; 
  char monitor[TBUFF+1];
} wsconf_t;

typedef struct ws2000 {
  ws2000stat_t status;
  wsconf_t config;
  sensor_t sensor[MAXSENSORS];
} ws2000_t;

typedef struct pcwsr {
  pcwsrstat_t status;
  wsconf_t config;
  sensor_t sensor[MAXSENSORS];
} pcwsr_t;

typedef struct onewire {
  onewirestat_t status;
  wsconf_t config;
  sensor_t sensor[MAXSENSORS];
} onewire_t;

typedef struct wmr9x8 {
  wmr9x8stat_t status;
  wsconf_t config;
  sensor_t sensor[MAXSENSORS];
} wmr9x8_t;

typedef struct conf {
  int command;
  int argcmd;
  int debug;
  int verbose;
  int timeout;
  int logfacility;
  char *hostname;
  char *configfile;
  char *port;
  char *units;
  char *outfmt;
} conf_t;

typedef void Sigfunc ( int);

typedef struct key {
  char *word;
  int id;
  char *descr;
} ws2000key_t;

int werrno;
int daemon_proc;          /* set nonzero by daemon_init() */

sqlite3 *ws2000db;
sqlite3 *pcwsrdb;
sqlite3 *onewiredb;
sqlite3 *wmr9x8db;

ws2000_t  ws2000station;
pcwsr_t   pcwsrstation;
onewire_t onewirestation;
wmr9x8_t wmr9x8station;
conf_t   wsconf;

int wthd_init();
int echodata( unsigned char *data, int mdat);
unsigned char getbits( unsigned char x, int p, int n);
char *mkmsg( const char *, ...);
char *mkmsg2( const char *, ...);
int usage (int exitcode, char *error, char *addl);
int usaged (int exitcode, char *error, char *addl);
int readconfig();
char *echoconfig( char *station);
Sigfunc *signal(int signo, Sigfunc *func);
void *signal_hd( void *arg);

extern int Socket(int, int, int); 
ssize_t Read(int, void *, size_t); 
int Write(int, void *, size_t);   
int Accept(int, SA *, socklen_t *);
int Bind(int, const SA *, socklen_t);
int Listen(int, int); 
int Close(int); 
pid_t Fork(void); 
int Setsockopt(int, int, int, const void *, socklen_t); 
int Writen(int, void *, size_t);  
const char *inet_ntop(int, const void *, char *, size_t);
int inet_pton(int, const char *, void *); 
int daemon_init( );

void *pcwsr_hd( void *arg);
void *ws2000_hd( void *arg);
void *onewire_hd( void *arg);
void *wmr9x8_hd( void *arg);
void *cmd_hd( void *arg);
int docmd( int sockfd);

int demasq( unsigned char *data, int *mdat);
int chkframe( unsigned char *data, int *mdat);
int datex( unsigned char *data, int ndat);
int getcd(unsigned char *data, int *mdat);
int getrd(unsigned char *data, int *mdat);
int getsrd(unsigned char *data, int *mdat);

char *wstat(unsigned char *data, int mdat);
time_t dcftime(unsigned char *data, int ndat);
int settime();
int wcmd();
int readdata( int fd, unsigned char *data, int *ndat);
ws2000key_t *c( int n);
int wstrlen( unsigned char *s);

char *tnstat( char *station);
char *tnusage (int exitcode, char *error, char *addl);
char *helpread (int exitcode, char *error, char *addl);
char *tnhelp( char *args);
char *execmd( char *args);
char *showcmd( char *args);
char *initcmd (char *args);

int datadb( long dataset_date, int sensor_param, float meas_value,
  sqlite3 *pcwsrdb);
int statdb( int sensor_status[], time_t statusset_date );
int newdb( long statusset_date, int sensor_no, int new_flag);
int writedb( int sensor_no, int nval, int sensor_meas_no[], 
      time_t dataset_date,
      float meas_value[] );
int senspardb( int sensor_meas_no, senspar_t *sspar, sqlite3 *wthdb);
int sensdevpar( char *parname, char *serialnum, sensdevpar_t *ssdp, sqlite3 *wthdb);
char *readdb( char *wstation);
int readpar( time_t *meastim, float *measval, 
      int sensor_no, int sensor_meas_no, time_t timedif, char *wstation);
char *readstat( char *wstation);
int maxsensmeas( sqlite3 *onewiredb);

char *ppagemem( uchar *pagemen);
int bitprint( int byte, char *s_reg);
int longprint( int byte, char *s_reg);
char *echo_serialnum( uchar *serialnum);
char *echo_familycode( uchar *serialnum);

int initwmr9x8 (int *pfd, struct termios *newtio, struct termios *oldtio);
int closewmr9x8( int fd, struct termios *oldtio);
int shuffdat( unsigned char *data, int ndat);
int getschar (int fd, unsigned char *schar);
int wmr9x8dac( unsigned char *data, int ndat);
int wind_rd( unsigned char *data);
int rain_rd( unsigned char *data);
int thin_rd( unsigned char *data);
int thout_rd( unsigned char *data);
int thb_rd( unsigned char *data);
int thbnew_rd( unsigned char *data);
int wmr9x8rd( int rfd);
