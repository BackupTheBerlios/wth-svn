/* testing time stuff */
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define MAXBUFF 1024

int
main (int argc, char **argv) {
    time_t mtime;
    struct tm *ctm;
    char clk[MAXBUFF];

    tzset(); /* setting timezone */
    time(&mtime);
    ctm = gmtime(&mtime);
    /* clk = ctime(&mtime); */
    strftime(clk, sizeof(clk), "%c", ctm);
    printf("%s\n", clk);

    return(0);
}
