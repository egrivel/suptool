#include "common.h"
#include "subtitle.h"
#include "supformat.h"
#include "subformat.h"
#include "charlist.h"
#include "charutils.h"
#include "output.h"
#include "subprop.h"
#include "util.h"

bool gl_debug = false;
int gl_debug_subtitle = -1;
int gl_debug_block = -1;
int gl_cur_subtitle = 0;
int gl_format = FORMAT_SRT;
int gl_normal_space_width = 0;
int gl_italic_space_width = 0;

typedef struct {
   int max_height;         // highest line
   int line_height;        // average hight of a line
   int base_height;        // average base hight
   int ascender_height;    // average ascender hight
   int descender_height;   // average descender height
   int line_total;
   int line_count;

   int *base_data;
   int *base_count;
   int base_nr_items;

   int *ascender_data;
   int *ascender_count;
   int ascender_nr_items;

   int *descender_data;
   int *descender_count;
   int descender_nr_items;

} LineSizes;

void line_sizes_add_single(int **data, int **count, int *nr_items, int value) {
   int i;
   for (i = 0; i < (*nr_items); i++) {
      if ((*data)[i] == value) {
         (*count)[i]++;
         return;
      }
      if ((*data)[i] == -1) {
         (*data)[i] = value;
         (*count)[i] = 1;
         return;
      }
   }
   int new_items = (*nr_items) * 2;
   if ((*nr_items) == 0) {
      new_items = 16;
   }
   int *new_data = malloc(new_items * sizeof(int));
   int *new_count = malloc(new_items * sizeof(int));
   for (i = 0; i < new_items; i++) {
      if (i < (*nr_items)) {
         new_data[i] = (*data)[i];
         new_count[i] = (*count)[i];
      } else {
         new_data[i] = -1;
         new_count[i] = 0;
      }
   }
   new_data[*nr_items] = value;
   new_count[*nr_items] = 1;
   if ((*nr_items) > 0) {
      free(*data);
      free(*count);
   }
   *data = new_data;
   *count = new_count;
   *nr_items = new_items;
}

void line_sizes_init(LineSizes *sizes) {
   memset(sizes, 0, sizeof(LineSizes));
}

void line_sizes_clean(LineSizes *sizes) {
   if (sizes->base_nr_items > 0) {
      free(sizes->base_data);
      free(sizes->base_count);
      sizes->base_nr_items = 0;
   }
   if (sizes->ascender_nr_items > 0) {
      free(sizes->ascender_data);
      free(sizes->ascender_count);
      sizes->ascender_nr_items = 0;
   }
   if (sizes->descender_nr_items > 0) {
      free(sizes->descender_data);
      free(sizes->descender_count);
      sizes->descender_nr_items = 0;
   }
}

void line_sizes_add(LineSizes *sizes, int base, int ascender, int descender) {
   line_sizes_add_single(&(sizes->base_data),
                         &(sizes->base_count),
                         &(sizes->base_nr_items),
                         base);
   line_sizes_add_single(&(sizes->ascender_data),
                         &(sizes->ascender_count),
                         &(sizes->ascender_nr_items),
                         ascender);
   line_sizes_add_single(&(sizes->descender_data),
                         &(sizes->descender_count),
                         &(sizes->descender_nr_items),
                         descender);
}

int line_sizes_get_max(int *data, int *count, int nr_items) {
   int max_data = 0;
   int max_count = 0;
   int i;
   for (i = 0; i < nr_items; i++) {
      if (count[i] > max_count) {
         max_data = data[i];
         max_count = count[i];
      }
   }
   return max_data;
}

void line_sizes_compute(LineSizes *sizes) {
   if (sizes->line_count) {
      sizes->line_height = ((sizes->line_total + (sizes->line_count / 2)) / sizes->line_count);
   }
   sizes->base_height = line_sizes_get_max(sizes->base_data,
                                           sizes->base_count,
                                           sizes->base_nr_items);
   sizes->ascender_height = line_sizes_get_max(sizes->ascender_data,
                                               sizes->ascender_count,
                                               sizes->ascender_nr_items);
   sizes->descender_height = line_sizes_get_max(sizes->descender_data,
                                                sizes->descender_count,
                                                sizes->descender_nr_items);
   if (gl_debug) {
      printf("Determine file properties to be:\n");
      printf("   line_height=%d, base_height=%d, ascender_height=%d, descender_height=%d\n",
             sizes->line_height, sizes->base_height, sizes->ascender_height,
             sizes->descender_height);
   }
}

/* Color Conversion */
/* conversion (any values < 0 become zero and value > 255 become 255):
   R = Y + 1.402 * (Cr - 128)
   G = Y - 0.34414 * (Cb - 128) - 0.71414 * (Cr - 128)
   B = Y + 1.772 * (Cb - 128)
*/
/*
void PaletteToRgb(PaletteData *data, Rgb *rgb) {
   double y = data->val_Y;
   double Cr = data->val_Cr;
   double Cb = data->val_Cb;

   double r = y + 1.402 * (Cr - 128);
   double g = y - 0.34414 * (Cb - 128) - 0.71414 * (Cr - 128);
   double b = y + 1.772 * (Cb - 128);

   if (r < 0) {
      rgb->red = 0;
   } else if (r > 255) {
      rgb->red = 255;
   } else {
      rgb->red = (unsigned)(r + 0.5);
   }

   if (g < 0) {
      rgb->green = 0;
   } else if (g > 255) {
      rgb->green = 255;
   } else {
      rgb->green = (unsigned)(g + 0.5);
   }

   if (b < 0) {
      rgb->blue = 0;
   } else if (b > 255) {
      rgb->blue = 255;
   } else {
      rgb->blue = (unsigned)(b + 0.5);
   }
}
*/

void subtitle_line_properties(Subtitle sbt, int nr, LineSizes *sizes) {
   if ((sbt == NULL)
       || (subtitle_bitmap(sbt) == NULL)
       || (sizes == NULL)) {
      return;
   }

   int *scan_lines;
   scan_lines = malloc(subtitle_get_height(sbt) * sizeof(int));
   int i, j;
   Bitmap bm = subtitle_bitmap(sbt);
   for (i = 0; i < subtitle_get_height(sbt); i++) {
      scan_lines[i] = 0;
      for (j = 0; j < subtitle_get_width(sbt); j++) {
         if (bitmap_get_bit(bm, j, i)) {
            scan_lines[i]++;
         }
      }
   }

   int cur_height = 0;
   int max_width = 0;
   for (i = 0; i < subtitle_get_height(sbt); i++) {
      if (scan_lines[i] == 0) {
         if (cur_height > 0) {
            sizes->line_total += cur_height;
            sizes->line_count++;
            if (cur_height > sizes->max_height) {
               sizes->max_height = cur_height;
            }

            // try to determine ascender height, base height and
            // descender height. Go through the recognized line height
            // and look for a pattern:
            //   one or more lines with width < 35% of max width (ascender)
            //   one or more lines with width >= 35% of max width (base)
            //   one or more lines with width < 35% of max width (descender)
            // If this pattern is recognized, add the ascender, base and
            // descender heights to the sizes
            int ascender = 0;
            int base = 0;
            int descender = 0;
            int j;
            bool match_pattern = true;
            for (j = i - cur_height; j < i; j++) {
               if (scan_lines[j] < ((max_width * 35) / 100)) {
                  if (base == 0) {
                     // still counting the ascender part
                     ascender++;
                  } else {
                     // past the base, now counting descender
                     descender++;
                  }
               } else if (scan_lines[j] >= ((max_width * 35) / 100)) {
                  if (descender == 0) {
                     // OK, this is part of the base
                     base++;
                  } else {
                     // can't have a width matching the base when
                     // already in descender
                     match_pattern = false;
                  }
               } else {
                  // this section doesn't match the pattern
                  // (only in case conditions above change to allow for
                  // a "something else" situation)
                  match_pattern = false;
               }
            }
            if ((ascender == 0)
                || (base == 0) 
                || (descender == 0)) {
               // this section doesn't match the pattern
               match_pattern = false;
            }
            if (match_pattern) {
               if (gl_debug) {
                  printf("Line %d: base=%d, ascender=%d, descender=%d\n",
                         nr, base, ascender, descender);
               }
               line_sizes_add(sizes, base, ascender, descender);
            }

            cur_height = 0;
         }
         max_width = 0;
      } else {
         cur_height++;
         if (scan_lines[i] > max_width) {
            max_width = scan_lines[i];
         }
      }
   }
   if (cur_height > 0) {
      sizes->line_total += cur_height;
      sizes->line_count++;
      if (cur_height > sizes->max_height) {
         sizes->max_height = cur_height;
      }

      // try to determine ascender height, base height and
      // descender height. Go through the recognized line height
      // and look for a pattern:
      //   one or more lines with width < 35% of max width (ascender)
      //   one or more lines with width >= 35% of max width (base)
      //   one or more lines with width < 35% of max width (descender)
      // If this pattern is recognized, add the ascender, base and
      // descender heights to the sizes
      int ascender = 0;
      int base = 0;
      int descender = 0;
      int j;
      bool match_pattern = true;
      for (j = i - cur_height; j < i; j++) {
         if (scan_lines[j] < ((max_width * 35) / 100)) {
            if (base == 0) {
               // still counting the ascender part
               ascender++;
            } else {
               // past the base, now counting descender
               descender++;
            }
         } else if (scan_lines[j] >= ((max_width * 35) / 100)) {
            if (descender == 0) {
               // OK, this is part of the base
               base++;
            } else {
               // can't have a width matching the base when
               // already in descender
               match_pattern = false;
            }
         } else {
            // this section doesn't match the pattern
            // (only in case conditions above change to allow for
            // a "something else" situation)
            match_pattern = false;
         }
      }
      if ((ascender == 0)
          || (base == 0) 
          || (descender == 0)) {
         // this section doesn't match the pattern
         match_pattern = false;
      }
      if (match_pattern) {
         line_sizes_add(sizes, base, ascender, descender);
      }
      cur_height = 0;
   }
}

void determine_line_properties(LineSizes *sizes) {
   int nr_subtitles = get_nr_subtitles();
   int i;
   for (i = 0; i < nr_subtitles; i++) {
      subtitle_line_properties(get_subtitle(i), i, sizes);
   }
   line_sizes_compute(sizes);
}

bool find_line_up(Bitmap bm, int block_width, int block_height,
                  int *line, int x, int y, bool debug) {
   if (debug) {
      printf("find line up from (%d, %d)\n", x, y);
   }
   bool result = false;
   if ((x < 0) || (x >= block_width)) {
      // outside of boundary
      return false;
   }
   if (y < 0) {
      return true;
   }

   if (x == (block_width - 1)) {
      // If we're moving up, a pixel at the right edge is still the end part
      // of a mark above the current letter, e.g. with an italic 'i', so 
      // those are still acceptable parts of the line up
      line[y] = x + 1;
      result = find_line_up(bm, block_width, block_height,
                            line, x, y - 1, debug);
      return result;
   } else  if (!bitmap_get_bit(bm, x, y)) {
      // so far, so good
      line[y] = x;
   }

   if (y == 0) {
      // Reached the top, so done
      return true;
   }
      
   while (!result
          && (x >= 0)
          && (x < block_width)
          && (!bitmap_get_bit(bm, x, y))) {
      line[y] = x;
      if (!result && (!bitmap_get_bit(bm, x, y - 1))) {
         if (debug) {
            printf("Try (%d, %d) which is '%c'\n", x, y-1, 
                   bitmap_get_bit(bm, x, y-1) ? '*' : ' ');
         }
         result = find_line_up(bm, block_width, block_height,
                               line, x, y - 1, debug);
      }
      if (!result && (x < (block_width - 1))
          && (!bitmap_get_bit(bm, x+1, y-1))) {
         if (debug) {
            printf("Try (%d, %d) which is '%c'\n", x+1, y-1,
                   bitmap_get_bit(bm, x+1, y-1) ? '*' : ' ');
         }
         result = find_line_up(bm, block_width, block_height,
                               line, x + 1, y - 1, debug);
      }
      if (!result && (x > 0)
          && (!bitmap_get_bit(bm, x-1, y-1))) {
         if (debug) {
            printf("Try (%d, %d) which is '%c'\n", x-1, y-1,
                   bitmap_get_bit(bm, x-1, y-1) ? '*' : ' ');
         }
         result = find_line_up(bm, block_width, block_height,
                               line, x - 1, y - 1, debug);
      }
      if (!result) {
         x++;
         if (debug) {
            printf("move over to x=%d\n", x);
         }
      }
   }
   return result;
}

bool find_line_down(Bitmap bm, int block_width, int block_height,
                    int *line, int x, int y, bool debug) {
   if (debug) {
      printf("find line down from (%d, %d)\n", x, y);
   }
   bool result = false;
   if ((x < 0) || (x >= block_width)) {
      // outside of boundary
      return false;
   }
   if (y >= block_height) {
      return true;
   }
   if (!bitmap_get_bit(bm, x, y)) {
      // so far, so good
      line[y] = x;
   }
   if (y == (block_height - 1)) {
      // Reached the bottom, so done
      return true;
   }
      
   while (!result
          && (x >= 0)
          && (x < block_width)
          && (!bitmap_get_bit(bm, x, y))) {
      line[y] = x;
      if (!result && !bitmap_get_bit(bm, x, y+1)) {
         if (debug) {
            printf("Try (%d, %d) which is '%c'\n", x, y+1,
                   bitmap_get_bit(bm, x, y+1) ? '*' : ' ');
         }
         result = find_line_down(bm, block_width, block_height,
                                 line, x, y + 1, debug);
      }
      if (!result && (x > 0)
          && (!bitmap_get_bit(bm, x-1, y+1))) {
         if (debug) {
            printf("Try (%d, %d) which is '%c'\n", x-1, y+1,
                   bitmap_get_bit(bm, x-1,y+1) ? '*' : ' ');
         }
         result = find_line_down(bm, block_width, block_height,
                                 line, x - 1, y + 1, debug);
      }
      if (!result && (x < (block_width - 1))
          && (!bitmap_get_bit(bm, x+1, y+1))) {
         if (debug) {
            printf("Try (%d, %d) which is '%c'\n", x+1, y+1,
                   bitmap_get_bit(bm, x+1, y+1) ? '*' : ' ');
         }
         result = find_line_down(bm, block_width, block_height,
                                 line, x + 1, y + 1, debug);
      }
      if (!result) {
         x--;
         if (debug) {
            printf("move over to x=%d\n", x);
         }
      }
   }
   return result;
}

char **memory = NULL;
int memory_capacity = 0;
int memory_count = 0;
void remember_guess(char *text, bool style) {
   if (memory_capacity == 0) {
      memory_capacity = 32;
      memory = malloc(memory_capacity * sizeof(char *));
   } else if (memory_count == memory_capacity) {
      int new_capacity = 2 * memory_capacity;
      char **new_memory = malloc(new_capacity * sizeof(char *));
      memcpy(new_memory, memory, memory_capacity * sizeof(char *));
      free(memory);
      memory = new_memory;
      memory_capacity = new_capacity;
   }

   char *buffer = malloc(strlen(text) + 3);
   if (style) {
      strcpy(buffer, "I-");
   } else {
      strcpy(buffer, "N-");
   }
   strcat(buffer, text);
   int i;
   for (i = 0; i < memory_count; i++) {
      if (!strcmp(buffer, memory[i])) {
         // Already have this one
         // printf("# --- this is a duplicate ---\n");
         free(buffer);
         return;
      }
   }
   memory[memory_count] = buffer;
   memory_count++;
}

void process_single_char(Bitmap bm, int block_height, int block_width,
                         int baseline, int alt_base1, int alt_base2,
                         int nr_try, char *fname, bool debug) {
   int bases[15];
   bases[0] = baseline;
   bases[1] = baseline + 1;
   bases[2] = baseline - 1;
   bases[3] = alt_base1;
   bases[4] = alt_base1 + 1;
   bases[5] = alt_base1 - 1;
   bases[6] = alt_base2;
   bases[7] = alt_base2 + 1;
   bases[8] = alt_base2 - 1;
   int nr_bases = 9;
   int i;

   if (get_base_height() > 20) {
      bases[9] = baseline + 2;
      bases[10] = baseline - 2;
      bases[11] = alt_base1 + 2;
      bases[12] = alt_base1 - 2;
      bases[13] = alt_base2 + 2;
      bases[14] = alt_base2 - 2;
      nr_bases = 15;
   }      
   char *base_encode = NULL;
   for (i = 0; i < nr_bases; i++) {
      base_encode = encode_bitmap_base(bm, 0, block_height - 1,
                                       0, block_width - 1,
                                       bases[i]);
      if (has_char(base_encode)) {
         // found a match
         break;
      } else {
         free(base_encode);
         base_encode = NULL;
      }
   }
   if (base_encode == NULL) {
      // None found, revert to regular baseline
      base_encode = encode_bitmap_base(bm, 0, block_height - 1,
                                       0, block_width - 1,
                                       baseline);
   }

   char *full_encode = NULL;
   for (i = 0; i < nr_bases; i++) {
      full_encode = bitmap_to_code(bm, bases[i]);
      if (has_char(full_encode)) {
         // found a match
         break;
      } else {
         free(full_encode);
         full_encode = NULL;
      }
   }
   if (full_encode == NULL) {
      // None found, revert to regular baseline
      full_encode = bitmap_to_code(bm, baseline);
   }

   char *base_result = NULL;
   int base_style = STYLE_UNKNOWN;
   // char *base_style_name = NULL;

   char *full_result = NULL;
   int full_style = STYLE_UNKNOWN;
   // char *full_style_name = NULL;

   
   if (has_char(base_encode)) {
      // use this as the base value
      base_result = get_char_string(base_encode);
      base_style = charlist_get_style(base_encode);
      // base_style_name = charlist_get_style_name(base_encode);
   }
   if (has_char(full_encode)) {
      full_result = get_char_string(full_encode);
      full_style = charlist_get_style(full_encode);
      // full_style_name = charlist_get_style_name(full_encode);
   } 

   if (base_result == NULL) {
      //printf("# Start of character dump baseline %d (first in %d/%d):\n", baseline, nr_try, gl_cur_subtitle);
      if (full_result == NULL) {
         //   printf("%s.ch = \n", base_encode);
         //   printf("%s.style = unknown\n", base_encode);
      } else {
         //    printf("# --- the following is a guess ---\n");
         remember_guess(full_result, full_style);
         // printf("%s.ch = %s\n", base_encode, full_result);
         // printf("%s.style = %s\n", base_encode, full_style_name);
      }
      add_char(base_encode);
   } else {
      if ((full_result != NULL) && strlen(full_result)) {
         // bool match = false;
         int len = strlen(full_result);
         int i = 0;
         while (i < strlen(base_result)) {
            int base_len = 0;
            while ((base_result[i + base_len] != '\0')
                   && (base_result[i + base_len] != ' ')) {
               base_len++;
            }
            // possibly found an item in the base result
            if (base_len == len) {
               if (!memcmp(base_result + i, full_result, len)) {
                  // match = true;
               }
            }
            i += base_len + 1;
         }
         /*
         if (!match || ((base_style & full_style) == 0x00)) {
            printf("# --- in %d/%d:\n", nr_try, gl_cur_subtitle);
            printf("# --- base encoding %s can be:\n", base_encode);
            printf("#      - '%s' %s (base)\n", base_result, base_style_name);
            printf("#      - '%s' %s (full)\n", full_result, full_style_name);
         }
         */
      }
   }
         

   if (debug || (full_result == NULL)) {
      printf("# Start of character dump baseline %d (first in %s %d/%d):\n", baseline, fname, nr_try, gl_cur_subtitle);
      dump_bitmap(bm);
      printf("%s.ch = \n", full_encode);
      printf("%s.style = unknown\n", full_encode);
      add_char(full_encode);
   }

   if ((base_result != NULL) && (full_result != NULL)) {
      output_string(full_result, full_style);
   } else if (base_result != NULL) {
      output_string(base_result, base_style);
   } else if (full_result != NULL) {
      output_string(full_result, full_style);
   } else {
   }
   free(base_encode);
   free(full_encode);
}

void process_char(Bitmap bm, int line_start, int line_end, int baseline, int alt_base1, int alt_base2, int char_start, int char_end, int subtitle_nr, char *fname) {
   int i, j;
   static int nr_try = 0;
   nr_try++;

   bool debug = false;
   if (nr_try == gl_debug_block) {
      debug = true;
   }

   if (debug) {
      printf("process_char(bm, %d, %d, %d, %d, %d, %d, %d)\n",
             line_start, line_end, baseline, alt_base1, alt_base2, char_start, char_end);
   }
   // Skip any leading empty lines
   bool is_data = false;
   for (line_start = line_start; line_start <= line_end; line_start++) {
      for (j = char_start; j <= char_end; j++) {
         if (bitmap_get_bit(bm, j, line_start)) {
            // line_start is not empty, OK
            is_data = true;
            break;
         }
      }
      if (is_data) {
         break;
      }
   }

   is_data = false;
   for (line_end = line_end; line_end >= line_start; line_end--) {
      for (j = char_start; j <= char_end; j++) {
         if (bitmap_get_bit(bm, j, line_end)) {
            // line_end is not empty, OK
            is_data = true;
            break;
         }
      }
      if (is_data) {
         break;
      }
   }

   if (debug) {
      printf("block becomes (%d, %d) to (%d, %d)\n",
             char_start, line_start, char_end,line_end);
   }
   // At this point, we have the rectangle with contains data with no
   // completely empty column in it. Next, we need to see if this
   // rectangle maybe contains multiple italic characters, so we need
   // to find the potential slanted empty column.

   int block_width = char_end - char_start + 1;
   int block_height = line_end - line_start + 1;
   Bitmap new_bm = bitmap_create();
   for (i = 0; i < block_height; i++) {
      for (j = 0; j < block_width; j++) {
         bitmap_set_bit(new_bm, j, i,
                          bitmap_get_bit(bm, j + char_start, i + line_start));
      }
   }

   // Adjust the baseline relative to the offset of the block
   baseline -= line_start;
   alt_base1 -= line_start;
   alt_base2 -= line_start;

   if (debug) {
      printf("Inspect character block %d with baseline %d:\n", nr_try, baseline);
      dump_bitmap(new_bm);
   }

   // Move through the character in the following direction:
   //    1   2   3
   //    8       4 
   //    7   6   5
   int finger_x = 0;
   int finger_y = block_height - 1;
   int direction = 2;
   int last_x = 0;
   int last_y = 0;
   // keep trying until we either hit the top of the box (line_start)
   // or the right edge (char_end)
   while ((finger_y >= 0) && (finger_x < (block_width - 1))) {
      if (debug) {
         printf("Moving around - finger is (%d, %d), direction is %d\n",
                finger_x, finger_y, direction);
      }
      if (!bitmap_get_bit(new_bm, finger_x, finger_y)) {
         // we should only have a blank in the first_column
         if (finger_x != 0) {
            printf("Assert: finger_x %d == 0\n", finger_x);
         }
         finger_y--;
      } else {
         // we are on the outline of a character. The finger will
         // trace the outline in a counter-clockwise direction.
         // Depending on how we came to this pixel, we start trying
         // to the pixel next to the one we came from (in counter-clockwise
         // fashion), and continue until we find another pixel or hit
         // the left edge. When we hit the left edge, we move up.
         // if old direction is 6, new one is 1
         // if old direction is 7, new one is 2
         // if old direction is 8, new one is 3
         // if old direction is 1, new one is 4
         // if old direction is 2, new one is 5
         // if old direction is 3, new one is 6
         // if old direction is 4, new one is 7
         // if old direction is 5, new one is 8
         direction += 3;
         if (direction > 8) {
            direction -= 8;
         }
         int next_x = finger_x;
         int next_y = finger_y;
         int count = 0;
         while (count < 10) {
            count++;
            switch (direction) {
            case 1:
               next_x = finger_x - 1;
               next_y = finger_y - 1;
               break;
            case 2:
               next_x = finger_x;
               next_y = finger_y - 1;
               break;
            case 3:
               next_x = finger_x + 1;
               next_y = finger_y - 1;
               break;
            case 4:
               next_x = finger_x + 1;
               next_y = finger_y;
               break;
            case 5:
               next_x = finger_x + 1;
               next_y = finger_y + 1;
               break;
            case 6:
               next_x = finger_x;
               next_y = finger_y + 1;
               break;
            case 7:
               next_x = finger_x - 1;
               next_y = finger_y + 1;
               break;
            case 8:
               next_x = finger_x - 1;
               next_y = finger_y;
               break;
            }
            if ((next_x < 0) || (next_x > (block_width - 1))
                || (next_y < 0) || (next_y > (block_height - 1))) {
               if (next_x < 0) {
                  // must be moving in direction 1 to get here
                  if (direction != 1) {
                     printf("Assert: direction %d = 1\n", direction);
                  }
                  // There is no choice, we have to move up now
                  direction = 2;
                  next_x = finger_x;
                  next_y = finger_y - 1;
                  // Actually, no need to keep trying, just go there
                  finger_x = next_x;
                  finger_y = next_y;
                  break;
               } else if (next_x > (block_width - 1)) {
                  // moved beyond the edge, there is only a single
                  // character in the block
                  last_x = char_end;
                  break;
               }
               if (next_y < 0) {
                  // we're done
                  finger_y = next_y;
                  finger_x = next_x;
                  break;
               } else if (next_y > (block_height - 1)) {
                  // reaching the bottom of the block. 
                  // direction has to be one of 5, 6 or 7 to have the y_value
                  // increased. So there are two possibilities:
                  //  - we just came from another pixel, and start looking
                  //    around for a new pixel. The previous direction must
                  //    have been 2, 3 or 4 to end up with 5, 6 or 7, but
                  //    since we can't have been moving up to get here, the
                  //    previous direction must have been 4 and the new one
                  //    must be 7.
                  //  - we are searching for the next pixel, so the previous
                  //    direction must have been 6, 7 or 8 to end up with
                  //    5, 6 or 7. Again, 6 and 7 are impossible for previous
                  //    directions, so it must have been 8 and the new
                  //    one is 7.
                  // Either way,we have to skip directions 6, 7 and 5 and
                  // jump straight to 4 to move past the bottom edge
                  // change the direction directly to 4
                  direction = 4;
                  // Apply the change for direction 4
                  next_x = finger_x + 1;
                  next_y = finger_y;
               }
            }
            if (debug) {
               printf("   Try %d: (%d, %d) direction %d\n",
                      count, next_x, next_y, direction);
            }
            if (bitmap_get_bit(new_bm, next_x, next_y)) {
               // found next pixel
               finger_x = next_x;
               finger_y = next_y;
               if (last_x < finger_x) {
                  last_x = finger_x;
                  last_y = finger_y;
               }
               break;
            } else {
               direction--;
               if (direction < 1) {
                  direction += 8;
               }
            }
         }
      }
   }

   if (last_x >= (block_width - 1)) {
      if (debug) {
         printf("Block %d contains only a single character with last_x=%d\n",
                nr_try, last_x);
      }
      if (gl_debug_subtitle == subtitle_nr) {
         printf("Process character #%d\n", nr_try);
      }
      process_single_char(new_bm, block_height, block_width, baseline, alt_base1, alt_base2, nr_try, fname, debug);
      bitmap_destroy(new_bm);
      return;
   }

   if (debug) {
      printf("\n");
      printf("Block %d has split character ending at (%d, %d):\n",
             nr_try, last_x, last_y);
      dump_bitmap(new_bm);
   }

   int *vertical_line = malloc(block_height * sizeof(int));
   memset(vertical_line, 0, block_height * sizeof(int));
   bool found_line = false;
   if (find_line_up(new_bm, block_width, block_height,
                    vertical_line, last_x + 1, last_y, debug)) {
      if (find_line_down(new_bm, block_width, block_height,
                          vertical_line, last_x + 1, last_y, debug)) {
         found_line = true;
      }
   }
   if (found_line) {
      if (debug) {
         printf("Vertical line: ");
         for (i = 0; i < block_height; i++) {
            printf("%d, ", vertical_line[i]);
         }
         printf("\n");
      }

      int new_width = 0;
      for (i = 0; i < block_height; i++) {
         while ((vertical_line[i] > 0)
                && (!bitmap_get_bit(new_bm, vertical_line[i] - 1, i))) {
            vertical_line[i]--;
         }
         // vertical_line[i] is now the first space in the row
         if (vertical_line[i] > new_width) {
            new_width = vertical_line[i];
         }
      }

      if (new_width == block_width) {
         // Single character after all
         if (debug) {
            printf("New width %d, block width %d\n", new_width, block_width);
            printf("Block %d contains only a single character with last_x=%d\n",
                   nr_try, last_x);
         }
         if (gl_debug_subtitle == subtitle_nr) {
            printf("Process character #%d\n", nr_try);
         }
         process_single_char(new_bm, block_height, block_width, baseline, alt_base1, alt_base2, nr_try, fname, debug);
         bitmap_destroy(new_bm);
         return;
      } else {
         Bitmap temp_bm = bitmap_create();
         for (i = 0; i < block_height; i++) {
            for (j = 0; j < new_width; j++) {
               if (j < vertical_line[i]) {
                  bitmap_set_bit(temp_bm, j, i,
                                   bitmap_get_bit(new_bm, j, i));
                  bitmap_set_bit(new_bm, j, i, false);
               }
            }
         }
         
         if (debug) {
            printf("Got new block:\n");
	    dump_bitmap(temp_bm);
            printf("Old block has become:\n");
	    dump_bitmap(new_bm);
         }
         
         // Process the first new character
         if (gl_debug_subtitle == subtitle_nr) {
            printf("Process first character #%d\n", nr_try);
         }
         process_char(temp_bm, 0, (block_height - 1), baseline, alt_base1, alt_base2, 0, new_width - 1, subtitle_nr, fname);
         bitmap_destroy(temp_bm);
         
         is_data = false;
         // figure out the char_start value of the remainder
         for (char_start = 0; char_start < block_width; char_start++) {
            for (i = 0; i < block_height; i++) {
               if (bitmap_get_bit(new_bm, char_start, i)) {
                  is_data = true;
                  break;
               }
            }
            if (is_data) {
               break;
            }
         }
         
         // recursively process the remainder
         if (gl_debug_subtitle == subtitle_nr) {
            printf("Process remainder #%d\n", nr_try);
         }
         process_char(new_bm, 0, block_height - 1, baseline, alt_base1, alt_base2, char_start, block_width - 1, subtitle_nr, fname);
      }
   } else {
      printf("could not find vertical line up\n");
   }

   bitmap_destroy(new_bm);
}

void parse_line(Subtitle sbt, int line_start, int line_end, int baseline,
                int alt_base1, int alt_base2, int nr, char *fname) {
   int width = subtitle_get_width(sbt);
   Bitmap bm = subtitle_bitmap(sbt);
   int i, j;
   int *cols = malloc(width * sizeof(int));
   if (cols == NULL) {
      return;
   }

   if (width == 0) {
      printf("Got subtitle %d with width %d\n", nr, width);
      return;
   }
   if (bm == NULL) {
      printf("Got subtitle %d with NULL bitmap\n", nr);
      return;
   }

   for (j = 0; j < width; j++) {
      cols[j] = 0;
      for (i = line_start; i <= line_end; i++) {
         if (bitmap_get_bit(bm, j, i)) {
            cols[j]++;
         }
      }
   }

   int char_start = -1;
   int prev_char_end = -1;
   for (i = 0; i < width; i++) {
      if (cols[i] == 0) {
         if (char_start >= 0) {
            if (prev_char_end > -1) {
               if ((char_start - prev_char_end) > get_normal_space_width()) {
                  // is a normal space
                  output_string(" ", STYLE_EITHER);
               } else if (output_is_italic()) {
                  // may an italic space?
                  if ((char_start - prev_char_end) > get_italic_space_width()) {
                     output_string(" ", STYLE_EITHER);
                  }
               }
            }
            process_char(bm, line_start, line_end, baseline, alt_base1, alt_base2, char_start, i - 1, nr, fname);
            prev_char_end = i - 1;
            char_start = -1;
         }
      } else {
         if (char_start < 0) {
            char_start = i;
         }
      }
   }
   
   if (char_start >= 0) {
      if (prev_char_end > -1) {
         if ((char_start - prev_char_end) > get_normal_space_width()) {
            // is a normal space
            output_string(" ", STYLE_EITHER);
         } else if (output_is_italic()) {
            // may an italic space?
            if ((char_start - prev_char_end) > get_italic_space_width()) {
               output_string(" ", STYLE_EITHER);
            }
         }
      }
      process_char(bm, line_start, line_end, baseline, alt_base1, alt_base2, char_start, i - 1, nr, fname);
   }
}

bool approx_equal(int nr1, int nr2) {
   if (get_base_height() > 30) {
      return ((nr1 == nr2)
              || (nr1 == (nr2 + 1))
              || (nr1 == (nr2 + 2))
              || (nr1 == (nr2 + 3))
              || (nr1 == (nr2 - 1))
              || (nr1 == (nr2 - 2))
              || (nr1 == (nr2 - 3)));
   } else if (get_base_height() > 20) {
      return ((nr1 == nr2)
              || (nr1 == (nr2 + 1))
              || (nr1 == (nr2 + 2))
              || (nr1 == (nr2 - 1))
              || (nr1 == (nr2 - 2)));
   } else {
      return ((nr1 == nr2)
              || (nr1 == (nr2 + 1))
              || (nr1 == (nr2 - 1)));
   }
}
               

void process_subtitle(char *fname, int nr, LineSizes *sizes) {
   gl_cur_subtitle = nr;
   bool debug = (gl_debug_subtitle == nr);
   Subtitle sbt = get_subtitle(nr);
   if ((sbt == NULL)
       || (subtitle_bitmap(sbt) == NULL)
       || (sizes == NULL)) {
      if (debug) {
         printf("Subtitle %d in %s is null\n", nr, fname);
      }
      return;
   }

   position pos = subtitle_get_position(sbt);
   if ((pos != TOP) && (pos != BOTTOM) && (pos != CENTER)) {
      // printf("Subtitle %d in %s has position %d\n", nr, fname, pos);
   }

   output_start_item(subtitle_get_start_time(sbt),
                     subtitle_get_end_time(sbt),
                     pos);

   Bitmap bm = subtitle_bitmap(sbt);
   int *scan_lines;
   scan_lines = malloc(subtitle_get_height(sbt) * sizeof(int));
   int i, j;
   for (i = 0; i < subtitle_get_height(sbt); i++) {
      scan_lines[i] = 0;
      for (j = 0; j < subtitle_get_width(sbt); j++) {
         if (bitmap_get_bit(bm, j, i)) {
            scan_lines[i]++;
         }
      }
   }

   // Determined the number of bits in each of the lines. Now try and
   // determine how many lines of text there are, and where the
   // 'baseline' of each text line is.

   int line_nr = 1;
   int line_start = 0;
   while (line_start < subtitle_get_height(sbt)) {
      while ((line_start < subtitle_get_height(sbt)) 
             && (scan_lines[line_start] == 0)) {
         line_start++;
      }
      int line_end = line_start;
      int max_width = 0;
      // Start at the first line with data and try and find the end. Note
      // that we would expect at least the base height for the line height
      while ((line_end < subtitle_get_height(sbt)) 
             && ((scan_lines[line_end] != 0)
                 || (line_end < line_start + sizes->base_height))) {
         if (scan_lines[line_end] > max_width) {
            max_width = scan_lines[line_end];
         }
         line_end++;
      }

      // Trying to calculate the hight of the ascenders, and the
      // baseline (lowest line that represents base characters)
      // for the segment.
      // Note: the ascender value will not be valid when the line
      // contains relatively many capital letters
      int asc = 0;
      int base = 0;
      int alt_base1 = 0;
      int alt_base2 = 0;
      for (i = line_start; i < line_end; i++) {
         if (scan_lines[i] < ((max_width * 35) /100)) {
            if (base == 0) {
               asc++;
            }
         } else {
            base = i;
         }
      }

      alt_base1 = base;
      alt_base2 = base;
      // Check for sanity. 
      if (approx_equal(line_end - line_start, sizes->base_height + sizes->ascender_height + sizes->descender_height)) {
         // line seems to have ascenders and descenders
         if (!approx_equal(base, line_start + sizes->ascender_height + sizes->base_height)) {
            // printf("Subtitle %d in %s: baseline = %d isn't the expected %d\n", nr, fname, base, line_start + sizes->ascender_height + sizes->base_height);
            alt_base1 = line_start + sizes->ascender_height + sizes->base_height;
         }
      } else if (approx_equal(line_end - line_start, sizes->base_height + sizes->ascender_height)
                 || approx_equal(line_end - line_start, sizes->base_height + sizes->descender_height)) {
         // line seems to have ascender or descender, not both
         if (!approx_equal(base, line_start + sizes->ascender_height + sizes->base_height)
             && !approx_equal(base, line_start + sizes->base_height)) {
            // printf("Subtitle %d: baseline = %d isn't the expected %d or %d\n", nr, base, line_start + sizes->ascender_height + sizes->base_height,
            //       line_start + sizes->base_height);
            alt_base1 = line_start + sizes->ascender_height + sizes->base_height;
            alt_base2 = line_start + sizes->base_height;
         }
      } else {
      }
      if (debug) {
         printf("Subtitle %d: found line_start=%d, line_end=%d, baseline=%d\n",
                nr, line_start, line_end, base);
         printf("      base=%d, asc=%d, desc=%d, alt_base1=%d, alt_base2=%d\n",
                sizes->base_height, sizes->ascender_height, sizes->descender_height, 
                alt_base1, alt_base2);
	 dump_bitmap(subtitle_bitmap(sbt));
         // dump_char(subtitle_bitmap(sbt), line_start, line_end, 0, subtitle_get_width(sbt));
      }

      parse_line(sbt, line_start, line_end, base, alt_base1, alt_base2, nr, fname);

      line_start = line_end;
      while ((line_start < subtitle_get_height(sbt)) 
             && (scan_lines[line_start] == 0)) {
         line_start++;
      }
      line_nr++;

      if (line_start < subtitle_get_height(sbt)) {
         output_newline();
      }
   }
   output_end_item();
}

void process_file(char *infile) {
   char *outfile = strdup(infile);
   int len = strlen(outfile);
   if (gl_format == FORMAT_SRT) {
      strcpy(outfile + len - 4, ".srt");
   } else if (gl_format == FORMAT_ASS) {
      strcpy(outfile + len - 4, ".ass");
   }

   if (!strcmp(infile + strlen(infile) - 4, ".sup")) {
      supformat_load(infile);
   } else {
      char *idx_fname = strdup(infile);
      strcpy(idx_fname + strlen(idx_fname) - 4, ".idx");
      printf("# Read index '%s' and sub '%s'\n", idx_fname, infile);
      subformat_load(idx_fname, infile);
   }
   printf("# Files were read\n");

   Subtitle sbt = subtitle_get(0);
   int video_width = 0;
   int video_height = 0;
   if (sbt != NULL) {
      video_width = subtitle_get_video_width(sbt);
      video_height = subtitle_get_video_height(sbt);
   }
   output_open(outfile, gl_format, video_width, video_height);

   if (gl_debug) {
      printf("Process file '%s'\n   output '%s'\n", infile, outfile);
   }
   LineSizes sizes;
   line_sizes_init(&sizes);
   determine_line_properties(&sizes);
   set_base_height(sizes.base_height);
   set_asc_height(sizes.ascender_height);
   set_desc_height(sizes.descender_height);

   if (gl_normal_space_width > 0) {
      // command line override -- changes even data file override
      override_normal_space_width(gl_normal_space_width);
   } else {
      if (sizes.base_height < 20) {
         set_normal_space_width(sizes.base_height / 2);
      } else {
         // value that works for large font
         set_normal_space_width(((sizes.base_height + 1) / 2) - 4);
      }
   }

   if (gl_italic_space_width > 0) {
      // command line override -- changes even data file override
      override_italic_space_width(gl_italic_space_width);
   } else {
      set_italic_space_width(sizes.base_height / 3);
   }

   int i;
   for (i = 0; i < get_nr_subtitles(); i++) {
      process_subtitle(infile, i, &sizes);
   }
   output_close();
   subtitle_clean();
   line_sizes_clean(&sizes);
}

int main(int argc, char *argv[]) {
   read_char_data("readsup.data");

   gl_debug_subtitle = -1;
   gl_debug_block = -1;
   gl_debug = false;

   // Arguments:
   //  -a           output .ass rather than .srt
   //  -b <nr>      debug block number <nr>
   //  -s <nr>      debug subtitle <nr>
   //  -is <nr>     use <nr> as minimal size for italic space
   //  -in <nr>     use <nr> as minimal size for normal space
   //  <filename>   process file <filename>
   if (argc > 0) {
      int i;
      for (i = 1; i < argc; i++) {
         if (!strcmp(argv[i], "-d")) {
            gl_debug = true;
         } else if (!strcmp(argv[i], "-a")) {
            gl_format = FORMAT_ASS;
         } else if (!strcmp(argv[i], "-b")) {
            i++;
            if (i < argc) {
               int value = 0;
               int j = 0;
               for (j = 0; argv[i][j]; j++) {
                  if ((argv[i][j] >= '0')
                      && (argv[i][j] <= '9')) {
                     value = 10 * value + (argv[i][j] - '0');
                  } else {
                     printf("-b parameter with non-numeric value '%s'\n",
                            argv[i]);
                     return 1;
                  }
               }
               gl_debug_block = value;
            } else {
               printf("-b parameter without value\n");
               return 1;
            }
         } else if (!strcmp(argv[i], "-s")) {
            i++;
            if (i < argc) {
               int value = 0;
               int j = 0;
               for (j = 0; argv[i][j]; j++) {
                  if ((argv[i][j] >= '0')
                      && (argv[i][j] <= '9')) {
                     value = 10 * value + (argv[i][j] - '0');
                  } else {
                     printf("-s parameter with non-numeric value '%s'\n",
                            argv[i]);
                     return 1;
                  }
               }
               gl_debug_subtitle = value;
            } else {
               printf("-s parameter without value\n");
               return 1;
            }
         } else if (!strcmp(argv[i], "-is")) {
            i++;
            if (i < argc) {
               int value = 0;
               int j = 0;
               for (j = 0; argv[i][j]; j++) {
                  if ((argv[i][j] >= '0')
                      && (argv[i][j] <= '9')) {
                     value = 10 * value + (argv[i][j] - '0');
                  } else {
                     printf("-is parameter with non-numeric value '%s'\n",
                            argv[i]);
                     return 1;
                  }
               }
               gl_italic_space_width = value;
            } else {
               printf("-is parameter without value\n");
               return 1;
            }
         } else if (!strcmp(argv[i], "-ns")) {
            i++;
            if (i < argc) {
               int value = 0;
               int j = 0;
               for (j = 0; argv[i][j]; j++) {
                  if ((argv[i][j] >= '0')
                      && (argv[i][j] <= '9')) {
                     value = 10 * value + (argv[i][j] - '0');
                  } else {
                     printf("-ns parameter with non-numeric value '%s'\n",
                            argv[i]);
                     return 1;
                  }
               }
               gl_normal_space_width = value;
            } else {
               printf("-ns parameter without value\n");
               return 1;
            }
         } else if (!strcmp(argv[i], "-ths")) {
            i++;
            if (i < argc) {
               int value = 0;
               int j = 0;
               for (j = 0; argv[i][j]; j++) {
                  if ((argv[i][j] >= '0')
                      && (argv[i][j] <= '9')) {
                     value = 10 * value + (argv[i][j] - '0');
                  } else {
                     printf("-ths parameter with non-numeric value '%s'\n",
                            argv[i]);
                     return 1;
                  }
               }
               util_set_threshold(value);
            } else {
               printf("-ths parameter without value\n");
               return 1;
            }
         } else {
            int len = strlen(argv[i]);
            if ((len <= 4)
                || (strcmp(argv[i] + len - 4, ".sup")
                    && strcmp(argv[i] + len - 4, ".sub")
                    )) {
               printf("File '%s' should end in '.sub' or '.sup' to work!\n",
                      argv[i]);
               return 1;
            }
            process_file(argv[i]);
         }
      }
   } else {
      printf("Must provide .sub or .sup file to process\n");
      return 1;
   }

   return 0;
}
