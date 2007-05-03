#/* wthnet.h

   network related header file for WS2000 weatherstation communication
   
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

/* function prototypes */
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
