#include "common.h"
#include "process.h"

#define NR_POST 20
char *post_str[NR_POST];
char *post_replace[NR_POST];
int nr_post = 0;

void add_postprocess(char *str, char *replace) {
   if (nr_post < NR_POST) {
      post_str[nr_post] = strdup(str);
      post_replace[nr_post] = strdup(replace);
      nr_post++;
   } else {
      printf("WARNING: reached maximum %d of post processing settings\n", NR_POST);
   }
}

//
// Postprocessing is done on a *static* buffer
//
void do_postprocess(char *str) {
   int i;
   for (i = 0; i < nr_post; i++) {
      char *offset = strstr(str, post_str[i]);
      if (offset) {
         char *temp = malloc(strlen(str) + strlen(post_replace[i]) + 1);
         int len = 0;
         if (offset > str) {
            memcpy(temp + len, str, (offset - str));
            len += (offset - str);
         }
         memcpy(temp + len, post_replace[i], strlen(post_replace[i]));
         len += strlen(post_replace[i]);
         strcpy(temp + len, offset + strlen(post_str[i]));
         strcpy(str, temp);
         free(temp);
      }
   }
}

