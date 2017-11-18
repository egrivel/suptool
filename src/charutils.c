/*
 * Character utilities
 */

#include "common.h"
#include "bitmap.h"
#include "charutils.h"

/* private functions */

/* public functions */

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

char *bitmap_to_code(Bitmap bm) {
  return NULL;
}

Bitmap code_to_bitmap(char *string) {
  return NULL;
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
