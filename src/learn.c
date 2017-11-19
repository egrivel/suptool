/*
 * Learn from the previously manually processed characters.
 */

#include "common.h"
#include "charlist.h"
#include "charutils.h"
#include "bitmap.h"

void read_file(char *fname) {
  read_char_data(fname);
  
  // In order to determine the most common width and height in a
  // subtitle file, go with the lowercase letter "x" for a pretty
  // good approximation.
  char *code = charlist_find_by_string("x", STYLE_NORMAL);
  if (code == NULL) {
    printf("Cannot find reference character 'x'\n");
    return;
  }

  Bitmap default_bm = code_to_bitmap(code);
  // int default_baseline = code_to_baseline(code);
  int x_width = bitmap_get_width(default_bm);
  int x_height = bitmap_get_height(default_bm);
  bitmap_destroy(default_bm);
  printf("Got x_width=%d, x_height=%d\n", x_width, x_height);

  int nr_entries = charlist_nr_entries();
  int i;
  for (i = 0; i < nr_entries; i++) {
    char *code = charlist_get_code(i);
    printf("Got code %s\n", code);
    char *string = get_char_string(code);
    // int style = charlist_get_style(code);
    Bitmap bm = code_to_bitmap(code);
    int baseline = code_to_baseline(code);

    if (strlen(string) == 1) {
      Bitmap minimal_bm = bitmap_to_minimal(bm, baseline, x_width, x_height);
      
      printf("Starting with this bitmap:\n");
      dump_bitmap(bm);
      printf("\nresulting in this minimal bitmap:\n");
      dump_bitmap(minimal_bm);
      printf("\n\n");

    // int minimal_baseline = 0;
    // char *minimal_code = bitmap_to_code(minimal, minimal_baseline);
    // printf("Got '%s'\n", minimal_code);
    // free(minimal_code);

      bitmap_destroy(minimal_bm);
    }

    bitmap_destroy(bm);
  }
}

int main(int argc, char *argv[]) {
  int i;
  if (argc < 2) {
    printf("Need to give file argument\n");
    return 0;
  }

  for (i = 1; i < argc; i++) {
    read_file(argv[i]);
  }

  return 0;
}
