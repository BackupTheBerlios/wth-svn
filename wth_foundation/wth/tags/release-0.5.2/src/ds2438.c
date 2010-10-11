/*

  ds2438.c - Sample code to read 
  - Vsens 
  - accumuluated current register
  of DS2438

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
#include "wth.h"
#include "ds2438.h"

/*

  SetupVsens

  setup DS2438 to read Vsens voltage difference

  enable IAD, CA and EE of status configuration register ( page <0h> byte <0h>)

  Vsens A/D conversion occurs with a frequency of 36.41 measurements/sec
  once IAD is enabled ( set to '1'). No special command necessary.
 
  input parameters
    portnum    port number
    SNum       serial number of DS2438
    device     device (USB DS2490 or serial DS9097U)

*/
int SetupVsens(int portnum, uchar *SNum, char *device)
{
   uchar datablock[50];
   uchar conf_reg = 0x00;
   int send_cnt = 0;
   int i;
   ushort lastcrc8;
   int busybyte;
   double ti, tf;
   struct timezone tz;
   struct timeval  tv; 

   gettimeofday( &tv, &tz);
   ti = tv.tv_sec+1.0e-6*tv.tv_usec;

   /* enable IAD, CA and EE of configuration byte */
   conf_reg |= IAD | CA | EE;

   owSerialNum(portnum,SNum,FALSE);
   // Recall the Status/Configuration page
   // Recall command
   datablock[send_cnt++] = 0xB8;

   // Page to Recall
   datablock[send_cnt++] = 0x00;

   if(!owBlock(portnum,FALSE,datablock,send_cnt))
      return FALSE;

   send_cnt = 0;

   
   if(owAccess(portnum))
   {
      // Read the Status/Configuration byte
      // Read scratchpad command
      datablock[send_cnt++] = 0xBE;

      // Page for the Status/Configuration byte
      datablock[send_cnt++] = 0x00;

      for(i=0;i<9;i++)
         datablock[send_cnt++] = 0xFF;

      if(owBlock(portnum,FALSE,datablock,send_cnt))
      {
         setcrc8(portnum,0);

         for(i=2;i<send_cnt;i++)
            lastcrc8 = docrc8(portnum,datablock[i]);

         if(lastcrc8 != 0x00)
            return FALSE;
      }//Block
      else
         return FALSE;

      if ( datablock[2] & conf_reg ) {
        syslog(LOG_DEBUG, "SetupVsens: IAD, CA and EE are set: return!\n");

	gettimeofday( &tv, &tz);
	tf = tv.tv_sec+1.0e-6*tv.tv_usec;
        syslog(LOG_DEBUG,
	       "SetupVsens: elapsed time: %f\n", tf -ti);

        return TRUE;
      } else {
	  syslog(LOG_DEBUG,
		 "SetupVsens: IAD, CA and EE are not set. Continue to setup\n");
      }

   }//Access

   if(owAccess(portnum))
   {
      send_cnt = 0;
      // Write the Status/Configuration byte
      // Write scratchpad command
      datablock[send_cnt++] = 0x4E;

      // Write page
      datablock[send_cnt++] = 0x00;

      // IAD, CA and EE set to "1"
      datablock[send_cnt++] |= conf_reg;

      // do not change the rest
      for(i=0;i<7;i++)
         datablock[send_cnt++] = datablock[i+3];

      if(owBlock(portnum,FALSE,datablock,send_cnt))
      {
         send_cnt = 0;

         if(owAccess(portnum))
         {
            // Copy the Status/Configuration byte
            // Copy scratchpad command
            datablock[send_cnt++] = 0x48;

            // Copy page
            datablock[send_cnt++] = 0x00;

            if(owBlock(portnum,FALSE,datablock,send_cnt))
            {
               busybyte = owReadByte(portnum);
         
               while(busybyte == 0)
                  busybyte = owReadByte(portnum);

	       gettimeofday( &tv, &tz);
	       tf = tv.tv_sec+1.0e-6*tv.tv_usec;
               syslog(LOG_DEBUG,
		      "SetupVsens: elapsed time: %f\n", tf -ti);
               return TRUE;
            }//Block
         }//Access
      }//Block

   }//Access

   return FALSE;
}

/*

  ReadVsens

  read DS2438 Vsens voltage difference

  input parameters
    portnum    port number
    vsens      switch to read either singel or accumulated vsens
    SNum       serial number of DS2438
    device     1-wire device (USB DS2490 or serial DS9097U)
*/
float ReadVsens(int portnum, int vsens, uchar *SNum, char *device)
{
  int vs_sign;
  long vs_low, vs_high, vs_val;
  uchar pageno;
  uchar pagemem[NBUFF+1] = "";
  float Vsens = (float) -1.0;
  double ti, tf;
  struct timezone tz;
  struct timeval  tv; 

  if ( SetupVsens( portnum, SNum, device) == FALSE) {
    syslog(LOG_ALERT,"Setup DS2438 to read Vsens failed!\n");
    return 0.0;
  }

  gettimeofday( &tv, &tz);
  ti = tv.tv_sec+1.0e-6*tv.tv_usec;

  if ( vsens == VSENS ) {   /* read Vsens voltage */ 
    pageno = 0x00;
    ds2438mem_rd( 0, SNum, pagemem, pageno, device);
    /* Vsens in current lsb and current msb */
    vs_low = pagemem[7];
    vs_high = ( pagemem[8] & ~VSS ) << 8;
    vs_val = vs_high + vs_low;
    if ( pagemem[8] & VSS) {
      vs_val |= ~0x3ff;
    }
    Vsens = 0.2441 * vs_val;

  } else if ( vsens == VACC ) {  /* read accumulated total of voltage */
    pageno = 0x01;
    ds2438mem_rd( 0, SNum, pagemem, pageno, device);
    Vsens = pagemem[6] * 0.4882;
  }
  gettimeofday( &tv, &tz);
  tf = tv.tv_sec+1.0e-6*tv.tv_usec;
  syslog(LOG_DEBUG,"ReadVsens: elapsed time: %f\n", tf -ti);

  return Vsens;
}


/*
  ds2338mem_rd

  Reading DS2438 memory

  returns one page of memory

  input parameters 
  portnum  the port number of the port being used for the
           1-Wire Network.
  SNum     the serial number for the part that the read is
           to be done on.
  pageno   the page number of memory to be read
 
 */
int 
ds2438mem_rd(int portnum, uchar *SNum, uchar *pagemem, uchar pageno, char *device) {
   int block_cnt;
   int i;
   ushort lastcrc8;

   owSerialNum(portnum,SNum,FALSE);

   block_cnt = 0;

   // Recall the Status/Configuration page
   // Recall command
   pagemem[block_cnt++] = 0xB8;
   // Page to Recall
   pagemem[block_cnt++] = pageno;
   owAccess(portnum);
   owBlock(portnum,FALSE,pagemem,block_cnt);     
   syslog(LOG_DEBUG, "ds2438mem_rd: recall memory (B8h %xh): %s\n", 
     pageno, ppagemem(pagemem));
   block_cnt = 0;
   // Read the Status/Configuration byte
   // Read scratchpad command
   pagemem[block_cnt++] = 0xBE;
   // Page for the Status/Configuration byte
   pagemem[block_cnt++] = pageno;
   for(i=0;i<9;i++)
     pagemem[block_cnt++] = 0xFF;
   owAccess(portnum);
   owBlock(portnum,FALSE,pagemem,block_cnt);     
   syslog(LOG_DEBUG,"ds2438mem_rd: read scratchpad (BEh %xh): %s \n", 
       pageno, ppagemem( pagemem));

   setcrc8(portnum,0);
   for(i=2;i<block_cnt;i++) {
     lastcrc8 = docrc8(portnum,pagemem[i]);
   }
   if(lastcrc8 != 0x00) {
     syslog(LOG_ALERT, "ds2438mem_rd: CRC error ");
     bitprint( lastcrc8, "lastcrc8");
     return 1;
   }

   return 0;
}


int
ds2438mem_dump( int portnum, int verbose, uchar *serialnum, char *device)
{
  int i;
  int vs_sign, t_sign;
  long vs_low, vs_high, vs_val;
  long t_low, t_high, t_val;
  uchar pageno;
  uchar pagemem[NBUFF+1] = "";
  uchar mempage[8] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
  float temp, Vsens, Vacc;

  syslog(LOG_INFO, "DS2438 memory pages dump:\n");
  if ( verbose == FALSE) {
    for ( i = 0; i < 8; i++) {
      ds2438mem_rd( 0, serialnum, pagemem, mempage[i], device);
      syslog(LOG_INFO, "page# <%xh>: %s\n", mempage[i], ppagemem(pagemem));
    }
  } else {  
    pageno  = 0x00;
    ds2438mem_rd( 0, serialnum, pagemem, pageno, device);
    syslog(LOG_INFO, "page# <%xh>: %s\n", pageno, ppagemem(pagemem));
    bitprint( pagemem[2], "\tstatus config   : page <0h> byte <0h>");
    /* Current A/D Control Bit */
    if (pagemem[2] & IAD) {
      syslog(LOG_INFO, "\t\tIAD: 1 ( current A/D and ICA enabled)\n");
    } else {
      syslog(LOG_INFO, "\t\tIAD: 0 ( current A/D and ICA disabled)\n");
    }
    /* Current Accumulator Configuration */
    if ( pagemem[2] & CA) {
      syslog(LOG_INFO, "\t\tCA : 1 ( CCA/DCA enabled)\n");
    } else {
      syslog(LOG_INFO, "\t\tCA : 0 ( CCA/DCA disabled)\n");
    }
    /* Current Accumulator Shadow Selector Bit */
    if ( pagemem[2] & EE ) {
      syslog(LOG_INFO, "\t\tEE : 1 ( CCA/DCA data shadowed to EEPROM)\n");
    } else {
      syslog(LOG_INFO, "\t\tEE : 0 ( CCA/DCA data not shadowd to EEPROM)\n");
    }
    /* Voltage A/D Input Select Bit */
    if ( pagemem[2] &  AD) {
      syslog(LOG_INFO, "\t\tAD : 1 ( Vdd selected for AD conversion)\n");
    } else {
      syslog(LOG_INFO, "\t\tAD : 0 ( Vad selected for AD conversion)\n");
    }
    /* Temperature Busy Flag */
    if ( pagemem[2] & TB) {
      syslog(LOG_INFO, "\t\tTB : 1 ( temperature conversion in progress)\n");
    } else {
      syslog(LOG_INFO, "\t\tTB : 0 ( temperature conversion completed)\n");
    }
    /* Nonvolatile Memory Busy Flag */
    if ( pagemem[2] & NVB) {
      syslog(LOG_INFO, "\t\tNVB: 1 ( copy from scratchpad to EEPROM in progress)\n");
    } else {
      syslog(LOG_INFO, "\t\tNVB: 0 ( nonvolatile memory not busy)\n");
    }
    /* A/D Converter Busy Flag */
    if ( pagemem[2] & ADB) {
      syslog(LOG_INFO, "\t\tADB: 1 ( A/D conversion in progress on battery voltage)\n");
    } else {
      syslog(LOG_INFO, "\t\tADB: 0 ( A/D conversion complete)\n");
    }

    bitprint( pagemem[3], "\ttemperature lsb ( page <0h> byte <1h>)");
    bitprint( pagemem[4], "\ttemperature msb ( page <0h> byte <2h>)");
    /*  temperature readout */
    /*  temperature sign is bit 7 of temperature msb */
    if ( pagemem[4] & TS) {
      t_sign = -1; 
      syslog(LOG_INFO, "\t\tTemperature sign bit set: temperature negative\n");
    } else {
      syslog(LOG_INFO, "\t\tTemperature sign bit not set: temperature positive\n");
      t_sign =  1;
    }

    /* temperature value is composed of temperature lsb and temperature msb  */
    t_low = pagemem[3] >> 3;
    t_high = ( pagemem[4] & ~TS) << 5;
    //t_high = pagemem[4] << 5;
    t_val = t_high + t_low;
    temp = 0.03125 * t_val * t_sign;
    syslog(LOG_INFO, "\t\tTemperature: %f [deg C]\n", temp);


    bitprint( pagemem[5], "\tvoltage lsb     ( page <0h> byte <3h>)");
    bitprint( pagemem[6], "\tvoltage msb     ( page <0h> byte <4h>)");
    bitprint( pagemem[7], "\tcurrent lsb     ( page <0h> byte <5h>)");
    bitprint( pagemem[8], "\tcurrent msb     ( page <0h> byte <6h>)");
    /* Vsens is contained in current lsb and current msb */
    if ( pagemem[8] & VSS) {
      vs_sign = -1;
    } else {
      vs_sign =  1;
    }
    vs_low = pagemem[7];
    vs_high = ( pagemem[8] & ~VSS) << 8;
    //vs_high = pagemem[8] << 8;
    vs_val = vs_high + vs_low;
    Vsens = 0.2441 * vs_val * vs_sign;
    syslog(LOG_INFO,"\t\tVsens: %f [mV]\n", Vsens);

    bitprint( pagemem[9], "\tthreshold       ( page <0h> byte <7h>)");

    pageno  = 0x01;
    ds2438mem_rd( 0, serialnum, pagemem, pageno, device);
    syslog(LOG_INFO,"page# <%xh>: %s\n", pageno, ppagemem(pagemem));
    bitprint( pagemem[2], "\tetm byte 0      ( page <1h> byte <0h>)");
    bitprint( pagemem[3], "\tetm byte 1      ( page <1h> byte <1h>)");
    bitprint( pagemem[4], "\tetm byte 2      ( page <1h> byte <2h>)");
    bitprint( pagemem[5], "\tetm byte 3      ( page <1h> byte <3h>)");
    bitprint( pagemem[6], "\tica             ( page <1h> byte <4h>)");
    Vacc = 0.4882 * pagemem[6];
    syslog(LOG_INFO,"\t\tIntegrated current accumulator: Vacc: %f [mVhr]\n", Vacc);
    bitprint( pagemem[7], "\toffset lsb      ( page <1h> byte <5h>)");
    bitprint( pagemem[8], "\toffset msb      ( page <1h> byte <6h>)");
    bitprint( pagemem[9], "\treserved        ( page <1h> byte <7h>)");

    pageno  = 0x02;
    ds2438mem_rd( 0, serialnum, pagemem, pageno, device);
    syslog(LOG_INFO, "page# <%xh>: %s\n", pageno, ppagemem(pagemem));
    bitprint( pagemem[2], "\tdisconnect #0   ( page <2h> byte <0h>)");
    bitprint( pagemem[3], "\tdisconnect #1   ( page <2h> byte <1h>)");
    bitprint( pagemem[4], "\tdisconnect #2   ( page <2h> byte <2h>)");
    bitprint( pagemem[5], "\tdisconnect #3   ( page <2h> byte <3h>)");
    bitprint( pagemem[6], "\tend of charge #0( page <2h> byte <4h>)");
    bitprint( pagemem[7], "\tend of charge #1( page <2h> byte <5h>)");
    bitprint( pagemem[8], "\tend of charge #2( page <2h> byte <6h>)");
    bitprint( pagemem[9], "\tend of charge #3( page <2h> byte <7h>)");

    pageno  = 0x07;
    ds2438mem_rd( 0, serialnum, pagemem, pageno, device);
    syslog(LOG_INFO,"page# <%xh>: %s\n", pageno, ppagemem(pagemem));
    bitprint( pagemem[2], "\tuser byte 0     ( page <7h> byte <0h>)");
    bitprint( pagemem[3], "\tuser byte 1     ( page <7h> byte <1h>)");
    bitprint( pagemem[4], "\tuser byte 2     ( page <7h> byte <2h>)");
    bitprint( pagemem[5], "\tuser byte 3     ( page <7h> byte <3h>)");
    bitprint( pagemem[6], "\tcca lsb         ( page <7h> byte <4h>)");
    bitprint( pagemem[7], "\tcca msb         ( page <7h> byte <5h>)");
    bitprint( pagemem[8], "\tdca lsb         ( page <7h> byte <6h>)");
    bitprint( pagemem[9], "\tdca msb         ( page <7h> byte <7h>)");
  }
  return(0);
}      

