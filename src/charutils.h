#ifndef _CHARMATCH_DEFINED
#define _CHARMATCH_DEFINED

#include "common.h"
#include "bitmap.h"

char *bytes_to_code(unsigned char *bytes, int length);
unsigned char *code_to_bytes(char *code, int *length);

char *bitmap_to_code(Bitmap bm, int baseline);
Bitmap code_to_bitmap(char *code);

void dump_bitmap(Bitmap bm);
void dump_code(char *code);

#endif /* _CHARMATCH_DEFINED */
