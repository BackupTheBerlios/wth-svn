/* wth.h

   global header file for serial weatherstation communication
   for use of 

     WS2000
     PC weathersensor receiver
     1-wire weatherstation
 
   $Id$
   $Revision$

   Copyright (C) 2000-2001,2005,2007 Volker Jahns <Volker.Jahns@thalreit.de>

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
#include <strings.h>	/* for convenience */
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
#include <termios.h>
#include <time.h>	/* timespec{} for pselect() */
#include <unistd.h>
#include <math.h>

#define VERSION     "0.5.0"
#define	MAXLINE     4096	
#define	MAXSOCKADDR 128
#define	BUFFSIZE    8192
#define MAXFD       64
#define MAXBUFF     131072
#define MAXSENSORS  42
#define MAXPARAM    8
#define MAXDATA     2048
#define PCWSRLEN    8

/* Serial port devices */
#if defined(LINUX)
# define SER_DEVICE "/dev/ttyS0"
#elif defined(FREEBSD)
# define SER_DEVICE "/dev/ttyd0"
#else // default
# define SER_DEVICE ""
#endif

/* from unp.h */
#define LISTENQ      1024    /* 2nd argument to listen() */

/* Following shortens all the type casts of pointer arguments */
#define	SA	     struct sockaddr
#define max(a,b)     ((a) > (b) ? (a) : (b))


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

enum {
  FALSE = 0,
  TRUE  = 1
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
};


/* data structures */
typedef struct dataset {
  time_t dataset_time;
  float meas_value;
} dataset_t;

typedef struct param {
  char *paramname;
  dataset_t sample[MAXDATA];
} param_t;

typedef struct sensor {
  int status;
  param_t param[MAXPARAM];
  char *sensorname;
  char *rrdfile;
  int address;
  char *version;
  float updatefreq;
  time_t lastseen;
} sensor_t;

typedef struct ws2000stat {
  time_t interval;  /* internal measurement interval of WS2000 PC interface */
  char *version;    /* internal version number */
  int DCFstat;      /* status of DCF receiver */
  int DCFsync;      /* sync bit of DCF receiver */
  time_t DCFtime;   /* DCF time */
  int HFstat;       /* HF status bit */
  int Battstat;     /* battery status */
  int numsens;      /* internal number of sensors */
  int ndats;        /* number of datasets retrieved */
} ws2000stat_t;

typedef struct wsconf {
  char *dbfile;
  char *device;
} wsconf_t;

typedef struct ws2000 {
  ws2000stat_t status;
  wsconf_t config;
  sensor_t sensor[MAXSENSORS];
} ws2000_t;

typedef struct pcwsr {
  wsconf_t config;
  sensor_t sensor[MAXSENSORS];
} pcwsr_t;

typedef struct conf {
  int command;
  int argcmd;
  int netflg;
  int debug;
  int verbose;
  int timeout;
  int logfacility;
  char *hostname;
  char *port;
  char *tnport;
  char *xmlport;
  char *wwwport;
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
int initdata();
char *echoconfig();
int echodata( unsigned char *data, int mdat);
unsigned char getbits( unsigned char x, int p, int n);
char *mkmsg( const char *, ...);

int chklockf( const char *lockfile);
int setlck( const char *lockfile);
int unlck( const char *lockfile, int fd);

/* pcwsr functions */
int pcwsr_loghandler();

/* ws2000 functions */
int ws2000_loghandler();
int ws2000_cmdhandler();
int demasq( unsigned char *data, int *mdat);
int chkframe( unsigned char *data, int *mdat);
int datex( unsigned char *data, int ndat);
int getcd(unsigned char *data, int *mdat);
int getrd(unsigned char *data, int *mdat);
int getnrd(unsigned char *data, int *mdat);
int getsrd(unsigned char *data, int *mdat);

char *wstat(unsigned char *data, int mdat);
time_t dcftime(unsigned char *data, int ndat);
int settime();
char *wcmd();
int readdata( int fd, unsigned char *data, int *ndat);
ws2000key_t *c( int n);
int wstrlen( unsigned char *s);

ws2000_t ws2000station;
pcwsr_t  pcwsrstation;
conf_t   wsconf;
