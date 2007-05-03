/* serial.c

   serial line communication routine

   $Id: serial.c,v 0.2 2001/03/01 06:30:03 volker Exp $
   $Revision: 0.2 $

*/

#include "wth.h"

static volatile int STOP = FALSE;
static volatile int wait_flag = TRUE;
static volatile int alrm_flag = FALSE;


/* 
   sigio_h
   
   handles SIGIO sent by weather station when data are available
*/
void sigio_h(int signum) {
  wait_flag = FALSE;
  return;
}

/*
   sigalrm_h
  
   handles SIGALRM when timer goes off after timeout read
*/
void sigalrm_h(int signum) {
  alrm_flag = TRUE;
  return;
}



/*  initserial

  opens serial port for communication
  installs SIGIO asynchronuous signal handler, SIGALRM for read timeout
  serial port settings:
     9600, 8, Parity,  2Stop
     lower RTS and raise DTR voltage

*/
int initserial (int *pfd, struct termios *newtio, struct termios *oldtio) {

  int i, itio; 

  /* open the device to be non-blocking (read will return immediatly) */
  *pfd = open(SER_DEVICE, O_RDWR | O_NOCTTY );
  if (*pfd <0) {perror(SER_DEVICE); exit(-1); }


  /* install the signal handler before making the device asynchronous */
  signal(SIGIO, sigio_h);
  signal(SIGALRM,sigalrm_h);

  /* allow the process to receive SIGIO */
  fcntl(*pfd, F_SETOWN, getpid());
  /* Make the file descriptor asynchronous (the manual page says only 
     O_APPEND and O_NONBLOCK, will work with F_SETFL...) */
  fcntl(*pfd, F_SETFL, FASYNC);

  
  /* lower DTR and RTS on serial line */
  ioctl(*pfd, TIOCMGET, &itio);
  itio &= ~TIOCM_DTR;
  itio &= ~TIOCM_RTS;
  ioctl(*pfd, TIOCMSET, &itio); 
  /* raise DTR */
  itio |= TIOCM_DTR;
  ioctl(*pfd, TIOCMSET, &itio);

  tcgetattr(*pfd,oldtio);  /* save current port settings */
  tcgetattr(*pfd,newtio); /* prefill port settings */

  
  /* set communication link parameters */
  cfsetispeed(newtio, BAUDRATE);
  cfsetospeed(newtio, BAUDRATE);


  newtio->c_iflag = 0;
  newtio->c_oflag = 0;
  newtio->c_cflag = 0;
  newtio->c_lflag = 0;

  for ( i = 0; i < NCCS; i++) {
	newtio->c_cc[i] = 0;
  }
  
  
  /* newtio->c_iflag &= (IXON | IXOFF | IXANY);  */
  
  newtio->c_cflag |= PARENB;
  newtio->c_cflag &= ~PARODD;
  newtio->c_cflag |= CLOCAL;
  newtio->c_cflag |= CREAD;
  newtio->c_cflag |= CS8;
  newtio->c_cflag |= CSTOPB;
  newtio->c_cflag |= CSIZE; 
  
  /* newtio->c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); */

  newtio->c_cc[VMIN]=0;
  newtio->c_cc[VTIME]=10;

  tcsetattr(*pfd,TCSANOW,newtio);
  return(0);

}


/*
   closeserial

   restore the old port settings
   lower DTR on serial line

*/
int closeserial( int fd, struct termios *oldtio) {
    int tset;


    /* lower DTR on serial line */
    ioctl(fd, TIOCMGET, &tset);
    tset &= ~TIOCM_DTR;
    ioctl(fd, TIOCMSET, &tset);

	/* restore old port settings */
	/* takes approx 3 sec to reopen fd under FreeBSD
	   if tcsetattr is called to restore */
    /* tcsetattr(fd,TCSANOW,oldtio);  */
  
    /* close serial port */
    close(fd);
    return(0);
}



/*
  readdata

  reading data from serial port
  signal SIGIO is sent by the weather station when data are available
  huh - the weather station sends its data frame in chunks of several bytes   

*/
int readdata (int fd, char *data, int *ndat) {
    int i;
    int err;
    char rbuf[MAXBUFF];

    /* timeout handling: use function alarm() */
    alarm(TIMEOUT);

    *ndat = 0;
	
	while ( STOP == FALSE) {
	  /* input available */
	  if ( wait_flag == FALSE) {
		if ( ( err = read(fd, &rbuf, MAXBUFF)) > 0 ) {
		  for ( i = 0; i < err; i++) {
			data[*ndat + i] = rbuf[i];
		  }
		  *ndat += err;

		  wait_flag = TRUE;
		  if  ( rbuf[err-1] == 3) {
			STOP = TRUE;
		  }
		}
	  }
	  /* timeout experienced */
	  if ( alrm_flag == TRUE) {
		syslog(LOG_INFO, "sigalrm_h : timeout");
		STOP = TRUE;
	  }
	}    

	/* reset loop control flags */
	STOP = FALSE;
	wait_flag = TRUE;

	/* turnoff alarm timeout */
	alarm(0);
	
	if ( alrm_flag == TRUE )
	  return(-1);
	else
	  return(0);
}


/*  cword
    returns the command word for the weather station
    input is integer value of the command where

    0 : Poll DCF Time
    1 : Request Dataset
    2 : Select Next Dataset
    3 : Activate 9 temperature sensors
    4 : Activate 16 temperature sensors
    5 : Request status
    6 : Set interval time         
    shouldn't this go into a configuration file?
 */
char *cword(int n) {
  static char *name[] = {
    "\x01\x30\xcf\x04", "\x01\x31\xce\x04",
    "\x01\x32\xcd\x04", "\x01\x33\xcc\x04",
    "\x01\x34\xcb\x04", "\x01\x35\xca\x04",
    "\x01\x36\xc9\x04", "Illegal"
  };

  return name[n];
}


/* getsrd 

   get raw data from serial interface
*/
int getsrd (char *data, int *mdat, struct cmd *pcmd) {
    int err;
    int fd;                         /* filedescriptor serial port */

    char cintv[5] = "\x01\x36\x53\xc9\x04";  
                                    /* temporary array to hold commandword
                                         in case of setting interval */

    struct termios newtio,oldtio;   /* termios structures to set 
                                     comm parameters */


    /* initialize serial port  */
    err = initserial(&fd, &newtio, &oldtio);

    /* read ETX from serial port
 
      when the weather station is activated by setting DTR it answers
      with an ETX char to signalize it's OK 
    */
    if ( (err = readdata(fd, data, mdat)) == -1 )
	  return(-1);
	
    /* syslog(LOG_INFO, "wcmd : mdat %d\n", mdat); */

    /* modify cword for intervall time */
    if ( pcmd->command == 6) {
      cintv[2] = pcmd->argcmd;
      cintv[3] = cintv[3] - pcmd->argcmd;
      err = write(fd, cintv, 5); 

    } else {
      /* write the command word to the weather station */ 
      err = write(fd, cword(pcmd->command), 4); 
    }
    syslog(LOG_INFO, "getsrd : command written : %d\n", pcmd->command);

    /* read answer of weather station from serial port */
    if ( ( err = readdata(fd, data, mdat)) == -1)
	  return(-1);
    
    /* echo raw dataframe */
    err = echodata(data, *mdat);
    
   /* leave serial line in good state */ 
    err = closeserial(fd, &oldtio);

    syslog(LOG_INFO, "getsrd : Data length getsrd : %d\n", *mdat);
    return(0);
}
