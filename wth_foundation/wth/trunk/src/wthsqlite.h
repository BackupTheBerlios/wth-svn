/* 
   wthsqlite.h

   header file for sqlite database handling

   $Id$
   $Revision$

   Copyright 2009-2012 Volker Jahns, <volker@thalreit.de>
 
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

/* sqlite database functions */

sensdevpar_t
sqlite_get_sensorparams( char *sensorname,
                         char *parametername,
                         int  stationtype);
sensflag_t
sqlite_get_sensorflags( char *sensorname, 
                        char *flagname,
                        int stationtype);
int sqlite_datadbn( long  dataset_date,
                    int   sensor_param, 
                    float meas_value,
                    int   stationtype);
int sqlite_statdbn( long statusset_date,
                    int  sensorflag, 
                    long unsigned int stat_value,
                    int stationtype);

/* WS2000 legacy */
int sqlite_stat_ws2000db( int    sensor_status[], 
                          time_t statusset_date);
int sqlite_new_ws2000db( int sensor_no,
                         int new_flag); 
int sqlite_is_ws2000sens( int sensor_no);

/* PCWSR legacy */
int sqlite_datadb( long    dataset_date, 
                   int     sensor_param,
                   float   meas_value,
                   sqlite3 *pcwsrdb);

