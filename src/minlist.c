#include <stdio.h>

#include "common.h"
#include "minlist.h"

// Separator to use between different character strings
#define SEPARATOR      " "
#define SEPARATOR_CHAR ' '

typedef struct {
  char *code;
  char *string;
  int style;
} MinItem;

MinItem *minlist = NULL;
int minlist_items = 0;
int minlist_capacity = 0;

MinItem *minlist_find(char *code) {
  int i;
  for (i = 0; i < minlist_items; i++) {
    if (!strcmp(minlist[i].code, code)) {
      return &minlist[i];
    }
  }
  return NULL;
}

void minlist_add_string_to_item(MinItem *item, char *string, int style) {
  if (item->style != style) {
    item->style = STYLE_EITHER;
  }

  printf("Code %s: look for '%s' in '%s'\n", item->code, string, item->string);
  // create a buffer large enough for the new item->string if we need it
  char *buffer = malloc(strlen(item->string) + strlen(string) + 3);
  strcpy(buffer, SEPARATOR);
  strcat(buffer, string);
  strcat(buffer, SEPARATOR);
  if (strstr(item->string, buffer)) {
    // string is already in there, nothing left to do
    printf("   already exists\n");
    free(buffer);
    return;;
  }

  // Create the new version of item->string
  strcpy(buffer, item->string);
  strcat(buffer, string);
  strcat(buffer, SEPARATOR);

  // destroy the old version
  free(item->string);;
  item->string = buffer;
  printf("   added: '%s'\n", item->string);
}

void minlist_add(char *code, char *string, int style) {
  MinItem *item = minlist_find(code);
  if (item) {
    minlist_add_string_to_item(item, string, style);
    return;
  }

  if (minlist_items >= minlist_capacity) {
    if (minlist_capacity == 0) {
      minlist_capacity = 32;
      minlist = malloc(minlist_capacity * sizeof(MinItem));
    } else {
      MinItem *oldlist = minlist;
      minlist = malloc(2 * minlist_capacity * sizeof(MinItem));
      memcpy(minlist, oldlist, minlist_capacity * sizeof(MinItem));
      free(oldlist);
      minlist_capacity = 2 * minlist_capacity;
    }
  }

  if (minlist_items < minlist_capacity) {
    minlist[minlist_items].code = strdup(code);
    minlist[minlist_items].string = malloc(strlen(string) + 3);
    strcpy(minlist[minlist_items].string, SEPARATOR);
    strcat(minlist[minlist_items].string, string);
    strcat(minlist[minlist_items].string, SEPARATOR);
    minlist[minlist_items].style = style;
    minlist_items++;
  }

  printf("Item %d becomes '%s'\n", minlist_items - 1, minlist[minlist_items - 1].string);
}

void minlist_write(char *fname) {
  FILE *fout = fopen(fname, "w");
  int i;
  for (i = 0; i < minlist_items; i++) {
    fprintf(fout, "%s.ch = %s\n", minlist[i].code, minlist[i].string);
    fprintf(fout, "%s.style = %s\n", minlist[i].code,
     get_style_name(minlist[i].style));
  }
  fclose(fout);
}

void minlist_read(char *fname) {
  static char buffer[16384];
  strcpy(buffer, "");
  FILE *fin = fopen(fname, "rt");
  if (fin == NULL) {
    printf("Cannot read character data '%s'\n", fname);
    return;
  }
  while (!feof(fin)) {
    fgets(buffer, sizeof(buffer), fin);
    if (strlen(buffer) > 0) {
      int code_start = 0;
      while (buffer[code_start] == ' ') {
        code_start++;
      }
      if (buffer[code_start] != '#') {
        int code_end = 0;
        int type_start = 0;
        int type_end = 0;
        int value_start = 0;
        int value_end = 0;
        code_end = code_start;
        while (buffer[code_end] && (buffer[code_end] != '.')) {
          code_end++;
        }
        if (buffer[code_end] == '.') {
          // Note: code_end can become -1 if line starts with 0
          code_end--;
          type_start = code_end + 2;
        }
        type_end = type_start;
        while (buffer[type_end]
               && (buffer[type_end] != ' ')
              && (buffer[type_end] != '=')) {
          type_end++;
        }
        type_end--;
        value_start = type_end + 1;
        while ((buffer[value_start] == ' ')
               || (buffer[value_start] == '=')) {
          value_start++;
        }
        value_end = value_start;
        while (buffer[value_end]
               && (buffer[value_end] != '\r')
               && (buffer[value_end] != '\n')) {
          value_end++;
        }
        value_end--;

        buffer[code_end + 1] = '\0';
        buffer[type_end + 1] = '\0';
        buffer[value_end + 1] = '\0';

        if (!strcmp(&buffer[type_start], "ch")) {
          if (buffer[value_start] == SEPARATOR_CHAR) {
            value_start++;
          }
          if (buffer[value_end] == SEPARATOR_CHAR) {
            buffer[value_end] = '\0';
          }
          minlist_add(&buffer[code_start], &buffer[value_start], STYLE_UNKNOWN);
        } else if (!strcmp(&buffer[type_start], "style")) {
          MinItem *item = minlist_find(&buffer[code_start]);
          if (item && item->style == STYLE_UNKNOWN) {
            if (!strcmp(&buffer[value_start], "normal")) {
              item->style = STYLE_NORMAL;
            } else if (!strcmp(&buffer[value_start], "italic")) {
              item->style = STYLE_ITALIC;
            } else if (!strcmp(&buffer[value_start], "either")) {
              item->style = STYLE_EITHER;
            }
          }
        }
      }
    }
  }
  fclose(fin);
}
