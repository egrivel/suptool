#ifndef _CHARMATCH_DEFINED
#define _CHARMATCH_DEFINED

#include "common.h"
#include "bitmap.h"

char *bitmap_to_code(Bitmap bm);
Bitmap code_to_bitmap(char *code);
unsigned char *code_to_bytes(char *code, int *length);
void dump_bitmap(Bitmap bm);
void dump_code(char *code);

#endif /* _CHARMATCH_DEFINED */
