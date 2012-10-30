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
  gcc -Wall -o linux_usb_iss_i2c linux_usb_iss_i2c.c 

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
void set_i2c_mode(void);      // Set the USB-ISS into I2C mode, 100KHz clock
void hh10d_i2c_rd(void);      // Read HH10D I2C EEPROM
void hh10d_freq_rd(void);     // Read HH10D frequency
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
      if(!error) hh10d_i2c_rd();
			
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

void hh10d_i2c_rd(void)
{
  int sensmsb = 0x00;
  int senslsb = 0x00;
  int offmsb  = 0x00;
  int offlsb  = 0x00; 
  int retbyte;
/*
  sbuf[0] = 0x55; // primary USB-ISS command
  sbuf[1] = 0xA4; // address of HH10D 0xA3 + 1 with R/W bit high
  sbuf[2] = 0x0A; // 1st internal register of SRF08 to read from: 0x0A sens MSB
  sbuf[3] = 0x04; // number of bytes we wish to read
*/

/* 
  Read Operation on M24C02

  Random Access Read
  using I2C_AD1 command
*/
  sbuf[0] = 0x55; // primary USB-ISS command
  sbuf[1] = 0xA3; // device address of HH10D: 0xA2 with R/W bit low
  sbuf[2] = 0x0A; // 1st internal register of HH10D to read from: 0x0A sens MSB
  sbuf[3] = 0x04; // read 4 bytes

  printf("send I2C_AD1 command\n");
  echodata( sbuf, 4);


  if (write(fd, sbuf, 4) < 0)
    perror("hh10d_i2c_rd write"); // Write data to USB-ISS
  if (tcdrain(fd) < 0)
    perror("hh10d_i2c_rd tcdrain");
  if (read(fd, sbuf, 4) < 0)
    perror("hh10d_i2c_rd read");  // Read back error byte

  printf("return byte #1 I2C_AD1 command: 0x%X\n", sbuf[0]);
  printf("return byte #2 I2C_AD1 command: 0x%X\n", sbuf[1]);
  printf("return byte #3 I2C_AD1 command: 0x%X\n", sbuf[2]);
  printf("return byte #4 I2C_AD1 command: 0x%X\n", sbuf[3]);
  /*

  if ( sbuf[0] == 0x0 ) {
    printf("error: returned byte: 0x%X\n", sbuf[0]);
    return;
  } else { 
    printf("returned byte: 0x%X\n", sbuf[0]);
  }
  */

/*
  Read Operation on M24C02

  Random Access Read
  using I2C_DIRECT command

*/
/*
  sbuf[0] = 0x57; // I2C_DIRECT command
  sbuf[1] = 0x01; // send start sequence
  sbuf[2] = 0x31; // write 2 bytes
  sbuf[3] = 0xA2; // EEPROM write access 0xA2
  sbuf[4] = 0x0B; // 1st address to read

  sbuf[5] = 0x01; // send 2nd start sequence
  sbuf[6] = 0x30; // write 1 byte
  sbuf[7] = 0xA3; // EEPROM read access 0xA3
  sbuf[8] = 0x20; // read 1 bytes
  sbuf[9] = 0x04; // send NACK
  sbuf[10] = 0x03; // send stop sequence

  printf("send I2C_DIRECT command\n");
  echodata( sbuf, 11);

  if (write(fd, sbuf, 11) < 0)
    perror("hh10d_i2c_rd write"); // Write data to USB-ISS
  if (tcdrain(fd) < 0)
    perror("hh10d_i2c_rd tcdrain");
  if (read(fd, sbuf, 2) < 0)
    perror("hh10d_i2c_rd read");  // Read back error byte
  printf("return byte #1 I2C_DIRECT command: 0x%X\n", sbuf[0]);
  printf("return byte #2 I2C_DIRECT command: 0x%X\n", sbuf[1]);
*/
/*
  Current Address Read
  using I2C_AD0 command
*/
/*
  sbuf[0] = 0x54; // primary USB-ISS command
  sbuf[1] = 0xA3; // address of HH10D 0xA3 + 1 with R/W bit high
  sbuf[3] = 0x01; // number of bytes we wish to read

  echodata( sbuf, 3);

  if (write(fd, sbuf, 3) < 0)
    perror("hh10d_i2c_rd write"); // Write data to USB-ISS
  if (tcdrain(fd) < 0)
    perror("hh10d_i2c_rd tcdrain");
  if (read(fd, sbuf, 2) < 0)
    perror("hh10d_i2c_rd read");  // Read back error byte
  printf("return byte #1 I2C_AD0 command: 0x%X\n", sbuf[0]);
  printf("return byte #2 I2C_AD0 command: 0x%X\n", sbuf[1]);

*/
/*
  sbuf[0] = 0x56; // primary USB-ISS command
  sbuf[1] = 0xA3; // address of HH10D 0xA3 + 1 with R/W bit high
  sbuf[2] = 0x0D; // 1st internal register of SRF08 to read from: 0x0A sens MSB
  sbuf[3] = 0x0A; // 1st internal register of SRF08 to read from: 0x0A sens MSB
  sbuf[4] = 0x04; // number of bytes we wish to read

  if (write(fd, sbuf, 5) < 0)
    perror("hh10d_i2c_rd write"); // Write data to USB-ISS
  if (tcdrain(fd) < 0)
    perror("hh10d_i2c_rd tcdrain");
  if (read(fd, sbuf, 4) < 0)
    perror("hh10d_i2c_rd read");  // Read back error byte
*/

/*
  sbuf[0] = 0x53; // primary USB-ISS command I2C_SGL
  sbuf[1] = 0xA2; // address of HH10D 0xA2 + 0 with R/W bit low
  sbuf[2] = 0x0A; // 1st internal register of HD10D to read from: 0x0A sens MSB

  if (write(fd, sbuf, 3) < 0)
    perror("hh10d_i2c_rd write"); // Write data to USB-ISS
  if (tcdrain(fd) < 0)
    perror("hh10d_i2c_rd tcdrain");
  if (read(fd, sbuf, 1) < 0)
    perror("hh10d_i2c_rd read");  // Read back error byte

  retbyte = sbuf[0];
  printf("return byte I2C_SGL command: 0x%X\n", retbyte);
*/

/*
  sensmsb = sbuf[0];
  printf("sensmsb at 0x0A: 0x%X\n", sensmsb);
  senslsb = sbuf[1];
  printf("senslsb at 0x0B: 0x%X\n", senslsb);
  offmsb  = sbuf[2];
  printf("offmsb at 0x0B: 0x%X\n", offmsb);
  offlsb = sbuf[3];
  printf("offlsb at 0x0B: 0x%X\n", offlsb);
*/

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


