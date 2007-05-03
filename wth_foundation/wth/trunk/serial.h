/* serial.h */

void signal_handler( int signum);
int  initserial ( int *pfd, struct termios *newtio, struct termios *oldtio);
int  closeserial( int fd, struct termios *oldtio);
int  readdata ( int fd, u_char *data, int *ndat);
char *cword( int n);
int  getsrd (char *data, int *mdat, struct cmd *pcmd);



