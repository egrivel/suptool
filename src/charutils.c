/*
 * Character utilities
 */

#include "common.h"
#include "bitmap.h"
#include "charutils.h"

char *bytes_to_code(unsigned char *data, int nr_bytes) {
   char *string = malloc(2 * nr_bytes + 1);

   int i = 0;
   int offset = 0;
   int mask = 0x80;
   int single_value = 0;
   int nr_bits = 0;
   while (offset < nr_bytes) {
      single_value = 2 * single_value;
      if (data[offset] & mask) {
         single_value++;
      }
      nr_bits++;
      if (nr_bits > 5) {
         if (single_value < 10) {
            single_value += '0';
         } else if (single_value < 36) {
            single_value += 'A' - 10;
         } else if (single_value < 62) {
            single_value += 'a' - 36;
         } else if (single_value == 62) {
            single_value = '-';
         } else {
            single_value = '_';
         }
         string[i++] = single_value;
         single_value = 0;
         nr_bits = 0;
      }
      mask = mask / 2;
      if (mask == 0) {
         offset++;
         mask = 0x80;
      }
   }
   if (nr_bits > 0) {
     while (nr_bits <= 5) {
       single_value *= 2;
       nr_bits++;
     }
     if (single_value < 10) {
       single_value += '0';
     } else if (single_value < 36) {
       single_value += 'A' - 10;
     } else if (single_value < 62) {
       single_value += 'a' - 36;
     } else if (single_value == 62) {
       single_value = '-';
     } else {
       single_value = '_';
     }
     string[i++] = single_value;
   }
   string[i++] = '\0';
   return string;
}

unsigned char *code_to_bytes(char *code, int *length) {
  int inlength = strlen(code);
  int outbuflength = (inlength / 5) * 8 + 2;
  unsigned char *outbuf = malloc(outbuflength);
  memset(outbuf, 0, outbuflength);

  *length = 0;
  int outbits = 0;
  
  while (*code) {
    int value ;
    if ((*code >= '0') && (*code <= '9')) {
      value = *code - '0';
    } else if ((*code >= 'A') && (*code <= 'Z')) {
      value = *code - 'A' + 10;
    } else if ((*code >= 'a') && (*code <= 'z')) {
      value = *code - 'a' + 36;
    } else if (*code == '-') {
      value = 62;
    } else {
      value = 63;
    }

    int value_mask = 0x20;
    while (value_mask) {
      outbuf[*length] = 2 * outbuf[*length];
      if (value & value_mask) {
	outbuf[*length]++;
      }
      outbits++;
      if (outbits == 8) {
	(*length)++;
	outbits = 0;
	if (*length >= outbuflength) {
	  printf("Out of memory in code_to_bytes: %d exceeds %d\n", *length, outbuflength);
	  exit(0);
	}
      }
      value_mask /= 2;
    }
    code++;
  }

  if (outbits != 0) {
    (*length)++;
  }

  return outbuf;
}

char *bitmap_to_code(Bitmap bm, int baseline) {
   int i, j;
   int height = bitmap_get_height(bm);
   int width = bitmap_get_width(bm);
   int nr_bits = height * width;
   int nr_bytes = (nr_bits + 7) / 8;

   unsigned char *byte_data = malloc(nr_bytes + 3);
   memset(byte_data, 0, nr_bytes + 3);

   byte_data[0] = height;
   byte_data[1] = width;
   byte_data[2] = baseline;

   int offset = 3;
   unsigned char mask = 0x80;
   for (i = 0; i < height; i++) {
      for (j = 0; j < width; j++) {
         if (bitmap_get_bit(bm, j, i)) {
            byte_data[offset] |= mask;
         }
         mask = mask / 2;
         if (mask == 0) {
            offset++;
            mask = 0x80;
         }
      }
   }

   char *code = bytes_to_code(byte_data, nr_bytes + 3);
   free(byte_data);

   return code;
}

Bitmap code_to_bitmap(char *code) {
  int length;
  unsigned char *bytes = code_to_bytes(code, &length);

  int width = bytes[1];
  int height = bytes[0];

  Bitmap bm = bitmap_create();
  bitmap_set_width(bm, width);
  bitmap_set_height(bm, height);

  int nr_bits = width * height;
  int mask = 0x80;
  int byte_ptr = 3;

  int x = 0;
  int y = 0;
  while (nr_bits) {
    bool bit = bytes[byte_ptr] & mask;
    bitmap_set_bit(bm, x, y, bit);
 
    x++;
    if (x >= width) {
      y++;
      x = 0;
    }

    mask = mask / 2;
    if (mask == 0) {
      byte_ptr++;
      mask = 0x80;
    }

    nr_bits--;
  }
  
  return bm;
}

int code_to_baseline(char *code) {
  int length;
  unsigned char *bytes = code_to_bytes(code, &length);

  int baseline = bytes[2];
  free(bytes);
  return baseline;
}

int code_to_width(char *code) {
  int length;
  unsigned char *bytes = code_to_bytes(code, &length);

  int width = bytes[1];
  free(bytes);
  return width;
}

int code_to_height(char *code) {
  int length;
  unsigned char *bytes = code_to_bytes(code, &length);

  int height = bytes[0];
  free(bytes);
  return height;
}

// The following rows make up the "minimal" image map:
//  1. Image row (x_height * 2.00) is fifth ascender 
//  2. Image row (x_height * 1.75) is fourth ascender 
//  3. Image row (x_height * 1.50) is third ascender 
//  4. Image row (x_height * 1.25) is second ascender
//  5. Image row (x_height + 1) is first ascender
//
//  6. Image row (x_height - 1) is top body row
//  7. Image row (x_height * 0.75) is next body row
//  8. Image row (x_height * 0.50) is next body row
//  9. Image row (x_height * 0.25) is next body row
// 10. Image row 0 is bottom body row
//
// 11. Image row (-2) is first descender
// 12. Image row (-x_height * 0.25) is second descender
// 13. Image row (-x_height * 0.50) is third descender
// 14. Image row (-x_height * 0.75) is fourth descender
// 15. Image row (-x_height * 1.00) is fifth descender
// This function returns, for minimal image row (nr), what the
// image row is to fill the minimal image from.
int get_image_row_from_minimal_row(int nr, int x_height) {
  switch (nr) {
  case 1: return x_height * 2;
  case 2: return x_height * 1.75;
  case 3: return x_height * 1.5;
  case 4: return x_height * 1.25;
  case 5: return x_height + 1;
  case 6: return x_height - 1;
  case 7: return x_height * 0.75;
  case 8: return x_height * 0.5;
  case 9: return x_height * 0.25;
  case 10: return 0;
  case 11: return -2;
  case 12: return -(x_height * 0.25);
  case 13: return -(x_height * 0.5);
  case 14: return -(x_height * 0.75);
  case 15: return -(x_height);
  }
  return 0;
}

// Assuming the image has row 0 as the baseline row and row number
// above that are positive, row numbers below that are negative.
//
// if (height-1) equal to baseline, 
//  - bitmap row 0 is image row heigth-1
//  - bitmap row 1 is image row height-2
//  - bitmap row n is image row (height-n-1)
//  - bitmap row (height-1) is image row 0
//
// if (height-1) is (baseline-3), then the image floats above the baseline
//  - bitmap row 0 is image row height+3-1
//  - bitmap row 1 is image row height+3-2
//  - bitmap row n is image row height+3-n-1
//  - bitmap row (height-1) is image row height+3-height+1-1 = 3
//
// If (height-1) is (baseline+4), then the image has descenders
//  - bitmap row 0 is image row height-4-1
//  - bitmap row 1 is image row height-4-2
//  - bitmap row n is image row height-4-n-1
//  - bitmap row (height-1) is image row height-4-height+1-1 = -4
//
// In general,
//  - bitmap row n is image row
//       height - (height-1-baseline) - n - 1
//     = height - height + 1 + baseline - n - 1
//     = 1 + baseline - n - 1
//     = baseline - n
// or conversely,
//   image row m = bitmap row (baseline - m)
//
// so rows 0 through (height-1) in the bitmap translate to rows
// (baseline) through (baseline - height + 1) in the image
int get_bitmap_row_from_image_row(int nr, int baseline) {
  return (baseline - nr);
}

// Create a new bitmap out of an existing one, reduced to "minimal"
// size
Bitmap bitmap_to_minimal(Bitmap bm, int baseline, int x_width, int x_height) {
  Bitmap minimal_bm = bitmap_create();

  int width = bitmap_get_width(bm);
  int height = bitmap_get_height(bm);
  
  // The minimal bitmap would sample the letter "x" as a 5x5 pattern.
  // So use as horizontal sampling:
  //  - column 0 * (x_width - 1)
  //  - column 0.25 * (x_width - 1)
  //  - column 0.50 * (x_width - 1)
  //  - column 0.75 * (x_width - 1)
  //  - column 1.00 * (x_width - 1);

  int row = 0;
  float x_fraction;
  int i;
  for (i = 1; i <= 15; i++) {
    int image_row = get_image_row_from_minimal_row(i, x_height);
    int bm_row = get_bitmap_row_from_image_row(image_row, baseline);
    if ((bm_row < height) && (bm_row >= 0)) {
      // bitmap row falls within the bitmap, so include it in the
      // minimal bitmap as (row)
      int col = 0;
      for (x_fraction = 0.0; (x_fraction * (x_width - 1)) < width; x_fraction += 0.25) {
	int bm_col = x_fraction * (x_width - 1);
	if ((bm_col >= 0) && (bm_col < width)) {
	  bitmap_set_bit(minimal_bm, col, row,
			 bitmap_get_bit(bm, bm_col, bm_row));;
	}
	col++;
      }
      row++;
    }
  }

  return minimal_bm;
}

void dump_bitmap(Bitmap bm) {
  if (bm != NULL) {
    int width = bitmap_get_width(bm);
    int height = bitmap_get_height(bm);
    printf("# Bitmap dump %d wide x %d high\n", width, height);
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

void dump_code(char *string) {
  int length;
  unsigned char *buffer = code_to_bytes(string, &length); 

  if (length < 3) {
    // We need to get at least 3 bytes of data back with height, width and
    // baseline. If there are fewer than 3 bytes of data in the buffer,
    // we can't continue.
    printf("dump_string(): Decoded data is too short (%d, should be at least 4 bytes)\n",
	   length);
    exit(0);
  }
  
  int height = buffer[0];
  int width = buffer[1];
  int baseline = buffer[2];
  
  printf("# Start of character dump baseline %d\n", baseline);
  printf("# (0, 0) to (%d, %d): %d wide, %d high\n",
	 width-1, height-1, width, height);
  
  // Based on width and height, determine the total number of bits we have
  // to process, and make sure they are all there in the data
  int nr_bits = width * height;
  if (length < (nr_bits + 7) / 8 + 3) {
    // Not enough bytes in the buffer to accomodate all the bits that make
    // up the character. Give an error and some debug info.
    printf("Got %d chars, expecting %d. Decoded string:\n",
	   length, (nr_bits + 7) / 8 + 3);
    int i;
    for (i = 0; i < length; i++) {
      printf("0x%02X ", buffer[i]);
    }
    printf("\n");
    exit(0);
  }
  
  // Know that the buffer has enough data, start peeling bits off
  int row = 0;
  int col = 0;
  int ptr = 3;
  unsigned char mask = 0x80;
  while (nr_bits) {
    nr_bits--;
    int bit = buffer[ptr] & mask;
    if (col == 0) {
      printf("# %02d: ", row);
    }
    if (bit) {
      printf("X");
    } else {
      printf(".");
    }
    
    mask = mask / 2;
    if (mask == 0) {
      ptr++;
      mask = 0x80;
    }
    col++;
    if (col == width) {
      row++;
      col = 0;
      printf("\n");
      if (row == (baseline + 1)) {
	printf("------");
	int j;
	for (j = 0; j < width; j++) {
	  printf("-");
	}
	printf("\n");
      }
    }
  }
  
  if (row <= baseline) {
    while (row <= baseline) {
      printf("# %02d:\n", row++);
    }
    printf("------");
    int j;
    for (j = 0; j < width; j++) {
      printf("-");
    }
    printf("\n");
  }

  free(buffer);
}
