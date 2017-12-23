#include "common.h"
#include "charlist.h"
#include "charutils.h"
#include "bitmap.h"
#include "minlist.h"

#define MAX_WIDTH 128
#define MAX_HEIGHT 128

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

        buffer[code_end + 1] = '\0';
        buffer[type_end + 1] = '\0';
        buffer[value_end + 1] = '\0';

        if (!strcmp(&buffer[type_start], "ch")) {
          if (buffer[value_start]) {
            // already has a value
            printf("%s.ch = %s\n", &buffer[code_start], &buffer[value_start]);
          } else {
            // need to guess
          }
          // CharItem *item = get_add_item(&buffer[code_start]);
          // item->string = strdup(&buffer[value_start]);
        } else if (!strcmp(&buffer[type_start], "style")) {
          if (!strcmp(&buffer[value_start], "unknown")) {
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


  int width_counts[MAX_WIDTH];
  memset(width_counts, 0, MAX_WIDTH * sizeof(int));
  int height_counts[MAX_HEIGHT];
  memset(height_counts, 0, MAX_HEIGHT * sizeof(int));

  read_char_data(fname);

  int nr_entries = charlist_nr_entries();

  int i;
  for (i = 0; i < nr_entries; i++) {
    char *code = charlist_get_code(i);
    int width = code_to_width(code);
    int height = code_to_height(code);
    printf("Code %s\n", code);
    printf("Got width=%d, height=%d\n", width, height);
    if ((width > 0) && (width < MAX_WIDTH)) {
      width_counts[width]++;
    }
    if ((height > 0) && (height < MAX_HEIGHT)) {
      height_counts[height]++;
    }
  }

  int common_width = 0;
  int common_height = 0;

  for (i = 0; i < MAX_WIDTH; i++) {
    if (width_counts[i] > width_counts[common_width]) {
      common_width = i;
    }
  }

  for (i = 0; i < MAX_HEIGHT; i++) {
    if (height_counts[i] > height_counts[common_height]) {
      common_height = i;
    }
  }

  printf("File %s: got common width=%d, height=%d\n",
    fname, common_width, common_height);
  // // In order to determine the most common width and height in a
  // // subtitle file, go with the lowercase letter "x" for a pretty
  // char *code = charlist_find_by_string("x", STYLE_NORMAL);
  // if (code == NULL) {
  //   printf("Cannot find reference character 'x'\n");
  //   return;
  // }

  // Bitmap default_bm = code_to_bitmap(code);
  // int x_width = bitmap_get_width(default_bm);
  // int x_height = bitmap_get_height(default_bm);
  // bitmap_destroy(default_bm);
  // printf("Got x_width=%d, x_height=%d\n", x_width, x_height);

  // int nr_entries = charlist_nr_entries();
  // int i;
  // for (i = 0; i < nr_entries; i++) {
  //   char *code = charlist_get_code(i);
  //   // printf("Got code %s\n", code);
  //   char *string = get_char_string(code);
  //   char *style = charlist_get_style_name(code);
  //   int style_value = charlist_get_style(code);
  //   Bitmap bm = code_to_bitmap(code);
  //   int baseline = code_to_baseline(code);
  //   bitmap_set_baseline(bm, baseline);

  //   if (strlen(string) < 10) {
  //     printf("Starting with this bitmap:\n");
  //     dump_bitmap(bm);

  //     Bitmap minimal_bm = bitmap_to_minimal(bm, baseline, x_width, x_height);

  //     // printf("\nresulting in this minimal bitmap:\n");
  //     dump_bitmap(minimal_bm);
  //     char *minimal_code = bitmap_to_code(minimal_bm,
  //         bitmap_get_baseline(minimal_bm));;
  //     printf("%s.ch = %s\n", minimal_code, string);
  //     printf("%s.style = %s\n", minimal_code, style);
  //     printf("\n\n");

  //     minlist_add(minimal_code, string, style_value);

  //     // int minimal_baseline = 0;
  //     // char *minimal_code = bitmap_to_code(minimal, minimal_baseline);
  //     // printf("Got '%s'\n", minimal_code);
  //     // free(minimal_code);

  //     bitmap_destroy(minimal_bm);
  //   }

  //   bitmap_destroy(bm);
  // }

  // char *buffer = malloc(strlen(fname) + strlen(".out") + 1);
  // strcpy(buffer, fname);
  // strcat(buffer, ".out");;
  // write_char_data(buffer);
  // charlist_reset();
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
