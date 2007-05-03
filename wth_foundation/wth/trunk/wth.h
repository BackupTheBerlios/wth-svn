/* wth.h

   global header file 
   includes, defines, data structures, function prototypes
   for WS2000 weatherstation communication
   
   $Id: wth.h,v 0.2 2001/03/01 06:32:18 volker Exp $
   $Revision: 0.2 $

*/

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include <netdb.h>
#include <poll.h>		/* for convenience */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>	/* for convenience */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>	/* for S_xxx file mode constants */
#include <sys/select.h>	/* for convenience */
#include <sys/socket.h>	/* basic socket definitions */
#include <sys/uio.h>	/* for iovec{} and readv/writev */
#include <sys/un.h>		/* for Unix domain sockets */
#include <sys/wait.h>
#include <syslog.h>
#include <termios.h>
#include <time.h>		/* timespec{} for pselect() */
#include <unistd.h>


#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define BAUDRATE B9600
#define MAXBUFF 255
#define TIMEOUT 7
#define SER_DEVICE "/dev/ttyd0"
#define MAXSENSORS 36
#define MAXDATA 2048
#define LOGFACILITY LOG_LOCAL7
#define WPORT "3963"
#define MAXFD   64



/* from unp.h */
#define	MAXLINE		4096	/* max text line length */
#define	MAXSOCKADDR  128	/* max socket address structure size */
#define	BUFFSIZE	8192	/* buffer size for reads and writes */
#define LISTENQ         1024            /* 2nd argument to listen() */
/* Following shortens all the type casts of pointer arguments */
#define	SA	struct sockaddr



#define SOH 0x01
#define STX 0x02
#define ETX 0x03
#define EOT 0x04
#define ENQ 0x05
#define ACK 0x06
#define DLE 0x10
#define DC2 0x12
#define DC3 0x13
#define NAK 0x15

#define FALSE 0
#define TRUE 1


/* data structures */
struct DCFstruct {
    int stat;
    int sync;
    time_t time;
};

struct wstatus {
    int numsens;
    int intv;
    int version;
};

struct dataset {
    time_t time;
    int sign;
    int hundreds;
    int tens;
    int units;
    int tenths;
};

struct sensor {
    int status;
    char *type;
    struct dataset mess[MAXDATA];
};

struct cmd {
    int command;
    int argcmd;
    int netflg;
    char *hostname;
    char *port;
};

typedef void Sigfunc (int);  


/* function prototypes */
int Socket(int, int, int); 
ssize_t Read(int, void *, size_t); 
void Write(int, void *, size_t);   
void Bind(int, const SA *, socklen_t);
void Listen(int, int); 
void Close(int); 
void daemon_init(const char *, int); 

pid_t Fork(void); 
void Setsockopt(int, int, int, const void *, socklen_t); 
void Writen(int, void *, size_t);  
ssize_t Readline(int, void *, size_t);  

void err_dump(const char *, ...);
void err_msg(const char *, ...);
void err_quit(const char *, ...);
void err_ret(const char *, ...);
void err_sys(const char *, ...);

void signal_handler( int signum);
int initserial ( int *pfd, struct termios *newtio, struct termios *oldtio);
int closeserial( int fd, struct termios *oldtio);
int readdata ( int fd, char *data, int *ndat);
char *cword( int n);
int getsrd (char *data, int *mdat, struct cmd *pcmd);

unsigned getbits(unsigned x, int p, int n);
int echodata(u_char *data, int mdat);
int wstrlen(char *s);
int usage(int exitcode, char *error, char *addl);
int usaged(int exitcode, char *error, char *addl);
Sigfunc *signal(int signo, Sigfunc *func);
Sigfunc *Signal(int signo, Sigfunc *func);	/* for our signal() function */

const char *inet_ntop(int, const void *, char *, size_t);
int inet_pton(int, const char *, void *); 
int getnrd(char *data, int *mdat, struct cmd *);

int datacorr(u_char *data, int *mdat);
int chkframe(u_char *data, int *mdat);
int getcd(char *data, int *mdat, struct cmd *pcmd);
int getrd(char *data, int *mdat, struct cmd *pcmd);
int wstat(char *data, int mdat, struct DCFstruct *DCF, struct sensor sens[], struct wstatus *setting);
int dcftime(u_char *data, int ndat);
int getone(char *data, int ndat, struct sensor sens[], long setno, struct DCFstruct DCF);
int pdata(struct sensor sens[], int snum);
int wcmd (struct sensor sens[], struct cmd *pcmd, int *setno);
int initsens(struct sensor sens[]);
