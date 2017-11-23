/*
 * bitmap
 *
 * Purpose: maintain a bitmap structure
 */

#define Bitmap struct _bitmap *

#include "common.h"
#include "bitmap.h"

struct _bitmap {
  int width;
  int height;
  int baseline;
  char *data;
};

#define BIT_SET    '*'
#define BIT_CLEAR  ' '

Bitmap bitmap_create() {
  Bitmap bm = malloc(sizeof(struct _bitmap));
  if (bm != NULL) {
    bm->width = 0;
    bm->height = 0;
    bm->baseline = 0;
    bm->data = NULL;
  }
  return bm;
}

void bitmap_destroy(Bitmap bm) {
  if (bm != NULL) {
    if (bm->data != NULL) {
      free(bm->data);
    }
    free(bm);
  }
}

int bitmap_calc_offset(int width, int height, int x, int y) {
  if ((x >= 0) && (x < width) && (y >= 0) && (y < height)) {
    return (width * y) + x;
  }
  printf("bitmap_calc_offset(%d, %d, %d, %d): position outside of dimensions\n",
	 width, height, x, y);
  return 0;
}

void bitmap_resize(Bitmap bm, int new_width, int new_height) {
  int old_width = bm->width;
  int old_height = bm->height;
  
  if ((bm == NULL) || (new_width < 0) || (new_height < 0)) {
    // Invalid input: ignored
    return;
  }
  
  // do not decrease in size
  if (new_width < old_width) {
    new_width = old_width;
  }
  if (new_height < old_height) {
    new_height = old_height;
  }
  
  if ((new_width == 0) || (new_height == 0)) {
    // With a width or height of zero, there is no data, so just
    // record the new sizes (if any)
    bm->width = new_width;
    bm->height = new_height;
    return;
  }
  
  if ((new_width == old_width) && (new_height == old_height)) {
    // no change in size
    return;
  }
  
  char *new_data = malloc(new_width * new_height);
  memset(new_data, BIT_CLEAR, new_width * new_height);
  if (bm->data != NULL) {
    // need to copy existing data
    int i, j;
    for (i = 0; i < old_height; i++) {
      for (j = 0; j < old_width; j++) {
	int old_offset = bitmap_calc_offset(old_width, old_height, j, i);
	int new_offset = bitmap_calc_offset(new_width, new_height, j, i);
	new_data[new_offset] = bm->data[old_offset];
      }
    }
    free(bm->data);
  }
  bm->data = new_data;
  bm->width = new_width;
  bm->height = new_height;
}

void bitmap_set_width(Bitmap bm, int width) {
  if (bm != NULL) {
    bitmap_resize(bm, width, bm->height);
  }
}

void bitmap_set_height(Bitmap bm, int height) {
  if (bm != NULL) {
    bitmap_resize(bm, bm->width, height);
  }
}

void bitmap_set_baseline(Bitmap bm, int baseline) {
  if (bm != NULL) {
    bm->baseline = baseline;
  }
}

int bitmap_get_width(Bitmap bm) {
  if (bm != NULL) {
    return bm->width;
  }
  return 0;
}

int bitmap_get_height(Bitmap bm) {
  if (bm != NULL) {
    return bm->height;
  }
  return 0;
}

int bitmap_get_baseline(Bitmap bm) {
  if (bm != NULL) {
    return bm->baseline;
  }
  return 0;
}

void bitmap_set_bit(Bitmap bm, int x, int y, Bit bit) {
  if ((bm != NULL)
      && (x >= 0) && (y >= 0)) {
    // make sure the bitmap has the right dimensions; this will
    // create the data if needed
    bitmap_resize(bm, x+1, y+1);
    if (bm->data != NULL) {
      // get the offset
      int offset = bitmap_calc_offset(bm->width, bm->height, x, y);
      if (bit) {
	bm->data[offset] = BIT_SET;
      } else {
	bm->data[offset] = BIT_CLEAR;
      }
    }
  }
}

Bit bitmap_get_bit(Bitmap bm, int x, int y) {
  if ((bm != NULL) && (bm->data != NULL)
      && (x >= 0) && (y >= 0)
      && (x < bm->width) && (y < bm->height)) {
    // get the offset
    int offset = bitmap_calc_offset(bm->width, bm->height, x, y);
    return (bm->data[offset] == BIT_SET);
  }
  return false;
}
