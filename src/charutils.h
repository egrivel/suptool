#ifndef _CHARMATCH_DEFINED
#define _CHARMATCH_DEFINED

#include "common.h"
#include "bitmap.h"

char *bytes_to_code(unsigned char *bytes, int length);
unsigned char *code_to_bytes(char *code, int *length);

char *bitmap_to_code(Bitmap bm, int baseline);
Bitmap code_to_bitmap(char *code);
int code_to_baseline(char *code);

Bitmap bitmap_to_minimal(Bitmap bm, int baseline, int x_width, int x_height);

void dump_bitmap(Bitmap bm);
void dump_code(char *code);

#endif /* _CHARMATCH_DEFINED */
