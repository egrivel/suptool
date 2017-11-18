#ifndef _CHARLIST_INCLUDED
#define _CHARLIST_INCLUDED

#include "bitmap.h"

#define STYLE_UNKNOWN  0x00
#define STYLE_NORMAL   0x01
#define STYLE_ITALIC   0x02
#define STYLE_EITHER   0x03

bool has_char(char *code);
void add_char(char *code);
char *get_char_string(char *code);
void set_char_string(char *code, char *string);
void read_char_data(char *fname);
int charlist_get_style(char *code);
char *charlist_get_style_name(char *code);
void charlist_set_style(char *code, int style);
char *encode_bitmap_base(Bitmap bm,
                         int line_start, int line_end,
                         int char_start, int char_end,
                         int baseline);
void decode_base(char *code);
char *do_postprocess(char *str);
unsigned char *decode_string_to_bytes(char *str, int *length);

#endif /* _CHARLIST_INCLUDED */
