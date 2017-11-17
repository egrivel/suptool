/*
 * subtitle
 *
 * Manage list of subtitles
 */

#define Subtitle struct subtitle_struct *

#include "common.h"
#include "subtitle.h"
#include "bitmap.h"

typedef struct palette_data_struct {
   unsigned char val_Y;
   unsigned char val_Cr;
   unsigned char val_Cb;
   unsigned char val_alpha;
} PaletteData;

typedef struct palette_struct {
   int id;
   int version;
   int size;
   int max_index;
   int threshold; // a total palette value greater than this is a foreground
                  // pixel, otherwise background
   PaletteData *data;
} Palette;

struct subtitle_struct {
   int x_offset;     // in pixels from left edge
   int y_offset;     // in pixels from top edge
   int width;        // in pixels
   int height;       // in pixels
   int video_width;  // in pixels
   int video_height; // in pixels
   long start_time;  // in miliseconds from start of movie
   long end_time;    // in miliseconds from start of movie
   Palette palette;
   Bitmap data;
};

/* ====================================================================== */
/* Manage the subtitle list                                               */
/* ====================================================================== */

#define INITIAL_LIST_CAPACITY    64
Subtitle *gl_subtitle_list = NULL;
int gl_subtitle_count = 0;
int gl_subtitle_capacity = 0;

// Return a pointer to a specific subtitle
Subtitle subtitle_get(int nr) {
   if ((nr >= 0) && (nr < gl_subtitle_count)) {
      return gl_subtitle_list[nr];
   }
   return NULL;
}

// Return a point to the current (latest) subtitle
Subtitle subtitle_get_current() {
   if (gl_subtitle_count > 0) {
      return gl_subtitle_list[gl_subtitle_count - 1];
   }
   return NULL;
}

// Create a new subtitle, make it the current, and return a pointer
// to it
Subtitle subtitle_get_new() {
   if (gl_subtitle_list == NULL) {
      // First time, create the list with initial capacity
      gl_subtitle_capacity = INITIAL_LIST_CAPACITY;
      gl_subtitle_list = malloc(gl_subtitle_capacity * sizeof(void *));
   } else if (gl_subtitle_count == gl_subtitle_capacity) {
      // List reached capacity, re-allocate it with double the capacity
      Subtitle *old = gl_subtitle_list;
      int old_capacity = gl_subtitle_capacity;
      gl_subtitle_capacity *= 2;
      gl_subtitle_list = malloc(gl_subtitle_capacity * sizeof(void *));
      memset(gl_subtitle_list, 0, gl_subtitle_capacity * sizeof(void *));
      if (gl_subtitle_list != NULL) {
         memcpy(gl_subtitle_list, old, old_capacity * sizeof(void *));
      }
      free(old);
   }
   if (gl_subtitle_list != NULL) {
      // Create new subtitle structure
      gl_subtitle_list[gl_subtitle_count] = malloc(sizeof(struct subtitle_struct));
      memset(gl_subtitle_list[gl_subtitle_count], 0, sizeof(struct subtitle_struct));
      gl_subtitle_count++;
      return gl_subtitle_list[gl_subtitle_count - 1];
   }
   return NULL;
}

int subtitle_count() {
   return gl_subtitle_count;
}

/*
 * Clean the subtitle list
 */
void subtitle_clean() {
   int i;
   for (i = 0; i < gl_subtitle_count; i++) {
      if (gl_subtitle_list[i]->data != NULL) {
         bitmap_destroy(gl_subtitle_list[i]->data);
      }
      if (gl_subtitle_list[i]->palette.data != NULL) {
         free(gl_subtitle_list[i]->palette.data);
      }
      free(gl_subtitle_list[i]);
   }
   free(gl_subtitle_list);
   gl_subtitle_list = NULL;
   gl_subtitle_count = 0;
   gl_subtitle_capacity = 0;
}

Bitmap subtitle_bitmap(Subtitle sbt) {
   if (sbt != NULL) {
      if (sbt->data == NULL) {
         sbt->data = bitmap_create();
         // Initialize the bitmat to the subtitle size, if any
         bitmap_set_width(sbt->data, sbt->width);
         bitmap_set_height(sbt->data, sbt->height);
      }
      return sbt->data;
   }
   return NULL;
}

int subtitle_get_height(Subtitle sbt) {
   if (sbt != NULL) {
      return sbt->height;
   }
   return 0;
}

int subtitle_get_width(Subtitle sbt) {
   if (sbt != NULL) {
      return sbt->width;
   }
   return 0;
}

int subtitle_get_video_height(Subtitle sbt) {
   if (sbt != NULL) {
      return sbt->video_height;
   }
   return 0;
}

int subtitle_get_video_width(Subtitle sbt) {
   if (sbt != NULL) {
      return sbt->video_width;
   }
   return 0;
}

long subtitle_get_start_time(Subtitle sbt) {
   if (sbt != NULL) {
      return sbt->start_time;
   }
   return 0;
}

long subtitle_get_end_time(Subtitle sbt) {
   if (sbt != NULL) {
      return sbt->end_time;
   }
   return 0;
}

void subtitle_set_start_time(Subtitle sbt, long start_time) {
   if (sbt != NULL) {
      if (start_time >= 0) {
         sbt->start_time = start_time;
      }
   }
}

void subtitle_set_end_time(Subtitle sbt, long end_time) {
   if (sbt != NULL) {
      if (end_time >= 0) {
         sbt->end_time = end_time;
      }
   }
}

void subtitle_set_position(Subtitle sbt, int x_offset, int y_offset) {
   if (sbt != NULL) {
      if (x_offset >= 0) {
         sbt->x_offset = x_offset;
      }
      if (y_offset >= 0) {
         sbt->y_offset = y_offset;
      }
   }
}

position subtitle_get_position(Subtitle sbt) {
   position pos = BOTTOM;

   if (sbt == NULL) {
      return pos;
   }

   if ((sbt->video_width > 0)
       && (sbt->video_width >= sbt->width)
       && (sbt->video_height > 0)
       && (sbt->video_height >= sbt->height)
       && ((sbt->x_offset + sbt->width) <= sbt->video_width)
       && ((sbt->y_offset + sbt->height) <= sbt->video_height)) {
      // values are valid for computation
      int left_margin = sbt->x_offset;
      int right_margin = sbt->video_width - (sbt->x_offset + sbt->width);
      int top_margin = sbt->y_offset;
      int bottom_margin = sbt->video_height - (sbt->y_offset + sbt->height);

      if (left_margin > (right_margin * 2)) {
         // right aligned
         if (top_margin > (bottom_margin * 2)) {
            // bottom aligned
            pos = BOTTOM_RIGHT;
         } else if (bottom_margin > (top_margin * 2)) {
            // top aligned
            pos = TOP_RIGHT;
         } else {
            // middle aligned
            pos = RIGHT;
         }
      } else if (right_margin > (left_margin * 2)) {
         // left aligned
         if (top_margin > (bottom_margin * 2)) {
            // bottom aligned
            pos = BOTTOM_LEFT;
         } else if (bottom_margin > (top_margin * 2)) {
            // top aligned
            pos = TOP_LEFT;
         } else {
            // middle aligned
            pos = LEFT;
         }
      } else {
         // center aligned
         if (top_margin > (bottom_margin * 2)) {
            // bottom aligned
            pos = BOTTOM;
         } else if (bottom_margin > (top_margin * 2)) {
            // top aligned
            pos = TOP;
         } else {
            // middle aligned
            pos = CENTER;
         }
      }
   }

   if ((pos != TOP)
       && (pos != BOTTOM)
       && (pos != CENTER)) {
   /*
      printf("Video (%dx%d), subtitle (%dx%d) at (%d, %d) results in %d\n",
             sbt->video_width, sbt->video_height,
             sbt->width, sbt->height,
             sbt->x_offset, sbt->y_offset,
             pos);
   */
   }
   return pos;
}

void subtitle_set_size(Subtitle sbt, int width, int height) {
   if (sbt != NULL) {
      if (width >= 0) {
         sbt->width = width;
      }
      if (height >= 0) {
         sbt->height = height;
      }
   }
}

void subtitle_set_video_size(Subtitle sbt, int width, int height) {
   if (sbt != NULL) {
      if (width >= 0) {
         sbt->video_width = width;
      }
      if (height >= 0) {
         sbt->video_height = height;
      }
   }
}

void subtitle_set_palette_ycrcb(Subtitle sbt, int index,
                                int val_Y, int val_Cr, int val_Cb,
                                int val_alpha) {
   if ((sbt == NULL) || (index < 0))  {
      return;
   }

   if ((sbt->palette.size == 0) || (index >= sbt->palette.size)) {
      // Need to expand the palette. Default is to double it
      int new_size;
      if (sbt->palette.size == 0) {
         new_size = 16;
      } else {
         new_size = sbt->palette.size * 2;
      }
      if (index >= new_size) {
         // If doubling doesn't fit, increase size to fit desired index
         new_size = index + 1;
      }

      PaletteData *new_data = malloc(new_size * sizeof(PaletteData));
      memset(new_data, 0, new_size * sizeof(PaletteData));
      if ((sbt->palette.size > 0) && (sbt->palette.data != NULL)) {
         memcpy(new_data, sbt->palette.data, sbt->palette.size * sizeof(PaletteData));
         free(sbt->palette.data);
      }
      sbt->palette.data = new_data;
      sbt->palette.size = new_size;
   }

   sbt->palette.data[index].val_Y = val_Y;
   sbt->palette.data[index].val_Cr = val_Cr;
   sbt->palette.data[index].val_Cb = val_Cb;
   sbt->palette.data[index].val_alpha = val_alpha;

   if (sbt->palette.max_index < index) {
      sbt->palette.max_index = index;
   }
}

/* Analyze the palette, determining which palette entries will result in
   a true bit and which will result in a false bit */
void subtitle_analyze_palette(Subtitle sbt) {
   if (sbt == NULL) {
      return;
   }
   PaletteData *pdata = sbt->palette.data;
   if (pdata == NULL) {
      // no data to analyze
      return;
   }
   int i;
   int max = 0;

   for (i = 0; i <= sbt->palette.max_index; i++) {
      // Simple solution: just add Y, Cr and Cb values to get some
      // approximation of luminosity
      int val = pdata[i].val_Y + pdata[i].val_Cr + pdata[i].val_Cb;
      // Apply alpha value, where alpha = 0xFF is fully opague and
      // alpha = 0x00 is fully transparant, i.e. show the background
      // which is assumed to be black
      val = (val * pdata[i].val_alpha) / 255;
      if (val > max) {
         max = val;
      }
   }
   sbt->palette.threshold = max * 0.9;
}

/*
 * Determine if a pixel of palette entry <index> is visible
 */
bool subtitle_is_visible(Subtitle sbt, int index) {
   bool result = false;
   if ((sbt == NULL) || (sbt->palette.data == NULL)) {
      return result;
   }
   if ((index < 0) || (index > sbt->palette.max_index)) {
      return result;
   }
   if (index >= sbt->palette.size) {
      return result;
   }
   PaletteData *pdata = sbt->palette.data;
   int val = pdata[index].val_Y + pdata[index].val_Cr + pdata[index].val_Cb;
   val = (val * pdata[index].val_alpha) / 255;

   if (val > sbt->palette.threshold) {
      result = true;
   }
   return result;
}
