/* xml.c

   xmlrpc related code

   $Id$
   $Revision$

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
#include "wthnet.h"
#include "wthprv.h"
#include "config.h"

#include <xmlrpc.h>
#include <xmlrpc_client.h>


/* Check error-handling environment for an XML-RPC fault. */
int return_on_fault (xmlrpc_env *env)
{
    if (env->fault_occurred) {
        syslog(LOG_INFO, "XML-RPC Fault: %s (%d)\n",
                env->fault_string, env->fault_code);
        return(-1);
    }
}


/* 

   getxmlrd - xmlrpc version
   
   client communication w/ wthxmld
 
*/
int getxmlrd(unsigned char *data, int *mdat, struct cmd *pcmd) {
    int i;
    xmlrpc_int32 ival;  /* holds interval time */
    xmlrpc_env env;
    xmlrpc_value *result;
    static char xmlserver_url[MAXBUFF];
    unsigned char rbuf[MAXBUFF]="\x02\x01\x15\xe8\x03";
    unsigned char* rdata;
    size_t* datasize;


    snprintf(xmlserver_url, 
	     sizeof(xmlserver_url), 
	     "http://%s:%s/RPC2", pcmd->hostname, pcmd->xmlport);

    /* Start up our XML-RPC client library. */
    xmlrpc_client_init(XMLRPC_CLIENT_NO_FLAGS, XMLNAME, VERSION);
    xmlrpc_env_init(&env);


    if ( pcmd->command == 0 ) { 
      result = xmlrpc_client_call(&env, xmlserver_url,
                                "poll.DCFTime", "()");
      return_on_fault(&env);
    } 
    else if ( pcmd->command == 1 ) { 
      result = xmlrpc_client_call(&env, xmlserver_url,
                                "request.Dataset", "()");
      return_on_fault(&env);
    } 
    else if ( pcmd->command == 2 ) { 
      result = xmlrpc_client_call(&env, xmlserver_url,
                                "select.NextDataset", "()");
      return_on_fault(&env);
    } 
    else if ( pcmd->command == 3 ) { 
      result = xmlrpc_client_call(&env, xmlserver_url,
                                "activate.8Sensors", "()");
      return_on_fault(&env);
    } 
    else if ( pcmd->command == 4 ) { 
      result = xmlrpc_client_call(&env, xmlserver_url,
                                "activate.16Sensors", "()");
      return_on_fault(&env);
    } 
    else if ( pcmd->command == 5 ) { 
      result = xmlrpc_client_call(&env, xmlserver_url,
                                "request.Status", "()");
      return_on_fault(&env);
    } 
    else if ( pcmd->command == 6 ) { 
      ival = pcmd->argcmd;
      result = xmlrpc_client_call(&env, xmlserver_url,
                                "set.LogInterval", "(i)",
                                (xmlrpc_int32) ival);
      return_on_fault(&env);
    } 
    else  {
      syslog(LOG_INFO, "getxmlrd: unknown command");
      werrno = ECMD;
      return(-1);
    }

    /* Parse our result value. */
    xmlrpc_parse_value(&env, result, "{s:6,*}",
                       "data", &rdata, &datasize);
    return_on_fault(&env);

    for ( i = 0; i < (int) datasize ; i++ ) {
      data[i] = rdata[i];
    }

    /* Dispose of our result value. */
    xmlrpc_DECREF(result);

    /* Shutdown our XML-RPC client library. */
    xmlrpc_env_clean(&env);
    xmlrpc_client_cleanup();
    
    *mdat = (int) datasize;
	
    return(0);
}





