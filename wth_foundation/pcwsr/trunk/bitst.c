/*
  bit testing for pc weathersensor receiver
*/


int
main(int argc, char **argv) {
    int abyte, bbyte, cbyte;
    int mask, shift;

    mask = 0x7f;
    shift = 0x0;
 
    abyte = 0xf0;

    printf("abyte: %x, mask: %x, abyte & mask: %x\n", 
        abyte, mask, abyte & mask);
    return(0);
}
