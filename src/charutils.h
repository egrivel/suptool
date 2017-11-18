#ifndef _CHARMATCH_DEFINED
#define _CHARMATCH_DEFINED

#include "common.h"
#include "bitmap.h"

char *bitmap_to_string(Bitmap bm);
Bitmap string_to_bitmap(char *string);
void dump_bitmap(Bitmap bm);
void dump_string(char *string);

#endif /* _CHARMATCH_DEFINED */
