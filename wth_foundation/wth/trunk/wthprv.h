#/* wth.h

   private header file for WS2000 weatherstation communication
      

   $Id: wth.h,v 0.4 2001/09/14 15:42:15 jahns Exp jahns $
   $Revision: 0.4 $

   Copyright (C) 2000-2001 Volker Jahns <Volker.Jahns@thalreit.de>

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


typedef void Sigfunc (int);  

/* function prototypes */
static void sigio_h(int signum);
static void sigalrm_h(int signum);

Sigfunc *signal(int signo, Sigfunc *func);
Sigfunc *Signal(int signo, Sigfunc *func);	/* for our signal() function */

int initserial( int *pfd, struct termios *newtio, 
		struct termios *oldtio, struct cmd *pcmd);
int closeserial( int fd, struct termios *oldtio);
int readdata( int fd, unsigned char *data, int *ndat);
Ckey *c(int n);

unsigned getbits(unsigned x, int p, int n);
int wstrlen(char *s);
char *mkmsg(const char *, ...);

const char *inet_ntop(int, const void *, char *, size_t);
int inet_pton(int, const char *, void *); 

int demasq(unsigned char *data, int *mdat);
int chkframe(unsigned char *data, int *mdat, struct cmd *pcmd);
int datex(unsigned char *data, int ndat, struct wthio *rw);
