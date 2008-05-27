/* wth.h

   global header file for WS2000 weatherstation communication
   
   $Id: wth.h,v 1.1 2002/07/04 09:50:21 jahns Exp jahns $
   $Revision: 1.1 $

   Copyright (C) 2000-2001,2005 Volker Jahns <Volker.Jahns@thalreit.de>

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

#if defined POSTGRES
  #include <pgsql/libpq-fe.h>
#endif

#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define BAUDRATE    B9600
#define MAXBUFF     131072
#define MAXMSGLEN   1024
#define TIMEOUT     7
/* Serial port devices */
#if defined(LINUX)
# define SER_DEVICE "/dev/ttyUSB0"
#elif defined(FREEBSD)
# define SER_DEVICE "/dev/ttyd0"
#else // default
# define SER_DEVICE ""
#endif
#define MAXSENSORS  42
#define MAXDATA     2048
#define LOGFACILITY LOG_LOCAL7
#define WPORT       "5001"
#define TNPORT      "5002"
#define XMLPORT     "8005"
#define MAXFD       64
#define XMLNAME     "XML-RPC Weatherstation C Client"
#define WSTYPE      "WS2000"
#define DATABASE    "weather"
#define DBUSER      "postgres"
#define UNITS       "SI"
#define OUTFMT      "old"

/* from unp.h */
#define	MAXLINE      4096	/* max text line length */
#define	MAXSOCKADDR  128	/* max socket address structure size */
#define	BUFFSIZE     8192	/* buffer size for reads and writes */
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
struct dataset {
  time_t time;
  long age;
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
  struct sensor sens[MAXSENSORS];
  struct DCFstruct DCF;
  struct wstatus wstat;
};  

struct cmd {
  int command;
  int argcmd;
  int netflg;
  int verbose;
  int timeout;
  int logfacility;
  char *hostname;
  char *port;
  char *tnport;
  char *xmlport;
  char *database;
  char *dbuser;
  char *wstype;
  char *device;
  int baudrate;
  int cstopb;
  int nbits;
  int parity;
  char *units;
  char *outfmt;
  char *rrdfile;
};

struct key {
  char *word;
  int id;
  char *descr;
};


typedef struct key Ckey;

/* global variables */
int werrno;      /* weatherstation errors */
int daemon_proc;		/* set nonzero by daemon_init() */

int daemon_init(const char *, int); 

char *tnusage(int exitcode, char *error, char *addl);
int usage(int exitcode, char *error, char *addl);
int usaged(int exitcode, char *error, char *addl);
char *readconfig(struct cmd *pcmd);
char *echoconfig(struct cmd *pcmd);

int getxmlrd(unsigned char *data, int *mdat, struct cmd *);
int getnrd(unsigned char *data, int *mdat, struct cmd *);
int getsrd(unsigned char *data, int *mdat, struct cmd *pcmd);

int getcd(unsigned char *data, int *mdat, struct cmd *pcmd);
int getrd(unsigned char *data, int *mdat, struct cmd *pcmd);
char *wstat(unsigned char *data, int mdat, struct wthio *rw);
time_t dcftime(unsigned char *data, int ndat);
int settime(struct wthio *rw);
char *wcmd(struct cmd *pcmd, struct wthio *rw);

int initdata(struct wthio *rw);
struct cmd *initcmd(void );

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

typedef void Sigfunc ( int);  
Sigfunc *signal( int signo, Sigfunc *func);

int initserial( int *pfd, struct termios *newtio, 
		struct termios *oldtio, struct cmd *pcmd);
int closeserial( int fd, struct termios *oldtio);
int readdata( int fd, unsigned char *data, 
  int *ndat, struct cmd *pcmd);

Ckey *c( int n);
int wstrlen( unsigned char *s);

unsigned char getbits( unsigned char x, int p, int n);
char *mkmsg( const char *, ...);

int demasq( unsigned char *data, int *mdat);
int chkframe( unsigned char *data, int *mdat, struct cmd *pcmd);
int datex( unsigned char *data, int ndat, struct wthio *rw, struct cmd *pcmd);
char *pdata( struct wthio *rw, struct cmd *pcmd );
int echodata( unsigned char *data, int mdat);
