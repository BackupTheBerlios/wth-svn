/* util.h */

typedef void Sigfunc (int);  

unsigned getbits(unsigned x, int p, int n);
int echodata(u_char *data, int mdat);
int wstrlen(char *s);
int usage(int exitcode, char *error, char *addl);
int usaged(int exitcode, char *error, char *addl);
Sigfunc *signal(int signo, Sigfunc *func);
Sigfunc *Signal(int signo, Sigfunc *func);	/* for our signal() function */

