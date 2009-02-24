/*

  ds2438.h 

  see also Dallas 1-Wire SDK header atod26.h 
  for access to voltage A/D and temperature

  $Id$
  $Revision$

  Copyright (C) 2009 Volker Jahns <volker@thalreit.de>

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
#define IAD 0x01
#define CA  0x02
#define EE  0x04
#define AD  0x08
#define TB  0x10
#define NVB 0x20
#define ADB 0x40

#define TS  0x80
#define VSS 0xFC

int SetupVsens( int portnum, uchar *SNum, char *device);
float ReadVsens( int portnum, int vsens, uchar *SNum, char *device);
int ds2438mem_rd( int portnum, uchar *SNum, uchar *pagemem, 
  uchar pageno, char *device);
int ds2438mem_dump( int portnum, int verbose, uchar *SNum, char *device);
