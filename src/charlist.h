#ifndef _CHARLIST_INCLUDED
#define _CHARLIST_INCLUDED

#include "bitmap.h"

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

char *charlist_find_by_string(char *string, int style);
int charlist_nr_entries();
char *charlist_get_code(int nr);

#endif /* _CHARLIST_INCLUDED */
