#ifndef _OUTPUT_DEFINED
#define _OUTPUT_DEFINED

#define FORMAT_SRT 1
#define FORMAT_ASS 2

void output_open(char *fname, int format, int width, int height);
void output_close();
void output_start_item(long start_time, long end_time, position pos);
void output_string(char *text, int style);
void output_newline();
void output_end_item();
bool output_is_italic();

#endif /* _OUTPUT_DEFINED */
