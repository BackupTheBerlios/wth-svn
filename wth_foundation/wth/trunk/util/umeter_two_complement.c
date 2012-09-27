/*

  umeter_two_complement.c

  calculating negative measurement value from data value
  which are output as two complement

  data values are 8 Bit output is hexadecimal format as string of ascii

  compile command
  gcc -Wall -o umeter_two_complement umeter_two_complement.c

*/
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

/*
  bitprint 

  utility function to print bits of 8-bit byte
*/
int 
bitprint ( int byte, char *s_reg ) {
  int x;
  struct timezone tz;
  struct timeval  tv;

  gettimeofday(&tv, &tz);
  printf("%5s | %lu.%lu : #", s_reg, ( long unsigned int)tv.tv_sec, tv.tv_usec);
  for( x = 7; x > -1; x-- )
    printf( "%i", (byte & 1 << x ) > 0 ? 1 : 0 );
  printf( "b\n" );
  return(0);
}


int
main(int argc, char *argv[])
{
  int base, isneg;
  char *endptr, *str;
  int val;  // int is _not_ 8 bit 

  if (argc < 2) {
    fprintf(stderr, "Usage: %s str [base]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  str = argv[1];
  base = (argc > 2) ? atoi(argv[2]) : 10;

  errno = 0;    /* To distinguish success/failure after call */
  val = strtol(str, &endptr, base);

  /* Check for various possible errors */

  if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
            || (errno != 0 && val == 0)) {
    perror("strtol");
    exit(EXIT_FAILURE);
  }

  if (endptr == str) {
    fprintf(stderr, "No digits were found\n");
    exit(EXIT_FAILURE);
  }

  /* If we got here, strtol() successfully parsed a number */
  printf("strtol() returned %d\n", val);
  printf("strtol() returned %x\n", val);

  bitprint( val, "val: unmodified");

  isneg = ( val >> 15 ) & 1;
  if ( isneg == 1) {
    printf("isneg: %d: value is negative number!\n", isneg);
    
    /* 
    val = val - 1;
    printf("val minus 1: %x\n", val);
    bitprint( val, "val: minus 1");
    val = 0xffff0000 + val;
    val = ~ val;
    printf("val inverted: %x\n", val);
    bitprint( val, "val: inverted");
    */
     
    val = ~ (0xfffeffff + val );
    printf("val inverted: %x\n", val);
    bitprint( val, "val: inverted");

  } else {
    printf("isneg: %d: value is positive number!\n", isneg);
  }
   
  /* string represents a negative number ? */
  /*
  if ( val > 0x7fffff) {
    val |= 0xff000000;
    printf("strtol() returned %ld\n", val);
  }
  */

  if (*endptr != '\0')        /* Not necessarily an error... */
    printf("Further characters after number: %s\n", endptr);

  exit(EXIT_SUCCESS);
}

