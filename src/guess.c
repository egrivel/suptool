#include "common.h"
#include "charlist.h"
#include "charutils.h"
#include "bitmap.h"
#include "minlist.h"

#define MAX_WIDTH 128
#define MAX_HEIGHT 128

void make_guess(char *code, int x_width, int x_height) {
  Bitmap bm = code_to_bitmap(code);
  int baseline = code_to_baseline(code);
  bitmap_set_baseline(bm, baseline);


  Bitmap medium_bm = bitmap_to_medium(bm, baseline, x_width, x_height);
  char *medium_code = bitmap_to_code(medium_bm,
    bitmap_get_baseline(medium_bm));
  char *string = minlist_get_string(medium_code);
  int style = minlist_get_style(medium_code);
  bitmap_destroy(medium_bm);

  if (string == NULL) {
    // Didn't find it in medium, try minimal
    Bitmap minimal_bm = bitmap_to_minimal(bm, baseline, x_width, x_height);
    char *minimal_code = bitmap_to_code(minimal_bm,
      bitmap_get_baseline(minimal_bm));
    string = minlist_get_string(minimal_code);
    style = minlist_get_style(minimal_code);
    bitmap_destroy(minimal_bm);
  }

  if (string == NULL) {
    string = "";
  }

  // Check for the number of spaces in the string. Normally, there will be
  // two spaces (one before the letter, one after). If there are more than
  // two spaces, the template wasn't conclusive. In that case, change the
  // style back to "unknown" to indicate that this entry must still be
  // looked at.
  int space_count, i;
  space_count = 0;
  for (i = 0; string[i]; i++) {
    if (string[i] == ' ') {
      space_count++;
    }
  }
  if (space_count > 2) {
    style = 0;
  }

  printf("%s.ch = %s\n", code, string);
  printf("%s.style = %s\n", code, get_style_name(style));

  bitmap_destroy(bm);
}

void process_file(char *fname, int width, int height) {
  static char buffer[16384];
  strcpy(buffer, "");

  FILE *fin = fopen(fname, "rt");
  if (fin == NULL) {
    printf("Cannot read input file '%s'\n", fname);
    return;
  }

  while (!feof(fin)) {
    fgets(buffer, sizeof(buffer), fin);
    if (strlen(buffer) > 0) {
      int code_start = 0;
      while (buffer[code_start] == ' ') {
        code_start++;
      }
      if (buffer[code_start] == '#') {
        // Comment line; copy buffer to output
        printf("%s", buffer);
      } else if (buffer[code_start] == '\r' || buffer[code_start] == '\n') {
        // empty line
        printf("\n");
      } else {
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
        while ((value_end > value_start) && (buffer[value_end] == ' ')) {
          // strip trailing spaces
          value_end--;
        }

        buffer[code_end + 1] = '\0';
        buffer[type_end + 1] = '\0';
        buffer[value_end + 1] = '\0';

        if (!strcmp(&buffer[type_start], "ch")) {
          if (buffer[value_start]) {
            // already has a value
            printf("%s.ch = %s\n", &buffer[code_start], &buffer[value_start]);
          } else {
            // need to guess
            make_guess(&buffer[code_start], width, height);
          }
          // CharItem *item = get_add_item(&buffer[code_start]);
          // item->string = strdup(&buffer[value_start]);
        } else if (!strcmp(&buffer[type_start], "style")) {
          if (strcmp(&buffer[value_start], "unknown")) {
            // already has a type
            printf("%s.style = %s\n", &buffer[code_start], &buffer[value_start]);
          } else {
            // was produced by the guess
          }
          // CharItem *item = get_add_item(&buffer[code_start]);
          // if (!strcmp(&buffer[value_start], "normal")) {
          //   item->style = STYLE_NORMAL;
          // } else if (!strcmp(&buffer[value_start], "italic")) {
          //   item->style = STYLE_ITALIC;
          // } else if (!strcmp(&buffer[value_start], "either")) {
          //   item->style = STYLE_EITHER;
          // }
        } else {
          // copy over
          printf("%s.%s = %s\n", &buffer[code_start], &buffer[type_start], &buffer[value_start]);
        }
        // } else if (!strcmp(&buffer[type_start], "ns")) {
        //   // normal space size
        //   int value = get_numeric_value(&buffer[value_start]);
        //   if ((value > 0) && (value < 100)) {
        //     override_normal_space_width(value);
        //   }
        // } else if (!strcmp(&buffer[type_start], "is")) {
        //   // italic space size
        //   int value = get_numeric_value(&buffer[value_start]);
        //   if ((value > 0) && (value < 100)) {
        //     override_italic_space_width(value);
        //   }
        // } else if (!strcmp(&buffer[type_start], "ths")) {
        //   // threshold value
        //   int value = get_numeric_value(&buffer[value_start]);
        //   if ((value > 0) && (value < 100)) {
        //     util_set_threshold(value);
        //   }
        // } else if (!strcmp(&buffer[type_start], "postprocess")) {
        //   add_postprocess(&buffer[code_start], &buffer[value_start]);
        // }
      }
    }
  }
  fclose(fin);
}

int main(int argc, char *argv[]) {
  int i;
  if (argc < 2) {
    printf("Need to give file argument\n");
    return 0;
  }

  minlist_read("template.data");

  int width = 0;
  int height = 0;
  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "width")) {
      i++;
      if (i < argc) {
         int value = 0;
         int j = 0;
         for (j = 0; argv[i][j]; j++) {
            if ((argv[i][j] >= '0')
                && (argv[i][j] <= '9')) {
               value = 10 * value + (argv[i][j] - '0');
            } else {
               printf("width parameter with non-numeric value '%s'\n",
                      argv[i]);
               return 1;
            }
         }
         width = value;
      } else {
         printf("width parameter without value\n");
         return 1;
      }
    } else if (!strcmp(argv[i], "height")) {
      i++;
      if (i < argc) {
         int value = 0;
         int j = 0;
         for (j = 0; argv[i][j]; j++) {
            if ((argv[i][j] >= '0')
                && (argv[i][j] <= '9')) {
               value = 10 * value + (argv[i][j] - '0');
            } else {
               printf("height parameter with non-numeric value '%s'\n",
                      argv[i]);
               return 1;
            }
         }
         height = value;
      } else {
         printf("height parameter without value\n");
         return 1;
      }
    } else {
      if ((width < 1) || (height < 1)) {
        printf("can't process file without both width and heitht\n");
        return 1;
      }
      process_file(argv[i], width, height);
    }
  }

  return 0;
}
