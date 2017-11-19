#include "common.h"
#include "charutils.h"

int main(int argc, char *argv[]) {
   int i;
   for (i = 1; i < argc; i++) {
     int length;
     unsigned char *outbuf = code_to_bytes(argv[i], &length);
     int j;
     for (j = 0; j < length; j++) {
       printf("0x%02x ", outbuf[j]);
     }
     printf("\n");
     free(outbuf);
   }
   return 0;
}
