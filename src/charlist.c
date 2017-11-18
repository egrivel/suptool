/*
 * charlist
 *
 * Purpose: maintain a list of characters that are known to the system.
 */

#include "common.h"
#include "charlist.h"
#include "subprop.h"
#include "bitmap.h"
#include "util.h"

typedef struct {
   char *code;
   char *string;
   int style;
} CharItem;

CharItem *charlist = NULL;
int nr_chars = 0;
int capacity = 0;

#define NR_POST 20
char *post_str[NR_POST];
char *post_replace[NR_POST];
int nr_post = 0;

CharItem *get_char_item(char *code) {
   int i;
   for (i = 0; i < nr_chars; i++) {
      if (!strcmp(charlist[i].code, code)) {
         return &charlist[i];
      }
   }
   return NULL;
}


bool has_char(char *code) {
   CharItem *item = get_char_item(code);
   return (item != NULL);
}

char *get_char_string(char *code) {
   CharItem *item = get_char_item(code);
   if ((item != NULL) && (item->string != NULL)) {
      return item->string;
   }
   return "";
}

void set_char_string(char *code, char *string) {
   CharItem *item = get_char_item(code);
   if (item != NULL) {
      item->string = strdup(string);
   }
}
   

int charlist_get_style(char *code) {
   CharItem *item = get_char_item(code);
   if (item != NULL) {
      return item->style;
   }
   return false;
}

char *charlist_get_style_name(char *code) {
   CharItem *item = get_char_item(code);
   if (item != NULL) {
      switch (item->style) {
      case STYLE_NORMAL:  return "normal";
      case STYLE_ITALIC:  return "italic";
      case STYLE_EITHER:  return "either";
      }
   }
   return "unknown";
}
void charlist_set_style(char *code, int style) {
   CharItem *item = get_char_item(code);
   if (item != NULL) {
      item->style = style;
   }
}

void add_char(char *code) {
   if (nr_chars >= capacity) {
      if (capacity == 0) {
         charlist = malloc(32 * sizeof(CharItem));
         capacity = 32;
      } else {
         CharItem *oldlist = charlist;
         charlist = malloc(2 * capacity * sizeof(CharItem));
         memcpy(charlist, oldlist, capacity * sizeof(CharItem));
         free(oldlist);
         capacity = 2 * capacity;
      }
   }
   if (nr_chars < capacity) {
      charlist[nr_chars].code = strdup(code);
      charlist[nr_chars].string = NULL;
      charlist[nr_chars].style = STYLE_UNKNOWN;
      nr_chars++;
   }
}

int get_numeric_value(char *str) {
   int value = 0;
   while (*str) {
      if ((*str >= '0') && (*str <= '9')) {
         value = 10 * value + (*str - '0');
      }
      str++;
   }
   return value;
}

CharItem *get_add_item(char *str) {
   CharItem *item = get_char_item(str);
   if (item == NULL) {
      add_char(str);
      item = get_char_item(str);
   }
   return item;
}

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
char *do_postprocess(char *str) {
   int i;
   for (i = 0; i < nr_post; i++) {
      char * offset = strstr(str, post_str[i]);
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
   return str;
}

void read_char_data(char *fname) {
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
               CharItem *item = get_add_item(&buffer[code_start]);
               item->string = strdup(&buffer[value_start]);
            } else if (!strcmp(&buffer[type_start], "style")) {
               CharItem *item = get_add_item(&buffer[code_start]);
               if (!strcmp(&buffer[value_start], "normal")) {
                  item->style = STYLE_NORMAL;
               } else if (!strcmp(&buffer[value_start], "italic")) {
                  item->style = STYLE_ITALIC;
               } else if (!strcmp(&buffer[value_start], "either")) {
                  item->style = STYLE_EITHER;
               }
            } else if (!strcmp(&buffer[type_start], "ns")) {
               // normal space size
               int value = get_numeric_value(&buffer[value_start]);
               if ((value > 0) && (value < 100)) {
                  override_normal_space_width(value);
               }
            } else if (!strcmp(&buffer[type_start], "is")) {
               // italic space size
               int value = get_numeric_value(&buffer[value_start]);
               if ((value > 0) && (value < 100)) {
                  override_italic_space_width(value);
               }
            } else if (!strcmp(&buffer[type_start], "ths")) {
               // threshold value
               int value = get_numeric_value(&buffer[value_start]);
               if ((value > 0) && (value < 100)) {
                  util_set_threshold(value);
               }
            } else if (!strcmp(&buffer[type_start], "postprocess")) {
               add_postprocess(&buffer[code_start], &buffer[value_start]);
            }
            /*
            printf("Found entry:\n");
            printf("   code = '%s'\n", &buffer[code_start]);
            printf("   type = '%s'\n", &buffer[type_start]);
            printf("   value = '%s'\n", &buffer[value_start]);
            */
         }
      }
   }
   fclose(fin);

   // find duplicates
   /*
   int i, j;
   for (i = 0; i < nr_chars; i++) {
      for (j = i + 1; j < nr_chars; j++) {
         if ((charlist[i].string != NULL)
             && (charlist[j].string != NULL)
             && strlen(charlist[i].string)
             && !strcmp(charlist[i].string, charlist[j].string)
             && (charlist[i].style == charlist[j].style)) {
            printf ("Duplicate items for %s:\n   %s\n   %s\n",
                    charlist[i].string, charlist[i].code, charlist[j].code);
         }
      }
   }
   */
}

char *get_encoded(unsigned char *data, int nr_bytes) {
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

char *encode_bitmap(Bitmap bm,
                    int line_start, int line_end,
                    int char_start, int char_end,
                    int baseline) {
   int i, j;
   int nr_bits = (line_end - line_start + 1) * (char_end - char_start + 1);
   int nr_bytes = (nr_bits + 7) / 8;
   unsigned char *char_data = malloc(nr_bytes + 3);
   memset(char_data, 0, nr_bytes + 3);
   char_data[0] = line_end - line_start + 1;
   char_data[1] = char_end - char_start + 1;
   char_data[2] = baseline - line_start;
   int offset = 3;
   unsigned char mask = 0x80;
   for (i = line_start; i <= line_end; i++) {
      for (j = char_start; j <= char_end; j++) {
         if (bitmap_get_bit(bm, j, i)) {
            char_data[offset] |= mask;
         }
         mask = mask / 2;
         if (mask == 0) {
            offset++;
            mask = 0x80;
         }
      }
   }
   char *encoded = get_encoded(char_data, nr_bytes + 3);
   free(char_data);

   /*
   CharItem *item = get_char_item(encoded);
   if (item != NULL) {
      char *base_code = encode_bitmap_base(data, line_start, line_end, char_start, char_end, baseline);
      CharItem *base_item = get_char_item(base_code);
      if (base_item == NULL) {
         add_char(base_code);
         base_item = get_char_item(base_code);
         base_item->string = strdup(item->string);
         base_item->style = item->style;
      }
      if (strcmp(item->string, base_item->string)) {
         printf("Base code %s can be '%s' and '%s'\n",
                base_code, item->string, base_item->string);
      }
   }
   */
   return encoded;
}

#define INIT_SIZE 16
struct bit_data {
   unsigned char *data;
   char *encoded;
   int data_size;
   int offset;
   unsigned char mask;
};

void add_bit(struct bit_data *data, int bit) {
   if (data->mask == 0x00) {
      if (data->offset >= data->data_size) {
         if (data->data_size == 0) {
            data->data_size = INIT_SIZE;
            data->data = malloc(data->data_size + 1);
            memset(data->data, 0, data->data_size + 1);
            data->offset = 0;
         } else {
            data->data_size = 2 * data->data_size;
            unsigned char *new_data = malloc(data->data_size + 1);
            memset(new_data, 0, data->data_size + 1);
            memcpy(new_data, data->data, data->data_size / 2);
            free(data->data);
            data->data = new_data;
            data->offset++;
         }
      } else {
         data->offset++;
      }
      data->mask = 0x80;
   }
   if (bit) {
      data->data[data->offset] |= data->mask;
   }
   data->mask /= 2;
}

char *get_encoded_from_struct(struct bit_data *data) {
   if (data->offset) {
      data->encoded = get_encoded(data->data, data->offset + 1);
   } else {
      data->encoded = strdup("");
   }
   return data->encoded;
}

int decode_out_value;
int decode_out_count;

void decode_bits(int nr) {
   int decode_in_mask = 0x20;
   while (decode_in_mask) {
      decode_out_value = 2 * decode_out_value;
      if (nr & decode_in_mask) {
         decode_out_value++;
      }
      decode_out_count++;
      if (decode_out_count == 8) {
         printf("0x%02x ", decode_out_value);
         decode_out_value = 0;
         decode_out_count = 0;
      }
      decode_in_mask /= 2;
   }
}   

unsigned char *decode_string_to_bytes(char *str, int *length) {
  int inlength = strlen(str);
  int outbuflength = (inlength / 5) * 8 + 2;
  unsigned char *outbuf = malloc(outbuflength);
  memset(outbuf, 0, outbuflength);

  *length = 0;
  int outbits = 0;
  
  while (*str) {
    int value ;
    if ((*str >= '0') && (*str <= '9')) {
      value = *str - '0';
    } else if ((*str >= 'A') && (*str <= 'Z')) {
      value = *str - 'A' + 10;
    } else if ((*str >= 'a') && (*str <= 'z')) {
      value = *str - 'a' + 36;
    } else if (*str == '-') {
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
	  printf("Out of memory in decode_string_to_bytes: %d exceeds %d\n", *length, outbuflength);
	  exit(0);
	}
      }
      value_mask /= 2;
    }
    str++;
  }

  if (outbits != 0) {
    (*length)++;
  }

  return outbuf;
}

void decode_base(char *base) {
   decode_out_count = 0;
   decode_out_value = 0;

   while (*base) {
      if ((*base >= '0') && (*base <= '9')) {
         decode_bits(*base - '0');
      } else if ((*base >= 'A') && (*base <= 'Z')) {
         decode_bits(*base - 'A' + 10);
      } else if ((*base >= 'a') && (*base <= 'z')) {
         decode_bits(*base - 'a' + 36);
      } else if (*base == '-') {
         decode_bits(62);
      } else {
         decode_bits(63);
      }
      base++;
   }
   
   if (decode_out_count) {
      while (decode_out_count < 8) {
         decode_out_value *= 2;
         decode_out_count++;
      }
      printf("0x%02x ", decode_out_value);
   }
   printf("\n");
}

/**
 * Dump the contents of a buffer (containing the string of bits representing
 * the character) on standard output.
 */
void dump_character_data(unsigned char *buffer, int length) {
  if (length < 3) {
    // We need to get at least 3 bytes of data back with height, width and
    // baseline. If there are fewer than 3 bytes of data in the buffer,
    // we can't continue.
    printf("Decoded data is too short (%d, should be at least 4 bytes)\n",
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
}

char *encode_bitmap_base(Bitmap bm,
                         int line_start, int line_end,
                         int char_start, int char_end,
                         int baseline) {
   bool debug = false;
   struct bit_data bitdata;
   memset(&bitdata, 0, sizeof(bitdata));
   /*
    * Encoding rules:
    *  - height_type: 2 bits
    *     0x00: base char only
    *     0x01: base char + ascender
    *     0x02: base char + descender
    *     0x03: base char + ascender + descender
    *  - width_type: 2 bits
    *     0x00: less than 1/3 of base width (narrow, e.g. "i", "l")
    *     0x01: between 1/3 and 2/3 of base width (medium, e.g. "f", "t")
    *     0x02: between 2/3 and 1 1/2 of base width (normal width)
    *     0x03: more than 1 1/2 of base width (wide)
    * - if ascender, 6 bits of ascender data:
    *        x  x  x
    *        x  x  x
    * - 14 bits of base data:
    *        x  x  x
    *        x  x  x
    *        x  x  x
    *        x     x
    *        x  x  x
    * - if descender, 6 bits of descender data:
    *        x  x  x
    *        x  x  x
    * This gives a total width of 18, 24 or 30 bits of data, resulting
    * in 3, 4 or 5 bytes
    */
   bool has_asc = false;
   bool has_desc = false;
   int height_type = 0x00;
   if (baseline > (line_start + get_base_height())) {
      // has ascender
      height_type |= 0x01;
      has_asc = true;
   }
   if (line_end > baseline) {
      // has descender
      height_type |= 0x02;
      has_desc = true;
   }
   add_bit(&bitdata, height_type & 0x02);
   add_bit(&bitdata, height_type & 0x01);

   int width_type = 0x00;
   int char_width = char_end - char_start + 1;
   int left = 0;
   int middle = 0;
   int right = 0;
   if (char_width < (get_base_width() / 3)) {
      width_type = 0x00;
      left = char_start;
      if (char_width > 2) {
         left++;
      }
      middle = -1;
      right = -1;
   } else if (char_width < ((2 * get_base_width()) / 3)) {
      width_type = 0x01;
      left = char_start + 1;
      middle = get_base_width() / 2;
      if (middle > char_width) {
         middle = char_width - 1;
      }
      right = -1;
   } else if (char_width < ((3 * get_base_width()) / 2)) {
      width_type = 0x02;
      left = char_start + 1;
      middle = char_width / 2;
      right = char_end - 1;
   } else {
      width_type = 0x03;
      left = char_start + 1;
      middle = char_width / 2;
      right = char_end - 1;
   }
   add_bit(&bitdata, width_type & 0x02);
   add_bit(&bitdata, width_type & 0x01);

   int i, j;

   if (debug) {
      printf("\n# Get base encoding....\n");
   }
   if (has_asc) {
      int first = line_start;
      int last = baseline - get_base_height() - 1;
      if (debug) {
         printf("Ascender: initial first=%d, last=%d\n", first, last);
      }
      if ((last - first) > 5) {
         first++;
         last--;
      }
      if (debug) {
         printf("Ascender: adjusted first=%d, last=%d\n", first, last);
      }
      if ((left != -1)
          && (first >= line_start)
          && (first <= line_end)
          && (first <= baseline - get_base_height())
          && (bitmap_get_bit(bm, left, first))){
         add_bit(&bitdata, 0x01);
      } else {
         add_bit(&bitdata, 0x00);
      }
      if ((middle != -1)
          && (first >= line_start)
          && (first <= line_end)
          && (first <= baseline - get_base_height())
          && (bitmap_get_bit(bm, middle, first))){
         add_bit(&bitdata, 0x01);
      } else {
         add_bit(&bitdata, 0x00);
      }
      if ((last != -1)
          && (first >= line_start)
          && (first <= line_end)
          && (first <= baseline - get_base_height())
          && (bitmap_get_bit(bm, right, first))){
         add_bit(&bitdata, 0x01);
      } else {
         add_bit(&bitdata, 0x00);
      }
      if ((left != -1)
          && (last >= line_start)
          && (last <= line_end)
          && (last <= baseline - get_base_height())
          && (bitmap_get_bit(bm, left, last))){
         add_bit(&bitdata, 0x01);
      } else {
         add_bit(&bitdata, 0x00);
      }
      if ((middle != -1)
          && (last >= line_start)
          && (last <= line_end)
          && (last <= baseline - get_base_height())
          && (bitmap_get_bit(bm, middle, last))){
         add_bit(&bitdata, 0x01);
      } else {
         add_bit(&bitdata, 0x00);
      }
      if ((right != -1)
          && (last >= line_start)
          && (last <= line_end)
          && (last <= baseline - get_base_height())
          && (bitmap_get_bit(bm, right, last))){
         add_bit(&bitdata, 0x01);
      } else {
         add_bit(&bitdata, 0x00);
      }
      if (debug) {
         for (i = line_start; i <= (baseline - get_base_height()); i++) {
            if ((i >= line_start) && (i <= line_end)) {
               printf("# %02d ", i);
               for (j = char_start; j <= char_end; j++) {
                  if (((i == first) || (i == last))
                      && ((j == left) || (j == middle) || (j == right))) {
                     printf("a");
                  } else {
                     printf("%c", bitmap_get_bit(bm, j, i) ? '*' : ' ');
                  }
               }
               printf("\n");
            }
         }
      }
   }
   int base1 = baseline - (4 * (get_base_height() + 1)) / 4;
   int base2 = baseline - (3 * (get_base_height() + 1)) / 4;
   int base3 = baseline - (2 * (get_base_height() + 1)) / 4;
   int base4 = baseline - (1 * (get_base_height() + 1)) / 4;
   int base5 = baseline;
   if ((base1 < line_start) || (base1 > line_end)) {
      base1 = -1;
   }
   if ((base2 < line_start) || (base2 > line_end)) {
      base2 = -1;
   }
   if ((base3 < line_start) || (base3 > line_end)) {
      base3 = -1;
   }
   if ((base4 < line_start) || (base4 > line_end)) {
      base4 = -1;
   }
   if ((base5 < line_start) || (base5 > line_end)) {
      base5 = -1;
   }

   if ((left != -1)
       && (base1 != -1)
       && (bitmap_get_bit(bm, left, base1))){
      add_bit(&bitdata, 0x01);
   } else {
      add_bit(&bitdata, 0x00);
   }
   if ((middle != -1)
       && (base1 != -1)
       && (bitmap_get_bit(bm, middle, base1))){
      add_bit(&bitdata, 0x01);
   } else {
      add_bit(&bitdata, 0x00);
   }
   if ((right != -1)
       && (base1 != -1)
       && (bitmap_get_bit(bm, right, base1))){
      add_bit(&bitdata, 0x01);
   } else {
      add_bit(&bitdata, 0x00);
   }

   if ((left != -1)
       && (base2 != -1)
       && (bitmap_get_bit(bm, left, base2))){
      add_bit(&bitdata, 0x01);
   } else {
      add_bit(&bitdata, 0x00);
   }
   if ((middle != -1)
       && (base2 != -1)
       && (bitmap_get_bit(bm, middle, base2))){
      add_bit(&bitdata, 0x01);
   } else {
      add_bit(&bitdata, 0x00);
   }
   if ((right != -1)
       && (base2 != -1)
       && (bitmap_get_bit(bm, right, base2))){
      add_bit(&bitdata, 0x01);
   } else {
      add_bit(&bitdata, 0x00);
   }

   if ((left != -1)
       && (base3 != -1)
       && (bitmap_get_bit(bm, left, base3))){
      add_bit(&bitdata, 0x01);
   } else {
      add_bit(&bitdata, 0x00);
   }
   if ((middle != -1)
       && (base3 != -1)
       && (bitmap_get_bit(bm, middle, base3))){
      add_bit(&bitdata, 0x01);
   } else {
      add_bit(&bitdata, 0x00);
   }
   if ((right != -1)
       && (base3 != -1)
       && (bitmap_get_bit(bm, right, base3))){
      add_bit(&bitdata, 0x01);
   } else {
      add_bit(&bitdata, 0x00);
   }

   if ((left != -1)
       && (base4 != -1)
       && (bitmap_get_bit(bm, left, base4))){
      add_bit(&bitdata, 0x01);
   } else {
      add_bit(&bitdata, 0x00);
   }
   if ((middle != -1)
       && (base4 != -1)
       && (bitmap_get_bit(bm, middle, base4))){
      add_bit(&bitdata, 0x01);
   } else {
      add_bit(&bitdata, 0x00);
   }
   if ((right != -1)
       && (base4 != -1)
       && (bitmap_get_bit(bm, right, base4))){
      add_bit(&bitdata, 0x01);
   } else {
      add_bit(&bitdata, 0x00);
   }

   if ((left != -1)
       && (base5 != -1)
       && (bitmap_get_bit(bm, left, base5))){
      add_bit(&bitdata, 0x01);
   } else {
      add_bit(&bitdata, 0x00);
   }
   if ((middle != -1)
       && (base5 != -1)
       && (bitmap_get_bit(bm, middle, base5))){
      add_bit(&bitdata, 0x01);
   } else {
      add_bit(&bitdata, 0x00);
   }
   if ((right != -1)
       && (base5 != -1)
       && (bitmap_get_bit(bm, right, base5))){
      add_bit(&bitdata, 0x01);
   } else {
      add_bit(&bitdata, 0x00);
   }

   if (debug) {
      for (i = (baseline - get_base_height() + 1); i <= baseline; i++) {
         if ((i >= line_start) && (i <= line_end)) {
            printf("# %02d ", i);
            for (j = char_start; j <= char_end; j++) {
               if (((i == base1) || (i == base2) || (i == base3)
                    || (i == base4) || (i == base5))
                   && ((j == left) || (j == middle) || (j == right))) {
                  printf("b");
               } else {
                  printf("%c", bitmap_get_bit(bm, j, i) ? '*' : ' ');
               }
            }
            printf("\n");
         }
      }
   }
   if (has_desc) {
      int first = baseline + 1;
      int last = line_end;
      if (debug) {
         printf("Descender: initial first=%d, last=%d\n", first, last);
      }
      if ((last - first) > 5) {
         first++;
         last--;
      }
      if (debug) {
         printf("Descender: adjusted first=%d, last=%d\n", first, last);
      }
      if ((left != -1)
          && (first >= line_start)
          && (first <= line_end)
          && (first <= baseline - get_base_height())
          && (bitmap_get_bit(bm, left, first))){
         add_bit(&bitdata, 0x01);
      } else {
         add_bit(&bitdata, 0x00);
      }
      if ((middle != -1)
          && (first >= line_start)
          && (first <= line_end)
          && (first <= baseline - get_base_height())
          && (bitmap_get_bit(bm, middle, first))){
         add_bit(&bitdata, 0x01);
      } else {
         add_bit(&bitdata, 0x00);
      }
      if ((last != -1)
          && (first >= line_start)
          && (first <= line_end)
          && (first <= baseline - get_base_height())
          && (bitmap_get_bit(bm, right, first))){
         add_bit(&bitdata, 0x01);
      } else {
         add_bit(&bitdata, 0x00);
      }
      if ((left != -1)
          && (last >= line_start)
          && (last <= line_end)
          && (last <= baseline - get_base_height())
          && (bitmap_get_bit(bm, left, last))){
         add_bit(&bitdata, 0x01);
      } else {
         add_bit(&bitdata, 0x00);
      }
      if ((middle != -1)
          && (last >= line_start)
          && (last <= line_end)
          && (last <= baseline - get_base_height())
          && (bitmap_get_bit(bm, middle, last))){
         add_bit(&bitdata, 0x01);
      } else {
         add_bit(&bitdata, 0x00);
      }
      if ((right != -1)
          && (last >= line_start)
          && (last <= line_end)
          && (last <= baseline - get_base_height())
          && (bitmap_get_bit(bm, right, last))){
         add_bit(&bitdata, 0x01);
      } else {
         add_bit(&bitdata, 0x00);
      }
      if (debug) {
         for (i = baseline + 1; i <= line_end; i++) {
            if ((i >= line_start) && (i <= line_end)) {
               printf("# %02d ", i);
               for (j = char_start; j <= char_end; j++) {
                  if (((i == first) || (i == last))
                      && ((j == left) || (j == middle) || (j == right))) {
                     printf("d");
                  } else {
                     printf("%c", bitmap_get_bit(bm, j, i) ? '*' : ' ');
                  }
               }
               printf("\n");
            }
         }
      }
   }
   char *encoded = get_encoded_from_struct(&bitdata);
   if (debug) {
      printf("# base encoding: %s\n", encoded);
   }
   free(bitdata.data);
   return encoded;
}

