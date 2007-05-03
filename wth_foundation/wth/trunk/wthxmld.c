 /* 
    wthxmld

    a simple standalone XML-RPC server for WS2000 weatherstation service

*/

#include <stdio.h>

#include "wth.h"

#include <xmlrpc.h>
#include <xmlrpc_abyss.h>

xmlrpc_value *
poll_dcftime (xmlrpc_env *env, xmlrpc_value *param_array, void *user_data)
{
    int i;
    xmlrpc_int32 n, arg1, arg2;
    struct cmd     cmd;
    unsigned char  rdata[MAXBUFF];
    unsigned char* nakfram = "\x02\x01\x15\xe8\x03";


    openlog("wthxmld", LOG_PID, LOGFACILITY);

    initcmd(&cmd);
    readconfig(&cmd);
    cmd.command = 0;
    n = 0;

    /* Parse our argument array. */
    /* addtl commands only if to set interval time */
    xmlrpc_parse_value(env, param_array,"()");
    if (env->fault_occurred)
	return NULL;

    /* call getsrd for data retrieval */
    if ( getsrd( rdata, &n, &cmd) == -1 ) {
      strncpy(rdata, nakfram, strlen(nakfram));
      n = strlen(nakfram);
    }

    /* Return our result. */
    return xmlrpc_build_value(env, "{s:6}", 
			      "data", rdata, n);
}


xmlrpc_value *
request_status (xmlrpc_env *env, xmlrpc_value *param_array, void *user_data)
{
  int i;
    xmlrpc_int32 n, arg1, arg2;
    struct cmd     cmd;
    unsigned char  rdata[MAXBUFF];
    unsigned char* nakfram = "\x02\x01\x15\xe8\x03";


    openlog("wthxmld", LOG_PID, LOGFACILITY);

    initcmd(&cmd);
    readconfig(&cmd);
    cmd.command = 5;
    n = 0;

    /* Parse our argument array. */
    /* addtl commands only if to set interval time */
    xmlrpc_parse_value(env, param_array, "()");
    if (env->fault_occurred)
	return NULL;

    /* call getsrd for data retrieval */
    if ( getsrd( rdata, &n, &cmd) == -1 ) {
      strncpy(rdata, nakfram, strlen(nakfram));
      n = strlen(nakfram);
    }

    /*
      strncpy(rdata, nakfram, strlen(nakfram));
      n = strlen(nakfram);
    */
 
    /* debugging stuff */
    /*
    for ( i = 0; i < n; i++) {
      printf("%x:", rdata[i]);
    }
    printf("\n");
    */

    /* Return our result. */
    return xmlrpc_build_value(env, "{s:6}", 
			      "data", rdata, n);
}


xmlrpc_value *
request_dataset (xmlrpc_env *env, xmlrpc_value *param_array, void *user_data)
{
  int i;
    xmlrpc_int32 n, arg1, arg2;
    struct cmd     cmd;
    unsigned char  rdata[MAXBUFF];
    unsigned char* nakfram = "\x02\x01\x15\xe8\x03";


    openlog("wthxmld", LOG_PID, LOGFACILITY);

    initcmd(&cmd);
    readconfig(&cmd);
    cmd.command = 1;
    n = 0;

    /* Parse our argument array. */
    /* addtl commands only if to set interval time */
    xmlrpc_parse_value(env, param_array, "()");
    if (env->fault_occurred)
	return NULL;

    /* call getsrd for data retrieval */
    if ( getsrd( rdata, &n, &cmd) == -1 ) {
      strncpy(rdata, nakfram, strlen(nakfram));
      n = strlen(nakfram);
    }

    /* Return our result. */
    return xmlrpc_build_value(env, "{s:6}", 
			      "data", rdata, n);
}
xmlrpc_value *
select_next_dataset (xmlrpc_env *env, xmlrpc_value *param_array, void *user_data)
{
  int i;
    xmlrpc_int32 n, arg1, arg2;
    struct cmd     cmd;
    unsigned char  rdata[MAXBUFF];
    unsigned char* nakfram = "\x02\x01\x15\xe8\x03";


    openlog("wthxmld", LOG_PID, LOGFACILITY);

    initcmd(&cmd);
    readconfig(&cmd);
    cmd.command = 2;
    n = 0;

    /* Parse our argument array. */
    /* addtl commands only if to set interval time */
    xmlrpc_parse_value(env, param_array, "()");
    if (env->fault_occurred)
	return NULL;

    /* call getsrd for data retrieval */
    if ( getsrd( rdata, &n, &cmd) == -1 ) {
      strncpy(rdata, nakfram, strlen(nakfram));
      n = strlen(nakfram);
    }

    /* Return our result. */
    return xmlrpc_build_value(env, "{s:6}", 
			      "data", rdata, n);
}
xmlrpc_value *
activate_8_sensors (xmlrpc_env *env, xmlrpc_value *param_array, void *user_data)
{
    int i;
    xmlrpc_int32 n, arg1, arg2;
    struct cmd     cmd;
    unsigned char  rdata[MAXBUFF];
    unsigned char* nakfram = "\x02\x01\x15\xe8\x03";


    openlog("wthxmld", LOG_PID, LOGFACILITY);

    initcmd(&cmd);
    readconfig(&cmd);
    cmd.command = 3;
    n = 0;

    /* Parse our argument array. */
    /* addtl commands only if to set interval time */
    xmlrpc_parse_value(env, param_array, "()");
    if (env->fault_occurred)
	return NULL;

    /* call getsrd for data retrieval */
    if ( getsrd( rdata, &n, &cmd) == -1 ) {
      strncpy(rdata, nakfram, strlen(nakfram));
      n = strlen(nakfram);
    }

    /* Return our result. */
    return xmlrpc_build_value(env, "{s:6}", 
			      "data", rdata, n);
}
xmlrpc_value *
activate_16_sensors (xmlrpc_env *env, xmlrpc_value *param_array, void *user_data)
{
  int i;
    xmlrpc_int32 n, arg1, arg2;
    struct cmd     cmd;
    unsigned char  rdata[MAXBUFF];
    unsigned char* nakfram = "\x02\x01\x15\xe8\x03";


    openlog("wthxmld", LOG_PID, LOGFACILITY);

    initcmd(&cmd);
    readconfig(&cmd);
    cmd.command = 4;
    n = 0;

    /* Parse our argument array. */
    /* addtl commands only if to set interval time */
    xmlrpc_parse_value(env, param_array, "()");
    if (env->fault_occurred)
	return NULL;

    /* call getsrd for data retrieval */
    if ( getsrd( rdata, &n, &cmd) == -1 ) {
      strncpy(rdata, nakfram, strlen(nakfram));
      n = strlen(nakfram);
    }

    /*
      strncpy(rdata, nakfram, strlen(nakfram));
      n = strlen(nakfram);
    */
 
    /* debugging stuff */
    /*
    for ( i = 0; i < n; i++) {
      printf("%x:", rdata[i]);
    }
    printf("\n");
    */

    /* Return our result. */
    return xmlrpc_build_value(env, "{s:6}", 
			      "data", rdata, n);
}


xmlrpc_value *
set_log_interval (xmlrpc_env *env, xmlrpc_value *param_array, void *user_data)
{
  int i, err;
    xmlrpc_int32 n, arg1, arg2;
    struct cmd     cmd;
    unsigned char  rdata[MAXBUFF];
    unsigned char* nakfram = "\x02\x01\x15\xe8\x03";


    openlog("wthxmld", LOG_PID, LOGFACILITY);

    initcmd(&cmd);
    readconfig(&cmd);
    cmd.command = 6;
    n = 0;

    /* Parse our argument array. */
    /* addtl commands only if to set interval time */
    xmlrpc_parse_value(env, param_array, "(i)", &arg1);
    if (env->fault_occurred)
	return NULL;

    cmd.argcmd = arg1;
   
 
    /* call getsrd for data retrieval */
    err = getsrd( rdata, &n, &cmd);

    if ( err == -1 ) {
      strncpy(rdata, nakfram, strlen(nakfram));
      n = strlen(nakfram);
    }

    /* Return our result. */
    return xmlrpc_build_value(env, "{s:6}", 
			      "data", rdata, n);
}


int main (int argc, char **argv)
{
    if (argc != 2) {
	fprintf(stderr, "Usage: wthxmld abyss.conf\n");
	exit(1);
    }

    xmlrpc_server_abyss_init(XMLRPC_SERVER_ABYSS_NO_FLAGS, argv[1]);
    xmlrpc_server_abyss_add_method("poll.DCFTime", &poll_dcftime, NULL);
    xmlrpc_server_abyss_add_method("request.Dataset", &request_dataset, NULL);
    xmlrpc_server_abyss_add_method("select.NextDataset", 
				   &select_next_dataset, NULL);
    xmlrpc_server_abyss_add_method("activate.8Sensors", 
				   &activate_8_sensors, NULL);
    xmlrpc_server_abyss_add_method("activate.16Sensors", 
				   &activate_16_sensors, NULL);
    xmlrpc_server_abyss_add_method("request.Status", &request_status, NULL);
    xmlrpc_server_abyss_add_method("set.LogInterval", &set_log_interval, NULL);

    printf("server: switching to background.\n");
    xmlrpc_server_abyss_run();
    
    return(0);
}








