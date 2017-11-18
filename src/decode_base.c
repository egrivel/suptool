#include "common.h"
#include "charlist.h"

int main(int argc, char *argv[]) {
   int i;
   for (i = 1; i < argc; i++) {
     // decode_base(argv[i]);
     int length;
     unsigned char *outbuf = decode_string_to_bytes(argv[i], &length);
     int j;
     for (j = 0; j < length; j++) {
       printf("0x%02x ", outbuf[j]);
     }
     printf("\n");
     free(outbuf);
   }
   return 0;
}
