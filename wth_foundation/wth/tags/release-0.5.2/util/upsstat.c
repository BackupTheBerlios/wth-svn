/*
 * Copyright 1996 Doug Hughes Auburn University
 * @(#)upsstat.c	1.3 08/26/96	this notice must be left intact
 * every second print in a nice table which RS-232 lines are on
 * and which are off. (X means on)
 */
#include <stdio.h>
#include <sys/time.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/signal.h>
#include <sys/stat.h>

main(int argc, char *argv[]) 
{
	int fd;
	int flag, i, j;
	struct stat sb;
	char devname[20]="/dev/";
	char status[40];

	if (argc < 2) {
		fprintf(stderr, "must supply device name (e.g. term/a, ttya)\n");
		exit(1);
	}

	if (strcmp(argv[1], "-ttytab") == 0) {
		fd = 0;		/* terminal has already been opened as fd 0 */
					/* This is for experimental support of SunOS */
	} else  {
		strcat(devname, argv[1]);
		if ((fd = open(devname, O_RDONLY| O_NOCTTY | O_NONBLOCK )) < 0) {
			fprintf(stderr, "couldn't open device: %s\n", devname);
			exit(1);
		}
	}
	
	/* Get terminal modes */
	i = TIOCM_DTR;
	/* ioctl(fd, TIOCMSET, &i); */

	i=0;
	flag=0;
	while (1) {		/* infinite loop */
		if (i % 22  == 0) printf("LE DTR RTS ST SR CTS CD RNG DSR\n");
		sprintf(status,          "O   O   O  O  O   O  O   O   O   ");
		i++;
		if (ioctl(fd, TIOCMGET, &j) < 0) {
			perror("Getting hardware status bits");
			exit(1);
		}
		if (j & TIOCM_LE) status[0] = 'X'; 
		if (j & TIOCM_DTR) status[4] = 'X';
		if (j & TIOCM_RTS) status[8] = 'X';
		if (j & TIOCM_ST) status[11] = 'X';
		if (j & TIOCM_SR) status[14] = 'X';
		if (j & TIOCM_CTS) status[18] = 'X';
		if (j & TIOCM_CAR) status[21] = 'X';
		if (j & TIOCM_RNG) status[25] = 'X';
		if (j & TIOCM_DSR) status[29] = 'X';
		printf("%s\n", status);
		sleep(1);
	}
}
