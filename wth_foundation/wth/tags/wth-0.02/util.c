/* util.c


   utility functions

   $Id: util.c,v 0.2 2001/03/01 06:32:40 volker Exp jahns $
   $Revision: 0.2 $

*/

#include "wth.h"

/* 
   getbits

   transfrom bytes of weather station data to BCD, otherwise picking single
   or groups of bits.
   Kerninghan, Ritchie, The C Programming Language, p49.
 */
unsigned getbits(unsigned x, int p, int n) {
  return ( x>>(p+1-n)) & ~(~0 <<n);
}

/* echodata
  
   print out raw data frame reveived from weather station 

*/
int echodata(u_char *data, int mdat) {
    int i;
    char frame[255] = "";
    char sf[3] = "";

    syslog(LOG_INFO, "echodata : length dataframe : %d\n",mdat);

    /* for better readability label each byte in frame */    
    for ( i = 0; i < mdat; i++ ) {
	sprintf(sf, "%2d:",i);
        strcat(frame, sf);
    }
    syslog(LOG_INFO, "echodata : %s\n", frame);    
    strcpy(frame, "");

    for ( i = 0; i < mdat; i++ ) {
	sprintf(sf, "%2x:",data[i]);
        strcat(frame, sf);
    }

    syslog(LOG_INFO, "echodata : %s\n", frame);   

    return(0);
} 


/* wstrlen

   Kerninghan, Ritchie, The C Programming Language, p103

*/
int wstrlen (char *s) {
    char *p = s;

    while ( *p != ETX ) {
	p++;
    }
    return p + 1 - s;
    
}

/* usage : print handling instructions of wthc */
int usage (int exitcode, char *error, char *addl) {
      fprintf(stderr,"Usage: wthc [Options]\n"
                     "where options include:\n"
                     "\t-c <command>\texecute command\n"
                     "\t\t0\tPoll DCF Time\n"
                     "\t\t1\tRequest dataset\n"
                     "\t\t2\tSelect next dataset\n"
                     "\t\t3\tActivate 9 temperature sensors\n"
                     "\t\t4\tActivate 16 temperature sensors\n"
                     "\t\t5\tRequest status\n"
                     "\t\t6\tSet interval time,\n" 
                     "\t\t\t\trequires extra value specified by option -i\n"
	             "\t\t12\tRequest all available data recursively\n"
                     "\t-i <interval>\tspecifies interval time\n"
		     "\t\tpermissible values for the interval lie\n"
		     "\t\twithin the range from 1 to 60 minutes\n"
                     "\t-h <hostname>\tconnect to <hostname/ipaddress>\n"
                     "\t-p <portnumber>\tuse portnumber at remote host\n"
                     "\t\tfor connection\n"
                     "\t-s\t\tuse local serial connection\n");

      if (error) fprintf(stderr, "%s: %s\n", error, addl);
      exit(exitcode);
}

/* usaged : print handling instructions of wthd */
int usaged (int exitcode, char *error, char *addl) {
      fprintf(stderr,"Usage: wthd [Options]\n"
                     "where options include:\n"
                     "\t-p <portnumber>\tuse portnumber to listen\n");

      if (error) fprintf(stderr, "%s: %s\n", error, addl);
      exit(exitcode);
}


Sigfunc *
signal(int signo, Sigfunc *func)
{
	struct sigaction	act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (signo == SIGALRM) {
#ifdef	SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;	/* SunOS 4.x */
#endif
	} else {
#ifdef	SA_RESTART
		act.sa_flags |= SA_RESTART;		/* SVR4, 44BSD */
#endif
	}
	if (sigaction(signo, &act, &oact) < 0)
		return(SIG_ERR);
	return(oact.sa_handler);
}
/* end signal */

Sigfunc *
Signal(int signo, Sigfunc *func)	/* for our signal() function */
{
	Sigfunc	*sigfunc;

	if ( (sigfunc = signal(signo, func)) == SIG_ERR)
		err_sys("signal error");
	return(sigfunc);
}

