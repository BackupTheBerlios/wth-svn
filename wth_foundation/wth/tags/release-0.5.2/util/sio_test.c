/* 
  sio_test.c

  testing serial I/O with wmr9x8 weatherstation

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
#include <time.h>

#define BENCHMARK 1
#define TBUFF 16

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

    printf("echodata : length dataframe : %d\n",mdat);
    for ( i = 0; i < mdat; i++ ) {
      printf("echodata : byte[%d] : %02x : ", i, data[i]); bitprint( data[i], " ");
    }

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
shuffdat( unsigned char *cdat, unsigned char *rdat, int len) {
  int i;

  cdat[0] = 0xff;
  cdat[1] = 0xff;

  for ( i = 0; i < len-2; i++) {
    cdat[i+2] = rdat[i];
  }

  return(0);
}

int 
readdata( int rfd, unsigned char *data, int *ndat) {
  int err = -1;
  int sync, cnt;
  unsigned char schr;
  unsigned char rdat[TBUFF+1];
  unsigned char cdat[TBUFF+1];

  sync = 0; cnt = 0;
  for (;;) {
    err = getschar( rfd, &schr);
    if ( err == 1) {
      printf("readdata: err: %d schr : %02x\n", err, schr);
      rdat[cnt] = schr;
      cnt++;
    } else {
      printf("readdata: could not read 1 char err: %d\n", err);
    }

    if ( schr == 0xff) {
      sync++;
    }

    if ( sync == 2) {
      printf("readdata: header sync received\n");
      shuffdat( cdat, rdat, cnt);
      printf("raw data\n");
      echodata( rdat, cnt);
      printf("corrected data\n");
      echodata( cdat, cnt);
      sync = 0; cnt = 0;
    }
  }

  return(err); 
}



int 
main (int argc, char **argv)
{
  int rfd, tset, i, j;
  int err, mdat;
  static unsigned char data[TBUFF+1];
  struct termios tp, op;

  /* test loop to check how long it takes to open/close the port */
  for ( j = 0; j < BENCHMARK; j++) {

  rfd = open("/dev/ttyd0", O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
  
  printf("open\t\t: errno %d\n", errno);
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
  printf("tcgetattr\t: errno %d\n", errno);
  printf("tcgetattr\t: current settings\n");
  printf("tcgetattr\t: iflag - 0%o; oflag - 0%o; cflag - 0%o; lflag - 0%o\n",
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
  printf("tcsetattr\t: errno %d\n", errno);
  printf("tcsetattr\t: current settings\n");
  printf("tcsetattr\t: iflag - 0%o; oflag - 0%o; cflag - 0%o; lflag - 0%o\n",
    tp.c_iflag, tp.c_oflag, tp.c_cflag, tp.c_lflag);

  tcgetattr(rfd, &tp);
  printf("tcgetattr\t: errno %d\n", errno);
  printf("tcgetattr\t: current settings\n");
  printf("tcgetattr\t: iflag - 0%o; oflag - 0%o; cflag - 0%o; lflag - 0%o\n",
    tp.c_iflag, tp.c_oflag, tp.c_cflag, tp.c_lflag);
  

  /* raise DTR and RTS */
  tset |= TIOCM_DTR;
  tset |= TIOCM_RTS;
  ioctl(rfd, TIOCMSET, &tset);

  /* read weatherstation */
  err = readdata( rfd, data, &mdat);

  /* lower DTR and RTS on serial line */
  printf("ioctl\t\t: lowering DTR and RTS voltage\n");
  ioctl(rfd, TIOCMGET, &tset);
  tset &= ~TIOCM_DTR;
  tset &= ~TIOCM_RTS;
  ioctl(rfd, TIOCMSET, &tset);

  /* reset terminal attributes */
  tcsetattr(rfd, TCSANOW, &op);
  tcgetattr(rfd, &tp);
  printf("tcgetattr\t: errno %d\n", errno);
  printf("tcgetattr\t: current settings\n");
  printf("tcgetattr\t: iflag - 0%o; oflag - 0%o; cflag - 0%o; lflag - 0%o\n",
    tp.c_iflag, tp.c_oflag, tp.c_cflag, tp.c_lflag);

  close(rfd);
  printf("close\t\t: errno: %d\n", errno);
  }
  return(err);
}
