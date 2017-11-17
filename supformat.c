#include "common.h"
#include "subtitle.h"
#include "supformat.h"

int gl_debug_level = 0;

/* ====================================================================== */
/* Definition of the data structures in the Blu-Ray .sup file format.     */
/* Note: these structure are byte-aligned, hence the need for the pragma. */
/* ====================================================================== */

#define BLOCK_START      0x16
#define BLOCK_POSITION   0x17
#define BLOCK_PALETTE    0x14
#define BLOCK_IMAGE      0x15
#define BLOCK_END        0x80

#pragma pack(1)

// The start of each block, including the PG indicator
struct block_header {
   char start_marker[2];
   int_32 dts;
   int_32 pts;
   int_8  blocktype;
   int_16 length;
};

// The 0x16 start block indicates the start of an event, to be ended
// by a 0x80 zero-length end block. The header's display timestamp is
// the display time when the event takes place.
// The start block is followed by a position block, possible data blocks,
// and an end block to end the event.
struct block_start {
   int_16 video_width;
   int_16 video_height;
   int_8  frame_rate;
   int_16 composition_nr;
   // Composition state is one of:
   //    0x00: normal
   //    0x40: acquisition point (?)
   //    0x80: epoch start
   //    0xc0: epoch continue
   int_8  composition_state;
   int_8  palette_update;
   int_8  palette_id;
   int_8  nr_composition_objects;
};

// Composition blocks optionally follow the start block
struct block_composition {
   int_16 composition_id;
   int_8  window_id;
   int_8  forced_flag;
   int_16 x_position;
   int_16 y_position;
};

struct block_window_desc {
   int_8  window_id;
   int_16 x_offset;
   int_16 y_offset;
   int_16 width;
   int_16 height;
};

// The 0x17 position block. This block follows the event start block and
// identifies where the event takes place
struct block_position {
   int_16 unk1;
   int_16 x_offset;
   int_16 y_offset;
   int_16 width;
   int_16 height;
};

/* Palette entries store a single color in YCrCb format with an alpha value */
struct block_palette_entry {
   int_8 index;
   int_8 val_Y;
   int_8 val_Cr;
   int_8 val_Cb;
   int_8 val_alpha;
};

/* Image block (only first instance; continuation blocks are not handled) */
struct block_image {
   int_16        obj_id;
   int_8         obj_ver;
   int_8         obj_seq;
   unsigned char unknown[3];
   int_16        width;
   int_16        height;
};

/* ====================================================================== */
/* Support functions.                                                     */
/* ====================================================================== */

/**
 * Compute a standard timestamp (miliseconds since the start of the
 * movie) from a .sup data file timestamp, by dividing by 90.
 */
long compute_time(int_32 timestamp) {
   /* divide by 90 */
   return (timestamp + 45) / 90;
}

void decode_image_data(void *data, int data_size,
                       int width, int height) {
   unsigned char palette[256];
   memset(palette, ' ', 256);

   Subtitle sbt = subtitle_get_current();
   if (sbt == NULL) {
      printf("Got NULL current subtitle\n");
      return;
   }
   subtitle_analyze_palette(sbt);

   Bitmap bm = subtitle_bitmap(sbt);

   unsigned char *buf = (unsigned char *)data;
   int idx = 0;

   int cur_x = 0;
   int cur_y = 0;

   int i = 0;
   int size = 0;

   subtitle_set_size(sbt, width, height);

   while ((idx < data_size) && (cur_y < height)) {
      int byte = buf[idx++];
      if (byte == 0) {
         if (idx >= data_size) {
            printf("Idx value over data size\n");
         }
         byte = buf[idx++];
         if (byte == 0) {
            // 00 00 indicates end of line
            if (cur_x > 0) {
               // padd the line with 0 values
               while (cur_x < width) {
                  bitmap_set_bit(bm, cur_x, cur_y,
                                 subtitle_is_visible(sbt, 0));
                  cur_x++;
               }
               cur_y++;
               cur_x = 0;
            }
         } else if ((byte & 0xC0) == 0x40) {
            // 00 4x xx means xxx zeroes
            size = (byte - 0x40) << 8;
            if (idx >= data_size) {
               printf("Idx value over data size\n");
            }
            size += buf[idx++];
            for (i = 0; i < size; i++) {
               bitmap_set_bit(bm, cur_x, cur_y,
                              subtitle_is_visible(sbt, 0));
               cur_x++;
               if (cur_x >= width) {
                  cur_x = 0;
                  cur_y++;
               }
            }
         } else if ((byte & 0xC0) == 0x80) {
            // 00 8x yy means x times value y
            size = byte - 0x80;
            if (idx >= data_size) {
               printf("Idx value over data size\n");
            }
            int pixel = buf[idx++];
            for (i = 0; i < size; i++) {
               bitmap_set_bit(bm, cur_x, cur_y,
                              subtitle_is_visible(sbt, pixel));
               cur_x++;
               if (cur_x >= width) {
                  cur_x = 0;
                  cur_y++;
               }
            }
         } else if ((byte & 0xC0) != 0) {
            // 00 Cx yy zz means xyy times value z
            size = (byte - 0xC0) << 8;
            if (idx >= data_size) {
               printf("Idx value over data size\n");
            }
            size += buf[idx++];
            if (idx >= data_size) {
               printf("Idx value over data size\n");
            }
            int pixel = buf[idx++];
            for (i = 0; i < size; i++) {
               bitmap_set_bit(bm, cur_x, cur_y,
                              subtitle_is_visible(sbt, pixel));
               cur_x++;
               if (cur_x >= width) {
                  cur_x = 0;
                  cur_y++;
               }
            }
         } else {
            // 00 xx means xx times 0
            size = byte;
            for (i = 0; i < size; i++) {
               bitmap_set_bit(bm, cur_x, cur_y,
                              subtitle_is_visible(sbt, 0));
               cur_x++;
               if (cur_x >= width) {
                  cur_x = 0;
                  cur_y++;
               }
            }
         }
      } else {
         // byte itself
         bitmap_set_bit(bm, cur_x, cur_y,
                        subtitle_is_visible(sbt, byte));
         cur_x++;
         if (cur_x >= width) {
            cur_x = 0;
            cur_y++;
         }
      }
   }
   // printf("\nBlock done, cur_x=%d, cur_y=%d, width=%d, height=%d\n",
   //        cur_x, cur_y, width, height);
}

int process_block_start(FILE *fp, struct block_header *header, long offset) {
   int length_to_read = header->length;
   struct block_start start;
   if (length_to_read < sizeof(start)) {
      printf("At offset %lx trying to read start block (%ld bytes), only %d bytes available\n",
             offset, sizeof(start), length_to_read);
      // not enough data to read
      return 1;
   }
   fread(&start, 1, sizeof(start), fp);
   length_to_read -= sizeof(start);
   start.video_width = swap_int_16(start.video_width);
   start.video_height = swap_int_16(start.video_height);
   start.composition_nr = swap_int_16(start.composition_nr);

   //printf("Offset %lx, start block %d, %dx%d, state 0x%2.2x, %d blocks\n",
   //        offset, start.composition_nr, start.video_width, start.video_height,
   //        start.composition_state, start.nr_composition_objects);

   // 8bit composition_state:
   //   - 0x00: normal
   //   - 0x40: acquisition point
   //   - 0x80: epoch start
   //   - 0xc0: epoch continue
   if (start.composition_state == 0x80) {
      // start of a new "epoch" (subtitle)
      Subtitle sbt = subtitle_get_new();
      if (sbt != NULL) {
         subtitle_set_start_time(sbt, compute_time(header->dts));
         subtitle_set_video_size(sbt, start.video_width, start.video_height);
         // printf("   start subtitle at ");
         // print_time(subtitle->start_time);
         // printf("\n");
      }
   } else if (start.composition_state == 0x00) {
      //0:10:42,560 --> 0:10:44,391
      Subtitle sbt = subtitle_get_current();
      if (sbt != NULL) {
         subtitle_set_end_time(sbt, compute_time(header->dts));
         // printf("   end subtitle at ");
         // print_time(subtitle->end_time);
         // printf(", started at ");
         // print_time(subtitle->start_time);
         // printf("\n");
         // printf("   size %dx%d placed at (%d, %d)\n",
         //        subtitle->width, subtitle->height,
         //        subtitle->x_offset, subtitle->y_offset);
      }
   } else if (start.composition_state == 0x40) {
      // "acquisition point", which should re-iterate the same info
      // (video size) that was in the start of the epoch
   } else if (start.composition_state == 0xc0) {
      // "epoch continue", ignore this
   } else {
      printf("Offset %ld, unrecognized composition state %2.2x\n",
             offset, start.composition_state);
   }
   int i;
   for (i = 0; i < start.nr_composition_objects; i++) {
      struct block_composition composition;
      if (length_to_read < sizeof(composition)) {
         printf("At offset %lx trying to read composition block (%ld bytes), only %d bytes available\n",
                offset, sizeof(composition), length_to_read);
         // not enough data to read
         return 1;
      }
      fread(&composition, 1, sizeof(composition), fp);
      length_to_read -= sizeof(composition);
      composition.composition_id = swap_int_16(composition.composition_id);
      composition.x_position = swap_int_16(composition.x_position);
      composition.y_position = swap_int_16(composition.y_position);
      // printf("   composition %d: at (%d, %d)\n",
      //        i, composition.x_position, composition.y_position);
      Subtitle sbt = subtitle_get_current();
      if (sbt != NULL) {
         subtitle_set_position(sbt,
                               composition.x_position,
                               composition.y_position);
      }
   }
   if (length_to_read) {
      printf("At offset %lx still %d bytes to read\n", offset, length_to_read);
      return 1;
   }
   return 0;
}

int process_block_window(FILE *fp, struct block_header *header, long offset) {
   unsigned char nr_windows;
   int length_to_read = header->length;
   if (length_to_read < 1) {
      return 1;
   }

   fread(&nr_windows, 1, 1, fp);
   length_to_read--;

   int i;
   for (i = 0; i < nr_windows; i++) {
      struct block_window_desc window_desc;
      if (length_to_read < sizeof(window_desc)) {
         return 1;
      }
      fread(&window_desc, 1, sizeof(window_desc), fp);
      length_to_read -= sizeof(window_desc);
      window_desc.x_offset = swap_int_16(window_desc.x_offset);
      window_desc.y_offset = swap_int_16(window_desc.y_offset);
      window_desc.width = swap_int_16(window_desc.width);
      window_desc.height = swap_int_16(window_desc.height);
      Subtitle sbt = get_current_subtitle();
      if (sbt != NULL) {
         subtitle_set_position(sbt,
                               window_desc.x_offset,
                               window_desc.y_offset);
         subtitle_set_size(sbt,
                           window_desc.width,
                           window_desc.height);
      }
   }
   if (length_to_read) {
      return 1;
   }
   return 0;
}

int process_block_palette(FILE *fp, struct block_header *header, long offset) {
   int length_to_read = header->length;

   unsigned char palette_id;
   unsigned char palette_version;

   if (length_to_read < 1) {
      return 1;
   }
   fread(&palette_id, 1, 1, fp);
   length_to_read--;

   if (length_to_read < 1) {
      return 1;
   }
   fread(&palette_version, 1, 1, fp);
   length_to_read--;

   int palette_size = length_to_read / sizeof(struct block_palette_entry);
   if ((sizeof(struct block_palette_entry) * palette_size) != length_to_read) {
      // remaining data has to be a multiple of the palette entry size
      return 1;
   }

   struct block_palette_entry *data;
   data = malloc(palette_size * sizeof(struct block_palette_entry));
   if (data == NULL) {
      return 1;
   }
   fread(data, sizeof(struct block_palette_entry), palette_size, fp);

   int i;
   Subtitle sbt = subtitle_get_current();
   for (i = 0; i < palette_size; i++) {
      subtitle_set_palette_ycrcb(sbt,
                                 data[i].index,
                                 data[i].val_Y,
                                 data[i].val_Cr,
                                 data[i].val_Cb,
                                 data[i].val_alpha);
   }

   free(data);
   return 0;
}

int process_block_image(FILE *fp, struct block_header *header,
                        long offset, int isFirst) {
   int length_to_read = header->length;
   struct block_image image;
   if (length_to_read < sizeof(image)) {
      // not enough data
      return 1;
   }
   if (!isFirst) {
      // Can't deal with subsequent image blocks 
      printf("Follow-up image blocks not supported\n");
      return 1;
   }
   fread(&image, 1, sizeof(image), fp);
   length_to_read -= sizeof(image);
   image.obj_id = swap_int_16(image.obj_id);
   image.width = swap_int_16(image.width);
   image.height = swap_int_16(image.height);

   if (length_to_read > 0) {
      unsigned char *data = malloc(length_to_read);
      fread(data, 1, length_to_read, fp);
      decode_image_data(data, length_to_read, image.width, image.height);
      length_to_read = 0;
   }
   return 0;
}

// Block type 128 marks the end of a section
long process_block_128(FILE *fp, struct block_header *header, long offset) {
   if (header->length != 0) {
      printf("Block type 128 at offset %lx has non-zero length (%d)\n",
             offset, header->length);
   }
   return 0;
}

/**
 * Load the contents of a suptitle file.
 * Return true on success, false on failure.
 */
bool supformat_load(char *fname) {
   bool success = true;  // so far so good

   FILE *fp = fopen(fname, "rb");
   if (fp == NULL) {
      printf("Error: file '%s' cannot be opened\n", fname);
      return false;
   }

   struct block_header header;
   size_t size;

   long offset = 0;
   int firstOds = 1;


   while ((size = fread(&header, 1, sizeof(header), fp)) == sizeof(header)) {
      char marker[3];
      marker[0] = header.start_marker[0];
      marker[1] = header.start_marker[1];
      marker[2] = '\0';
      if (strcmp(marker, "PG")) {
         printf("Error: block at offset %lx doesn't start with \"PG\" but with \"%s\"\n", offset, marker);
         success = false;
         break;
      }

      // Fix byte order of numbers
      header.dts = swap_int_32(header.dts);
      header.pts = swap_int_32(header.pts);
      header.length = swap_int_16(header.length);

      offset += sizeof(header);

      switch (header.blocktype) {
      case BLOCK_PALETTE:
         if (gl_debug_level > 0) {
            printf("Process palette block\n");
         }
         if (process_block_palette(fp, &header, offset)) {
            success = false;
         }
         offset += header.length;
         break;
      case BLOCK_IMAGE:
         if (gl_debug_level > 0) {
            printf("Process image block\n");
         }
         if (process_block_image(fp, &header, offset, firstOds)) {
            success = false;
         }
         offset += header.length;
         firstOds = 0;
         break;
      case BLOCK_START:
         if (gl_debug_level > 0) {
            printf("Process start block\n");
         }
         if (process_block_start(fp, &header, offset)) {
            success = false;
         }
         offset += header.length;
         firstOds = 1;
         break;
      case BLOCK_POSITION:
         if (gl_debug_level > 0) {
            printf("Process position block\n");
         }
         if (process_block_window(fp, &header, offset)) {
            success = false;
         }
         offset += header.length;
         break;
      case BLOCK_END:
         if (gl_debug_level > 0) {
            printf("Process end block\n");
         }
         offset += process_block_128(fp, &header, offset);
         firstOds = 1;
         break;
      default:
         printf("   Block type 0x%2.2x not recognized (skipped)\n", header.blocktype);
         fseek(fp, (long)header.length, SEEK_CUR);
         offset += (long)header.length;
         break;
      }
      if (!success) {
         break;
      }
   }
   fseek(fp, 0L, SEEK_END);
   long sz = ftell(fp);
   if (sz != offset) {
      printf("End at offset %ld, file size %ld\n", offset, sz);
   }
   fclose(fp);
   return success;
}
