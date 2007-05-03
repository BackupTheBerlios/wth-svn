/* wthprv.h

   private header file for WS2000 weatherstation communication
      

   $Id: wthprv.h,v 1.1 2002/07/04 09:52:46 jahns Exp $
   $Revision: 1.1 $

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

Sigfunc *signal(int signo, Sigfunc *func);
/* for our signal() function */
/* static Sigfunc *Signal(int signo, Sigfunc *func); */

int initserial( int *pfd, struct termios *newtio, 
		struct termios *oldtio, struct cmd *pcmd);
int closeserial( int fd, struct termios *oldtio);
int readdata( int fd, unsigned char *data, 
  int *ndat, struct cmd *pcmd);

Ckey *c(int n);
int wstrlen(char *s);

unsigned getbits(unsigned x, int p, int n);
char *mkmsg(const char *, ...);

int demasq(unsigned char *data, int *mdat);
int chkframe(unsigned char *data, int *mdat, struct cmd *pcmd);
int datex(unsigned char *data, int ndat, struct wthio *rw, struct cmd *pcmd);
char *pdata(struct wthio *rw);
int echodata(unsigned char *data, int mdat);
