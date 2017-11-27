#ifndef _MINLIST_DEFINED
#define _MINLIST_DEFINED

void minlist_add(char *code, char *string, int style);

int minlist_get_count(char *code);
char *minlist_get_char(char *code, int count);
int minlist_get_style(char *code, int count);

void minlist_write(char *fname);
void minlist_read(char *fname);

#endif /* _MINLIST_DEFINED */
