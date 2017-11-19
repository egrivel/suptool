/*
 * Learn from the previously manually processed characters.
 */

#include "common.h"
#include "charlist.h"
#include "charutils.h"
#include "bitmap.h"

void read_file(char *fname) {
  read_char_data(fname);

  char *default_char = charlist_find_by_string("x", STYLE_NORMAL);
  if (default_char == NULL) {
    printf("Cannot find reference character 'x'\n");
    return;
  }

  Bitmap default_bm = code_to_bitmap(code);
  int default_baseline = code_to_baseline(code);
  int x_width = bitmap_get_width(default_bm);
  int x_height = bitmap_get_height(default_bm);
  bitmap_destroy(default_bm);

  int nr_entries = charlist_nr_entries();
  int i;
  for (i = 0; i < nr_entries; i++) {
    char *code = charlist_get_code(i);
    char *string = get_char_string(code);
    int style = get_char_style(code);
    Bitmap bm = code_to_bitmap(code);
    int baseline = code_to_baseline(code);

    Bitmap minimal = bitmap_to_minimal(bm, baseline, x_width, x_height);
    int minimal_baseline = 0;
    char *minimal_code = bitmap_to_code(minimal, minimal_baseline);
    printf("Got '%s'\n", minimal_code);
    bitmap_destroy(bm);
    bitmap_destroy(minimal);
    free(minimal_code);
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
