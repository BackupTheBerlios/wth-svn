/*
  USB-ISS test application for use of HH10D humdity sensor in Linux.

  Sets USB-ISS into I2C mode 
  Reading Version
  Reading SerialNumber
  Reading Coefficients
  - Sensitivity
  - Offset

  Compiled using  gcc, tested on 
  - Ubuntu 10.4 LTS
  - Ubuntu 12.04.1 LTS
  - OpenSUSE 11.4 x86_64

  Compile command
  gcc -Wall -o lm75_usb_iss_i2c lm75_usb_iss_i2c.c 

  technical documentation 
  http://www.robot-electronics.co.uk/htm/usb_iss_tech.htm

  http://elk.informatik.fh-augsburg.de/pub/elinux-abgaben/ss11/Schwarz/Studienarbeit_final/lm75.html
  http://www.spurtikus.de/basteln/datalogger/index.html

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>

void display_version(void);   // Read and display USB-ISS module information
void display_serialnum(void); // Read and display USB-ISS module serial number
void set_i2c_mode(void);      // Set the USB-ISS into I2C mode
void lm75_i2c_rd(void);       // Read LM75 I2C EEPROM
int echodata( unsigned char *data, int ndat);

int fd;
unsigned char sbuf[20];      // serial buffer for r/w
unsigned char error = 0x00;  // Byte used to indicate errors

int main(int argc, char *argv[])
{

  if(argc != 1)
    printf("** Incorrect input! **\n\n");
  else {
    struct termios defaults; // to store innitial default port settings
    struct termios config;   // These will be our new settings
    const char *device = "/dev/ttyACM0";
    fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if(fd == -1) {
      printf( "failed to open port\n" );
    } else {
      if (tcgetattr(fd, &defaults) < 0) 
        perror("tcgetattr"); // Grab snapshot of current settings  for port
      cfmakeraw(&config);  // make options for raw data
      if (tcsetattr(fd, TCSANOW, &config) < 0)
        perror("tcsetattr config"); // Set options for port
		
      display_version();
      display_serialnum();
      set_i2c_mode();
      if(!error) lm75_i2c_rd();
			
      if (tcsetattr(fd, TCSANOW, &defaults) < 0) 
        perror("tcsetattr default");	// Restore port default before closing
    }
    close(fd);
  }
  return 0;
}	

void display_version(void)
{
  sbuf[0] = 0x5A; // USB_ISS byte
  sbuf[1] = 0x01; // Software return byte

  // Write data to USB-ISS
  if (write(fd, sbuf, 2) < 0) 
    perror("display_version write");
  if (tcdrain(fd) < 0)
     perror("display_version tcdrain");
  // Read data back from USB-ISS, module ID and software version
  if (read(fd, sbuf, 3) < 0)
     perror("display_version read"); 

  printf("USB-ISS Module ID: %u \n", sbuf[0]);
  printf("USB-ISS Software v: %u \n", sbuf[1]);
}

void display_serialnum(void)
{
  sbuf[0] = 0x5A; // USB_ISS byte
  sbuf[1] = 0x03; // Serial Number Return Byte

  if (write(fd, sbuf, 2) < 0)
    perror("display_serialnum write");	// Write data to USB-ISS
  if (tcdrain(fd) < 0)
    perror("display_serialnum tcdrain");
  if (read(fd, sbuf, 8) < 0)
     perror("display_serialnum read");	// Read data back from USB-ISS, 7 Bytes Serial Number

  sbuf[8] = '\0';
  printf("USB-ISS Serial Number: %s\n", sbuf);
}

void set_i2c_mode(void)
{
  sbuf[0] = 0x5A;	// USB_ISS command
  sbuf[1] = 0x02;	// Set mode

  //sbuf[2] = 0x20;	// Set mode to 20kHz I2C
  //sbuf[2] = 0x30;	// Set mode to 50kHz I2C
  //sbuf[2] = 0x40;	// Set mode to 100kHz I2C
  sbuf[2] = 0x50;	// Set mode to 400kHz I2C

  sbuf[3] = 0x00;	// Spare pins set to output low

  if (write(fd, sbuf, 4) < 0) 
    perror("set_i2c_mode write"); // Write data to USB-ISS
  if (tcdrain(fd) < 0) 
    perror("set_i2c_mode tcdrain");
  if (read(fd, sbuf, 2) < 0) 
    perror("set_i2c_mode read"); // Read back error byte
  // If first returned byte is not 0xFF then an error has occured
  if(sbuf[0] != 0xFF) {
    printf("**set_i2c_mode: Error setting I2C mode!**\n\n");
    error = 0xFF; // Set error byte
  }
}

void lm75_i2c_rd(void)
{
  int   tempmsb = 0x00;
  int   templsb = 0x00;
  float temp    = 0.00;

/* 
  Read Operation on LM75 EEPROM

  Random Access Read
  using I2C_AD1 command
*/
  sbuf[0] = 0x55; // primary USB-ISS command
  sbuf[1] = 0x91; // device address of LM75: 0xA3 with R/W bit high
  sbuf[2] = 0x00; // 1st internal register of LM75 to read from: 0x00 MSB
  sbuf[3] = 0x02; // read 4 bytes

  printf("send I2C_AD1 command\n");
  echodata( sbuf, 4);


  if (write(fd, sbuf, 4) < 0)
    perror("lm75_i2c_rd write"); // Write data to USB-ISS
  if (tcdrain(fd) < 0)
    perror("lm75_i2c_rd tcdrain");
  if (read(fd, sbuf, 2) < 0)
    perror("lm75_i2c_rd read");  // Read back error byte

  printf("return byte #1 I2C_AD1 command: 0x%X\n", sbuf[0]);
  printf("return byte #2 I2C_AD1 command: 0x%X\n", sbuf[1]);
  /*

  if ( sbuf[0] == 0x0 ) {
    printf("error: returned byte: 0x%X\n", sbuf[0]);
    return;
  } else { 
    printf("returned byte: 0x%X\n", sbuf[0]);
  }
  */

  tempmsb = sbuf[0];
  printf("tempmsb at 0x00: 0x%X\n", tempmsb);

  templsb = sbuf[1];
  printf("templsb at 0x01: 0x%X\n", templsb);

  templsb = templsb >> 7;
  printf("templsb shifted: 0x%X\n", templsb);

  temp = tempmsb + 0.5 * templsb;
  printf("temp: %f\n", temp);

}

int
echodata(unsigned char *data, int mdat) {
    int i;
    char frame[1024] = {'\0'};
    char sf[4];

    printf("echodata : length dataframe : %d\n",mdat);

    /* for better readability label each byte in frame */  
    for ( i = 0; i < mdat; i++ ) {
      snprintf(sf, 4, "%02d:",i);
      strncat(frame, sf, 3);
    }
    printf("echodata : %s\n", frame);    
    strcpy(frame, "");

    for ( i = 0; i < mdat; i++ ) {
      snprintf(sf, 4, "%02x:",data[i]);
      strncat(frame, sf, 4);
    }
    printf("echodata : %s\n", frame);   

    return(0);
} 


