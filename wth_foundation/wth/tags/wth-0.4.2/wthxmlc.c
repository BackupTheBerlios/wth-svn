/*

  Weatherstation client w/ xmlrpc communication 
  uses Eric Kidd's xmlrpc-c

*/


#include <stdio.h>
#include <xmlrpc.h>
#include <xmlrpc_client.h>

#define NAME       "XML-RPC Weatherstation C Client"
#define VERSION    "0.1"
#define SERVER_URL "http://194.59.113.24:8005/RPC2"


/* Check our error-handling environment for an XML-RPC fault. */
void exit_on_fault (xmlrpc_env *env)
{
    if (env->fault_occurred) {
        fprintf(stderr, "XML-RPC Fault: %s (%d)\n",
                env->fault_string, env->fault_code);
        exit(1);
    }
}


int main (int argc, char** argv)
{
  int i;
    xmlrpc_env env;
    xmlrpc_value *result;
    unsigned char* data;
    size_t* datasize;

    /* Start up our XML-RPC client library. */
    xmlrpc_client_init(XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION);
    xmlrpc_env_init(&env);

    printf("xmlrpc_client_call:\n");
    /* Call our XML-RPC server. */
    result = xmlrpc_client_call(&env, SERVER_URL,
                                "poll.DCFTime", "(ii)",
                                (xmlrpc_int32) 5,
                                (xmlrpc_int32) 3);
    printf("done\n");
    exit_on_fault(&env);

    /* Parse our result value. */
    printf("xmlrpc_parse_value\n");
    xmlrpc_parse_value(&env, result, "{s:6,*}",
                       "data", &data, &datasize);
    printf("done\n");
    exit_on_fault(&env);

    /* echo communication result */
    printf("ndata: %d, datasize: %u\n", strlen(data), (int) datasize);
    for ( i = 0; i < (int) datasize ; i++ ) {
      printf("%x:", data[i]);
    }
    printf("\n");

    /* Dispose of our result value. */
    xmlrpc_DECREF(result);

    /* Shutdown our XML-RPC client library. */
    xmlrpc_env_clean(&env);
    xmlrpc_client_cleanup();
    

    return 0;
}













