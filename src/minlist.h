#ifndef _MINLIST_DEFINED
#define _MINLIST_DEFINED

void minlist_add(char *code, char *string, int style);

char *minlist_get_string(char *code);
int minlist_get_style(char *code);

void minlist_write(char *fname);
void minlist_read(char *fname);

#endif /* _MINLIST_DEFINED */
