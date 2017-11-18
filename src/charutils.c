/*
 * Character utilities
 */

#include "common.h"
#include "bitmap.h"
#include "charutils.h"

char *bitmap_to_string(Bitmap bm) {
  return NULL;
}

Bitmap string_to_bitmap(char *string) {
  return NULL;
}

void dump_bitmap(Bitmap bm) {
  if (bm != NULL) {
    int width = bitmap_get_width(bm);
    int height = bitmap_get_height(bm);
    printf("# bitmap dump %d wide x %d high\n", width, height);
    if ((width > 0) && (height > 0)) {
      int y = 0;
      char *data = malloc(width + 1);
      memset(data, 0, width + 1);
      for (y = 0; y < height; y++) {
	int x = 0;
	for (x = 0; x < width; x++) {
	  data[x] = bitmap_get_bit(bm, x, y) ? 'X' : '.';
	}
	printf("# %s\n", data);
      }
      free(data);
    }
  }
}

void dump_string(char *string) {
}
