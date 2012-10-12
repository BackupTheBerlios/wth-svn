/* 
   wth.h

   global header file for serial port communication with 

     Ultimeter weatherstation
     1-Wire weatherstation
     WMR 9X8 weatherstation
     WS2000 weatherstation
     PC weathersensor receiver

   $Id$
   $Revision$

   Copyright 2009-2012 Volker Jahns, <volker@thalreit.de>
 
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
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/resource.h>
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
#include <pgsql/libpq-fe.h>
#include <termios.h>
#include <time.h>	/* timespec{} for pselect() */
#include <unistd.h>
#include <math.h>
#include <pthread.h>


#define VERSION         "0.5.0"
#define	MAXLINE         4096	
#define	BUFFSIZE        8192
#define MAXFD           64
#define NBUFF           8192
#define SBUFF           1024
#define TBUFF           256
#define MAXSENSMEAS     128
#define MAXMSGLEN       256
#define MAXQUERYLEN     1024
#define NOSENSORPARAM  -1
#define FALSE          0
#define TRUE           1


#define max(a,b)     ((a) > (b) ? (a) : (b))


/* weatherstation type */
enum {
  UMETER  = 1,
  ONEWIRE = 2,
  WMR9X8  = 3,
  WS2000  = 4,
  PCWSR   = 5
};

enum {
  POSTGRESQL = 1,
  MYSQL      = 2, 
  SQLITE     = 3,
  ORACLE     = 4
};

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

static const int success = 0;
static const int failure = 1;

struct mset {
  double mtime;
  float mval;
  struct mset *next;
};

struct sset {
  double stime;
  long unsigned int sval;
  struct sset *next;
};

typedef struct sensdevpar {
  int sensor_meas_no;
  char sensorname[TBUFF+1];
  char par_name[TBUFF+1];
  char devicetyp[TBUFF+1];
  char familycode[TBUFF+1];
  char serialnum[TBUFF+1];
  float offset;
  float gain;
} sensdevpar_t;

typedef struct sensflag {
  int sensor_flag_no;
  char sensorname[TBUFF+1];
  char flagname[TBUFF+1];
  char devicetyp[TBUFF+1];
  char familycode[TBUFF+1];
  char serialnum[TBUFF+1];
} sensflag_t;




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
  unsigned char is_present;   /* flag if station is present */
  char message[MAXMSGLEN]; /* holds ws2000 response message */
} ws2000stat_t;

typedef struct pcwsrstat {
  time_t lastread;  /* last status read date */
  int numsens;      /* internal number of sensors */
  int ndats;        /* number of datasets retrieved */
  unsigned char is_present;   /* flag if station is attached */
} pcwsrstat_t;

typedef struct onewirestat {
  time_t lastread;  /* last status read date */
  int numsens;      /* internal number of sensors */
  int ndats;        /* number of datasets retrieved */
  unsigned char is_present;   /* flag if station is attached */
} onewirestat_t;

typedef struct wmr9x8stat {
  time_t lastread;  /* last status read date */
  int numsens;      /* internal number of sensors */
  int ndats;        /* number of datasets retrieved */
  unsigned char is_present;   /* flag if station is attached */
  unsigned char wmrrecord[MAXMSGLEN]; /* holds last wmr9x8 record */
} wmr9x8stat_t;

typedef struct umeterstat {
  time_t lastread;  /* last status read date */
  int numsens;      /* internal number of sensors */
  int ndats;        /* number of datasets retrieved */
  unsigned char is_present;   /* flag if station is attached */
  unsigned char umrecord[MAXMSGLEN]; /* holds last ultimeter record */
} umeterstat_t;

typedef struct wsconf {
  int mcycle;
  int dbtype;
  char dbfile[TBUFF+1];
  char dbconn[TBUFF+1];
  char device[TBUFF+1];
  char dbpath[SBUFF+1];
  char monitor[TBUFF+1];
} wsconf_t;

typedef struct ws2000 {
  ws2000stat_t status;
  wsconf_t config;
} ws2000_t;

typedef struct pcwsr {
  pcwsrstat_t status;
  wsconf_t config;
} pcwsr_t;

typedef struct onewire {
  onewirestat_t status;
  wsconf_t config;
} onewire_t;

typedef struct wmr9x8 {
  wmr9x8stat_t status;
  wsconf_t config;
} wmr9x8_t;

typedef struct umeter {
  umeterstat_t status;
  wsconf_t config;
} umeter_t;

typedef struct conf {
  int command;
  int argcmd;
  int debug;
  int verbose;
  int timeout;
  int elevation;
  int log_facility;
  char *hostname;
  char *configfile;
  char *units;
  char *outfmt;
} conf_t;

typedef void Sigfunc ( int);

typedef struct key {
  char *word;
  int id;
  char *descr;
} ws2000key_t;


typedef struct threadinfo {
  int num_active;
  pthread_cond_t thread_exit_cv;
  pthread_mutex_t mutex;
  int received_shutdown_req; /* 0=false, 1=true */
} thread_info_t;

thread_info_t pthread_info;



int werrno;
int daemon_proc;          /* set nonzero by daemon_init() */

sqlite3 *ws2000db;
sqlite3 *pcwsrdb;
sqlite3 *onewiredb;
sqlite3 *wmr9x8db;
sqlite3 *umeterdb;

PGconn  *pg_conn;

ws2000_t  ws2000station;
pcwsr_t   pcwsrstation;
onewire_t onewirestation;
wmr9x8_t  wmr9x8station;
umeter_t  umeterstation;

conf_t   wsconf;

int wthd_init();
int echodata( unsigned char *data, int mdat);
unsigned char getbits( unsigned char x, int p, int n);
unsigned char lownibble(unsigned char x);
unsigned char highnibble(unsigned char x);
char *mkmsg( const char *, ...);
char *mkmsg2( const char *, ...);
int usage (int exitcode, char *error, char *addl);
int usaged (int exitcode, char *error, char *addl);
int readconfig();
char *echoconfig( char *station);
Sigfunc *signal(int signo, Sigfunc *func);
void *signal_hd( void *arg);
pid_t Fork(void);
int daemon_init(void );

void *pcwsr_hd( void *arg);
void *ws2000_hd( void *arg);
void *onewire_hd( void *arg);
void *wmr9x8_hd( void *arg);


int bitprint( int byte, char *s_reg);
int longprint( int byte, char *s_reg);
int maxsensmeas( int dbtype);

sensdevpar_t get_sensorparams( 
                char *sensorname, char *parametername,
                int stationtype, int dbtype);
 
int measval_hd( char *sensorname, char *parametername,
                int stationtype, int dbtype, 
                double mtime, float mval);

int statval_hd( char *sensorname, char *flagname,
                int stationtype, int dbtype, 
                double stime, long unsigned int sval);

/* sqlite database functions */
int sqlite_datadb( long dataset_date, int sensor_param, float meas_value,
  sqlite3 *pcwsrdb);
int stat_ws2000db( int sensor_status[], time_t statusset_date, sqlite3 *ws2000db);
int new_ws2000db( long statusset_date, int sensor_no, int new_flag, 
      sqlite3 *ws2000db);
int sqlite_writedb( int sensor_no, int nval, int sensor_meas_no[], 
      time_t dataset_date,
      float meas_value[], sqlite3 *ws2000db );
int sqlite_get_onewireinfo( char *parname, char *serialnum, sensdevpar_t *ssdp, 
      sqlite3 *wthdb);
int is_ws2000sens( int sensor_no, sqlite3 *ws2000db);
int readpar( time_t *meastim, float *measval, 
      int sensor_no, int sensor_meas_no, time_t timedif, char *wstation);
int sqlite_maxsensmeas( sqlite3 *onewiredb);

sensdevpar_t
sqlite_get_sensorparams( char *sensorname, char*parametername,
                         int stationtype);
sensflag_t
sqlite_get_sensorflags( char *sensorname, char *flagname,
                         int stationtype);
int sqlite_datadbn( long dataset_date, int sensor_param, 
                    float meas_value, int stationtype);

int measval_db( char *sensorname, char *parametername, 
      time_t dataset_date, float mval, sqlite3 *database);
int statval_db( char *sensorname, char *statusname, 
      time_t dataset_date, long unsigned int sval, sqlite3 *database);

/* postgresql database functions */
int pg_datadb( long dataset_date, int sensor_param, float meas_value,
      PGconn *pg_conn);
int pg_stat_ws2000db( int sensor_status[], time_t statusset_date, 
      PGconn *pg_conn);
int pg_new_ws2000db( long statusset_date, int sensor_no, int new_flag, 
      PGconn *pg_conn);
int pg_writedb( int sensor_no, int nval, int sensor_meas_no[], 
      time_t dataset_date, float meas_value[], PGconn *pg_conn );
int pg_get_onewireinfo( char *parname, char *serialnum, sensdevpar_t *ssdp,
      PGconn *pg_conn);
int pg_is_ws2000sens( int sensor_no, PGconn *pg_conn);
int pg_readpar( time_t *meastim, float *measval, 
      int sensor_no, int sensor_meas_no, time_t timedif, char *wstation);
int pg_maxsensmeas( PGconn *pg_conn);


void *umeter_hd( void *arg);
int datalogger_rd( unsigned char * datalogdata, int ndat);
int packet_rd( unsigned char * packetdata, int ndat);
int complete_rd( unsigned char * completedata, int ndat);
int umeter_rd();
int initumeter ( int *pfd, struct termios *newtio,struct termios *oldtio);
int closeumeter( int fd, struct termios *oldtio);

