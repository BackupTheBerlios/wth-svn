#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
 
int main(int argc,char** argv)
{
        struct termios tio;
        int tty_fd;
        fd_set rdset;
        int ndat;
 
        unsigned char c='D';
        unsigned char data[1024];
 
        printf("Please start with ultimeter_test <devicename> , eg. ultimeter_test /dev/ttyS1\n");
 
        memset(&tio,0,sizeof(tio));
        tio.c_iflag=0;
        tio.c_oflag=0;
        tio.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
        tio.c_lflag=0;
        tio.c_cc[VMIN]=1;
        tio.c_cc[VTIME]=0;
 
        tty_fd=open(argv[1], O_RDWR | O_NONBLOCK);      
        cfsetospeed(&tio,B2400);            // 115200 baud
        cfsetispeed(&tio,B2400);            // 115200 baud
 
        tcsetattr(tty_fd,TCSANOW,&tio);
        while (c!='q')
        {

/*
                if (read(tty_fd,&c,255)>0)    
                  printf("c: %02x\n", c);
*/
                if (( ndat = read(tty_fd,&data,1023))>0) {
                  printf("data: %d\n", ndat);
                  printf("data: %s\n", data);
                }

        }
 
        close(tty_fd);
}

