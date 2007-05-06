/* data.c

   $Id: dataxml.c,v 1.1 2002/07/04 09:51:10 jahns Exp $
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

#include "wth.h"
#include "wthprv.h"

/* getrd

   get raw data
*/
int getrd (unsigned char *data, int *mdat, struct cmd *pcmd) {
    int err;

    if ( pcmd->netflg == 0 ) /* read serial interface */ {
	  if ( ( err = getsrd(data, mdat, pcmd)) == -1)
		return(-1);
    }
    else if (pcmd->netflg == 1 ) /* read network interface */ {
	  if ( ( err = getnrd(data, mdat, pcmd)) == -1)
		return(-1);
    }
    else if (pcmd->netflg == 2 ) /* xmlrpc */ {
	  if ( ( err = getxmlrd(data, mdat, pcmd)) == -1)
		return(-1);
    }
    return(0);
}



