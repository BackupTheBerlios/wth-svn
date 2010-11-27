/* 
  wmr9x8_test.c

  testing serial I/O with wmr9x8 weatherstation

  compile command
  gcc -Wall -O0 -g -o wmr9x8_test wmr9x8_test.c   

  $Id$
  Volker Jahns, volker@thalreit.de

*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <syslog.h>
#include <time.h>

#define BENCHMARK 1
#define TBUFF 256

#define WINDLEN 11
#define RAINLEN 16
#define THINLEN 9
#define THOUTLEN 9
#define TINLEN 7
#define THBLEN 13
#define THBNEWLEN 14
#define MINLEN 5
#define CLOCKLEN 9

#define WINDTYP   0x00
#define RAINTYP   0x01
#define THINTYP   0x02
#define THOUTTYP  0x03
#define TINTYP    0x04
#define THBTYP    0x05
#define THBNEWTYP 0x06
#define MINTYP    0x0e
#define CLOCKTYP  0x0f


/*
  bitprint 

  utility function to print bits of 8-bit byte
*/
int 
bitprint ( int byte, char *s_reg ) {
  int x;
  struct timezone tz;
  struct timeval  tv;

  gettimeofday(&tv, &tz);
  printf("%5s | %lu.%lu : #", s_reg, ( long unsigned int)tv.tv_sec, tv.tv_usec);
  for( x = 7; x > -1; x-- )
    printf( "%i", (byte & 1 << x ) > 0 ? 1 : 0 );
  printf( "b\n" );
  return(0);
}

/* echodata
  
   print out raw data frame reveived from weather station 

*/
int
echodata(unsigned char *data, int mdat) {
    int i;
    char frame[TBUFF+1];
    char sf[5];

    syslog(LOG_DEBUG, "echodata : length dataframe : %d\n",mdat);
    memset( frame, 0, TBUFF);
    memset( sf, 0, 3);
    /* for better readability label each byte in frame */  
    for ( i = 0; i < mdat; i++ ) {
      snprintf(sf, 4, "%02d:",i);
      strncat(frame, sf, 3);
    }
    syslog(LOG_DEBUG, "echodata : %s\n", frame);    

    memset(frame, 0, TBUFF);

    for ( i = 0; i < mdat; i++ ) {
      snprintf(sf, 4, "%02x:",data[i]);
      strncat(frame, sf, 3);
    }

    syslog(LOG_DEBUG, "echodata : %s\n", frame);   

    return(0);
} 


int 
getschar (int fd, unsigned char *schar) 
{
  int err = -1;
  unsigned char rbyte;

  int maxfd = fd + 1;
  fd_set readfs;
  struct timeval tv;

  tv.tv_sec  = 7;
  tv.tv_usec = 0;

  FD_ZERO(&readfs);
  FD_SET(fd, &readfs);

  err = select(maxfd, &readfs, NULL, NULL, &tv);

  if ( err < 0 ) {
    if ( errno == EBADF ) 
      perror("readdata: select error: EBADF");
    else if ( errno == EINTR ) 
      perror("readdata: select error: EINTR");
    else if ( errno == EINVAL ) 
      perror("readdata: select error: EINVAL");
    else
      perror("readdata: select error");
  }

  if ( err && FD_ISSET (fd, &readfs)) {

    if ((err = read (fd, &rbyte, 1)) < 0)  { 
      printf ("reading device failed \n");
      return (err);
    }
    else if (err == 1) {
      *schar = rbyte;
      return( err);
    }

  }

  return( err);
}

int
shuffdat( unsigned char *data, int ndat) {
  int i;
  unsigned char tdat[TBUFF+1]; 
  tdat[0] = 0xff;
  tdat[1] = 0xff;

  for ( i = 0; i < ndat-2; i++) {
    tdat[i+2] = data[i];
  }

  for ( i = 0; i < ndat; i++) 
    data[i] = tdat[i];

  return(0);
}

/*
  wmr9x8rd

  reading data records from serial port
  data acquisition to database

*/
int 
wmr9x8rd( int rfd) {
  int i;
  int err = -1;
  int sync = 0;
  int ndat = 0;
  int dtyp = -1;
  unsigned char schr = 0x00;
  unsigned char data[TBUFF+1];

  memset( data, 0 , TBUFF);
  for (;;) {

    err = getschar( rfd, &schr);
    if ( err == 1) {
      data[ndat] = schr;
      ndat++;
    } else {
      syslog(LOG_DEBUG, "wmr9x8rd: could not read 1 char err: %d\n", err);
    }

    if ( schr == 0xff) {
      sync++;
    }

    if (sync == 2) {
      if (( schr == 0x00) || ( schr == 0x01 ) || (schr == 0x02) || (schr == 0x03) || (schr == 0x05) || ( schr == 0x0e) || ( schr == 0x0f) || ( schr == 0x06)) {
        syslog( LOG_DEBUG,"devicetyp: %x", schr);
	dtyp = schr;
	sync++;
      }
    }
    
    if ( sync == 3) {
      syslog(LOG_DEBUG, "wmr9x8rd: header sync + devicetype %x received", dtyp);
      data[0] = 0xff;
      data[1] = 0xff;
      data[2] = dtyp;

      if ( dtyp == WINDTYP ) {
	syslog(LOG_DEBUG, "wmr9x8rd: devicetyp is WINDTYP");
	for ( i = 0; i < WINDLEN - 3 ; i++) {
	  err = getschar( rfd, &schr);
          data[i+3] = schr;
	}
	ndat = WINDLEN;

      } else if ( dtyp == RAINTYP ) {
	syslog(LOG_DEBUG, "wmr9x8rd: devicetyp is RAINTYP");
	for ( i = 0; i < RAINLEN - 3; i++) {
	  err = getschar( rfd, &schr);
          data[i+3] = schr;
	}
	ndat = RAINLEN;

      } else if ( dtyp == THINTYP ) {
	syslog(LOG_DEBUG, "wmr9x8rd: devicetyp is THINTYP");
	for ( i = 0; i < THINLEN - 3; i++) {
	  err = getschar( rfd, &schr);
          data[i+3] = schr;
	}
	ndat = THINLEN;

      } else if ( dtyp == THOUTTYP ) {
	syslog(LOG_DEBUG, "wmr9x8rd: devicetyp is THOUTTYP");
	for ( i = 0; i < THOUTLEN-3; i++) {
	  err = getschar( rfd, &schr);
          data[i+3] = schr;
	}
	ndat = THOUTLEN;

      } else if ( dtyp == TINTYP ) {
	syslog(LOG_DEBUG, "wmr9x8rd: devicetyp is TINTYP");
	for ( i = 0; i < TINLEN - 3; i++) {
	  err = getschar( rfd, &schr);
          data[i+3] = schr;
	}
	ndat = TINLEN;
      } else if ( dtyp == THBTYP ) {
	syslog(LOG_DEBUG, "wmr9x8rd: devicetyp is THBTYP");
	for ( i = 0; i < THBLEN - 3; i++) {
	  err = getschar( rfd, &schr);
          data[i+3] = schr;
	}
	ndat = THBLEN;

      } else if ( dtyp == MINTYP ) {
	syslog(LOG_DEBUG, "wmr9x8rd: devicetyp is MINTYP");
	for ( i = 0; i < MINLEN - 3; i++) {
	  err = getschar( rfd, &schr);
          data[i+3] = schr;
	}
	ndat = MINLEN;
      } else if ( dtyp == CLOCKTYP ) {
	syslog(LOG_DEBUG, "wmr9x8rd: devicetyp is CLOCKTYP");
	for ( i = 0; i < CLOCKLEN - 3; i++) {
	  err = getschar( rfd, &schr);
          data[i+3] = schr;
	}
	ndat = CLOCKLEN;

      } else if ( dtyp == THBNEWTYP ) {
	syslog(LOG_DEBUG, "wmr9x8rd: devicetyp is THBNEWTYP");
	for ( i = 0; i < THBNEWLEN - 3; i++) {
	  err = getschar( rfd, &schr);
          data[i+3] = schr;
	}
	ndat = THBNEWLEN;
      } else {
	syslog(LOG_ALERT, "wmr9x8rd: UNKNOWN devicetyp");

      }
      syslog(LOG_DEBUG, "wmr9x8rd: data record\n");
      echodata( data, ndat);

      sync = 0; ndat = 0; memset( data, 0, TBUFF);    
    }

  }

  return(err); 
}


int 
main (int argc, char **argv)
{
  int rfd, tset, i, j;
  int err;

  struct termios tp, op;

  openlog("wmr9x8_test", LOG_PID | LOG_PERROR , LOG_LOCAL5);
  memset(&tp, 0, sizeof( struct termios));
  /* test loop to check how long it takes to open/close the port */
  for ( j = 0; j < BENCHMARK; j++) {
  rfd = open("/dev/ttyd0", O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
  
  syslog(LOG_DEBUG,"open\t\t: errno %d\n", errno);
  if (rfd == -1)
  {
    perror("open: couldn't open /dev/ttyd0");
  }
  else {
    fcntl(rfd, F_SETFL, 0);
  }

  /* get terminal attributes */
  tcgetattr(rfd, &op);
  tcgetattr(rfd, &tp);
  syslog(LOG_DEBUG,"tcgetattr\t: errno %d\n", errno);
  syslog(LOG_DEBUG,"tcgetattr\t: current settings\n");
  syslog(LOG_DEBUG,"tcgetattr\t: iflag - 0%o; oflag - 0%o; cflag - 0%o; lflag - 0%o\n",
    tp.c_iflag, tp.c_oflag, tp.c_cflag, tp.c_lflag);


  /* set terminal attr */
  cfsetispeed(&tp, 9600);
  cfsetospeed(&tp, 9600);

  tp.c_iflag = 0;
  tp.c_oflag = 0;
  tp.c_cflag = 0;
  tp.c_lflag = 0;

  for ( i = 0; i < NCCS; i++) {
	tp.c_cc[i] = 0;
  }

  tp.c_iflag &= (IXON | IXOFF | IXANY);
  
  tp.c_cflag |= PARENB;
  tp.c_cflag &= ~PARODD;
  tp.c_cflag |= CLOCAL;
  tp.c_cflag |= CREAD;
  tp.c_cflag |= CS8;
  tp.c_cflag |= CSTOPB;
  tp.c_cflag |= CSIZE; 
  

  tp.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

  tp.c_cc[VMIN]=0;
  tp.c_cc[VTIME]=1;

  tcsetattr(rfd, TCSANOW, &tp);
  syslog(LOG_DEBUG,"tcsetattr\t: errno %d\n", errno);
  syslog(LOG_DEBUG,"tcsetattr\t: current settings\n");
  syslog(LOG_DEBUG,"tcsetattr\t: iflag - 0%o; oflag - 0%o; cflag - 0%o; lflag - 0%o\n",
    tp.c_iflag, tp.c_oflag, tp.c_cflag, tp.c_lflag);

  tcgetattr(rfd, &tp);
  syslog(LOG_DEBUG,"tcgetattr\t: errno %d\n", errno);
  syslog(LOG_DEBUG,"tcgetattr\t: current settings\n");
  syslog(LOG_DEBUG,"tcgetattr\t: iflag - 0%o; oflag - 0%o; cflag - 0%o; lflag - 0%o\n",
    tp.c_iflag, tp.c_oflag, tp.c_cflag, tp.c_lflag);
  

  /* raise DTR and RTS */
  tset |= TIOCM_DTR;
  tset |= TIOCM_RTS;
  ioctl(rfd, TIOCMSET, &tset);

  /* read weatherstation */
  //err = readdata( rfd, data, &mdat);
  err = wmr9x8rd( rfd);

  /* lower DTR and RTS on serial line */
  syslog(LOG_DEBUG,"ioctl\t\t: lowering DTR and RTS voltage\n");
  ioctl(rfd, TIOCMGET, &tset);
  tset &= ~TIOCM_DTR;
  tset &= ~TIOCM_RTS;
  ioctl(rfd, TIOCMSET, &tset);

  /* reset terminal attributes */
  tcsetattr(rfd, TCSANOW, &op);
  tcgetattr(rfd, &tp);
  syslog(LOG_DEBUG,"tcgetattr\t: errno %d\n", errno);
  syslog(LOG_DEBUG,"tcgetattr\t: current settings\n");
  syslog(LOG_DEBUG,"tcgetattr\t: iflag - 0%o; oflag - 0%o; cflag - 0%o; lflag - 0%o\n",
    tp.c_iflag, tp.c_oflag, tp.c_cflag, tp.c_lflag);

  close(rfd);
  syslog(LOG_DEBUG,"close\t\t: errno: %d\n", errno);
  closelog();
  }
  return(err);
}

