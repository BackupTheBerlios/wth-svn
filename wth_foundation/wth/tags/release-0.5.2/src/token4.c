/*

  token4.c
  token handling in wthcmd.c 

  compile command
  gcc -Wall -o token4 token4.c

  $Id$

  Copyright 2008, Volker Jahns, volker@thalreit.de

*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXMSGLEN 1024

char *
tnhelp( char *args) {
  int err, ntok;
  int is_show, is_init, is_exec;
  int is_conf, is_data, is_stat;
  char *rbuf;
  char *sbuf;
  char *sep = "\\/:;=- ";
  char *token;

  if ( ( rbuf = malloc(MAXMSGLEN+1)) == NULL )
    return NULL;

  //printf("tnhelp: input string: \"%s\"\n", args);
  if ( args == NULL ) {
    snprintf(rbuf,MAXMSGLEN, "tnhelp: provide general HELP\n");
    return(rbuf);
  }

  ntok=0; 
  token = strtok_r(args, sep, &sbuf);

  do {
    ntok++;
    if ( ( err = strncmp(token, "show", 4) == 0 )) {
      is_show = 1;
    } else if ( ( err = strncmp(token, "reload", 4) == 0 )) {
      is_init = 1;
    } else if ( ( err = strncmp(token, "restart", 4) == 0 )) {
      is_init = 1;
    } else if ( ( err = strncmp(token, "exec", 4) == 0 )) {
      is_exec = 1;
    } else if ( ( err = strncmp(token, "exec", 4) == 0 )) {
      is_exec = 1;
    } else if ( ( err = strncmp(token, "config", 4) == 0 )) {
      is_conf = 1;
    } else if ( ( err = strncmp(token, "data", 4) == 0 )) {
      is_data = 1;
    } else if ( ( err = strncmp(token, "stat", 4) == 0 )) {
      is_stat = 1;
    }
  } while ( ( token = strtok_r( NULL, sep, &sbuf)) != NULL );

  if ( is_show == 1) {
    if ( is_data == 1) {
      snprintf(rbuf, MAXMSGLEN, "thhelp: provide help on SHOW DATA");
    } else if ( is_stat == 1) {
      snprintf(rbuf, MAXMSGLEN, "thhelp: provide help on SHOW STATUS");
    } else if ( is_conf == 1) {
      snprintf(rbuf, MAXMSGLEN, "thhelp: provide help on SHOW CONFIG");
    } else {
      snprintf(rbuf, MAXMSGLEN, "thhelp: provide general help on SHOW");
    }
  } else if ( is_init == 1 ) {
    snprintf(rbuf, MAXMSGLEN, "tnhelp: provide help on RESTART/RELOAD");
  } else if ( is_exec == 1) {
    snprintf(rbuf, MAXMSGLEN, "tnhelp: provide help on EXEC");
  } else {  // catch all 
    snprintf(rbuf, MAXMSGLEN, "thhelp: provide general HELP");
  }
  return(rbuf);
}


char *
execcmd( char *args) {
  int err, ntok;
  int is_ws2000;
  int ncmd;
  char *rbuf;
  char *sbuf;
  char *sep = "\\/:;=- ";
  char *token;

  if ( ( rbuf = malloc(MAXMSGLEN+1)) == NULL )
    return NULL;

  if ( args == NULL ) {
    snprintf(rbuf,MAXMSGLEN, "execcmd: provide HELP (called w/o all args)");
    return(rbuf);
  }

  ntok=0; 
  token = strtok_r(args, sep, &sbuf);
  do {
    ntok++;
    if ( ( err = strncmp(token, "ws2000", 4) == 0 )) {
      is_ws2000 = 1;
    } else if ( ( err = strncmp(token, "polldcftime", 4) == 0 )) {
      ncmd = 0;
    } else if ( ( err = strncmp(token, "requestdataset", 4) == 0 )) {
      ncmd = 1;
    } else if ( ( err = strncmp(token, "selectnextdataset", 4) == 0 )) {
      ncmd = 2;
    } else if ( ( err = strncmp(token, "set9sensor", 4) == 0 )) {
      ncmd = 3;
    } else if ( ( err = strncmp(token, "set16sensor", 5) == 0 )) {
      ncmd = 4;
    } else if ( ( err = strncmp(token, "pollstatus", 4) == 0 )) {
      ncmd = 5;
    } else if ( ( err = strncmp(token, "setintervaltime", 4) == 0 )) {
      ncmd = 6;
    } else if ( token == NULL ) {
      ncmd = 100;
    } else { /* no command in arguments */
      ncmd = 100;
    }
  } while ( ( token = strtok_r( NULL, sep, &sbuf)) != NULL );

  if ( is_ws2000 == 1) {
    if ( ntok > 1 ) { 
      snprintf(rbuf, MAXMSGLEN, "execcmd: ws2000: execute cmd %d", ncmd);
    } else if ( ncmd == 100) {
      snprintf(rbuf, MAXMSGLEN, "execcmd: provide HELP (ws2000 called w/o command)");
    } else {
      snprintf(rbuf, MAXMSGLEN, "execcmd: provide HELP (ws2000 command unknown)");
    }
  } else {  // catch all 
    snprintf(rbuf, MAXMSGLEN, 
      "execcmd: provide general HELP (only ws2000 supported)");
  }
  return(rbuf);
}

char *
showcmd( char *args) {
  int err, ntok;
  int is_ws2000, is_pcwsr, is_1wire;
  int is_data, is_stat, is_conf;
  char *rbuf;
  char *sbuf;
  char *sep = "\\/:;=- ";
  char *token;

  if ( ( rbuf = malloc(MAXMSGLEN+1)) == NULL )
    return NULL;

  if ( args == NULL ) {
    snprintf(rbuf,MAXMSGLEN, "showcmd: provide HELP (called w/o all args");
    return(rbuf);
  }

  ntok=0; 
  token = strtok_r(args, sep, &sbuf);
  do {
    ntok++;
    if ( ( err = strncmp(token, "data", 4) == 0 )) {
      is_data = 1;
    } else if ( ( err = strncmp(token, "status", 4) == 0 )) {
      is_stat = 1;
    } else if ( ( err = strncmp(token, "config", 4) == 0 )) {
      is_conf = 1;
    } else if ( ( err = strncmp(token, "ws2000", 4) == 0 )) {
      is_ws2000 = 1;
    } else if ( ( err = strncmp(token, "pcwsr", 4) == 0 )) {
      is_pcwsr = 1;
    } else if ( ( err = strncmp(token, "1wire", 5) == 0 )) {
      is_1wire = 1;
    }
  } while ( ( token = strtok_r( NULL, sep, &sbuf)) != NULL );

  if ( is_data == 1) {
    if ( is_ws2000 == 1 ) { 
      snprintf(rbuf, MAXMSGLEN, "showcmd: show data ws2000");
    } else if ( is_pcwsr == 1) {
      snprintf(rbuf, MAXMSGLEN, "showcmd: show data pcwsr");
    } else if ( is_1wire == 1) {
      snprintf(rbuf, MAXMSGLEN, "showcmd: show data 1wire)");
    } else {
      snprintf(rbuf, MAXMSGLEN, "showcmd: provide HELP (no weatherstation specified)"); 
    }
  } else if ( is_stat == 1) { 
    if ( is_ws2000 == 1 ) { 
      snprintf(rbuf, MAXMSGLEN, "showcmd: show status ws2000");
    } else if ( is_pcwsr == 1) {
      snprintf(rbuf, MAXMSGLEN, "showcmd: show status pcwsr");
    } else if ( is_1wire == 1) {
      snprintf(rbuf, MAXMSGLEN, "showcmd: show status 1wire)");
    } else {
      snprintf(rbuf, MAXMSGLEN, "showcmd: provide HELP (no weatherstation specified)"); 
    }
  } else if ( is_conf == 1) { 
    if ( is_ws2000 == 1 ) { 
      snprintf(rbuf, MAXMSGLEN, "showcmd: show config ws2000");
    } else if ( is_pcwsr == 1) {
      snprintf(rbuf, MAXMSGLEN, "showcmd: show config pcwsr");
    } else if ( is_1wire == 1) {
      snprintf(rbuf, MAXMSGLEN, "showcmd: show config 1wire)");
    } else {
      snprintf(rbuf, MAXMSGLEN, "showcmd: provide HELP (no weatherstation specified)"); 
    }
  } else {  // catch all 
    snprintf(rbuf, MAXMSGLEN, 
      "execcmd: provide general HELP (only ws2000 supported)");
  }
  return(rbuf);
}

char *
initcmd( char *args) {
  int err;
  int is_reload, is_restart;
  char *rbuf;

  if ( ( rbuf = malloc(MAXMSGLEN+1)) == NULL )
    return NULL;

  if ( args == NULL ) {
    snprintf(rbuf,MAXMSGLEN, "initcmd: provide HELP (called w/o all args)");
  } else if ( ( err = strncmp(args, "reload", 4)) == 0 ) {
    is_reload = 1;
  } else if ( ( err = strncmp(args, "restart", 4)) == 0 ) {
    is_restart = 1;
  }

  
  if ( is_reload == 1) {
    snprintf(rbuf, MAXMSGLEN, "initcmd: reload"); 
  } else if ( is_restart == 1) { 
    snprintf(rbuf, MAXMSGLEN, "initcmd: restart"); 
  } else { 
    snprintf(rbuf, MAXMSGLEN, "initcmd: provide HELP"); 
  }
  return(rbuf);

}

char *
tnusage( ) {

  char *rbuf;
  if ( ( rbuf = malloc(MAXMSGLEN+1)) == NULL )
    return NULL;
  snprintf(rbuf, MAXMSGLEN, "usage: help");

  return(rbuf);
}


int main ( int argc, char **argv) {
  int i, err, ntok;
  int maxcmd = 20;
  char readline[][1024] = { "help show data", 
                            "help show stat", 
                            "help show k0l\n", 
                            "help show", 
                            "help exec", 
                            "help reload", 
                            "help restart",
                            "help nocmd", 
                            "help",
                            "exec ws2000 polldcftime", 
                            "exec ws2000 requestdataset", 
                            "exec ws2000", 
                            "exec", 
                            "show data ws2000", 
                            "show status ws2000",
                            "show config ws2000", 
                            "show", 
                            "reload", 
                            "restart",
                            "nocmd"
                           }; 


  char *rbuf;
  char *sbuf;
  char *token;
  char *sep = "\\/:;=- ";

  /* loop over all test inputs */
  for ( i = 0; i< maxcmd; i++) {
    rbuf = strdup(readline[i]);
    printf("main: inputstring: \"%s\"\n", rbuf);
    ntok=0; token = strtok_r(rbuf, sep, &sbuf);
    if ( ( err = strncmp( token,"help",4)) == 0) {
      printf("main: %s\n\n", tnhelp( sbuf)); 
    } else if ( ( err = strncmp( token, "exec",4)) == 0 ) {
      printf("main: %s\n\n", execcmd( sbuf)); 
    } else if ( ( err = strncmp( token, "show",4)) == 0 ) {
      printf("main: %s\n\n", showcmd(sbuf)); 
    } else if ( ( err = strncmp( token, "reload",4)) == 0 ) {
      printf("main: %s\n\n", initcmd(token)); 
    } else if ( ( err = strncmp( token, "restart",4)) == 0 ) {
      printf("main: %s\n\n", initcmd(token));
      initcmd(sbuf); 
    } else if ( token == NULL) {
      printf("main: token NULL\n");
      tnusage(); 
    } else {
      printf("main: provide HELP (unknown command)\n");
    }
  }
  return 0;
}

