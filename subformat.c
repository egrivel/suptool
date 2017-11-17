#include "common.h"
#include "subformat.h"
#include "subtitle.h"
#include "util.h"

// Index contains the list of subtitle images, with the timestamp at which to
// display and the offset of the image in the file
typedef struct  {
   char *timestamp;
   long timestampNum;
   long offset;
} index_s;

index_s *sub_index = NULL;
int sub_index_count = 0;
int sub_index_allocated = 0;
int sub_width = 0;
int sub_height = 0;
int sub_x_offset = 0;
int sub_y_offset = 0;

// When a frame doesn't have a delay specified, used the previous one
int prevDelay = 0;

#define SUB_NR_PALETTE 16
char *sub_palette[SUB_NR_PALETTE];
int sub_palette_count = 0;

int sub_lastAlpha[4];

bool sub_debug = false;
#define DEBUG1(x1) if (sub_debug) { printf(x1); } 
#define DEBUG2(x1, x2) if (sub_debug) { printf(x1, x2); } 
#define DEBUG3(x1, x2, x3) if (sub_debug) { printf(x1, x2, x3); } 
#define DEBUG4(x1, x2, x3, x4) if (sub_debug) { printf(x1, x2, x3, x4); } 
#define DEBUG5(x1, x2, x3, x4, x5) if (sub_debug) { printf(x1, x2, x3, x4, x5); } 

void sub_reset() {
   int i;
   if (sub_index) {
      for (i = 0; i < sub_index_count; i++) {
         free(sub_index[i].timestamp);
      }
      free(sub_index);
   }
   sub_index = NULL;
   sub_index_count = 0;
   sub_index_allocated = 0;

   for (i = 0; i < SUB_NR_PALETTE; i++) {
      if (i < sub_palette_count) {
         free(sub_palette[i]);
      }
      sub_palette[i] = NULL;
   }
   sub_palette_count = 0;

   sub_width = 0;
   sub_height = 0;
   sub_x_offset = 0;
   sub_y_offset = 0;

   for (i = 0; i < 4; i++) {
      // initialize previous alpha
      sub_lastAlpha[i] = 0;
   }

   prevDelay = 15000;
}

long timeToNum(char *str) {
   int hr, min, sec, ms;
   sscanf(str, "%02d:%02d:%02d:%03d", &hr, &min, &sec, &ms);
   long result = hr;
   result = 60 * result + min;
   result = 60 * result + sec;
   result = 1000 * result + ms;
   return result * 90;
}

char *timeFromNum(long num) {
   static char buffer[15];
   num /= 90;
   int ms = num - 1000 * (num / 1000);
   num /= 1000;
   int sec = num - 60 * (num / 60);
   num /= 60;
   int min = num - 60 * (num / 60);
   num /= 60;
   int hr = num - 60 * (num / 60);
   num /= 60;
   sprintf(buffer, "%02d:%02d:%02d:%03d", hr, min, sec, ms);
   if (num > 0) {
      strcat(buffer, "*");
   }
   return buffer;
}

// Add one entry to the index
void sub_add_index_entry(char *timestamp, long filepos) {
   if (sub_index_count == sub_index_allocated) {
      // current index is full, extend it
      sub_index_allocated += 64;
      index_s *new_index = malloc(sub_index_allocated * sizeof(index_s));
      if (sub_index_count) {
         // copy over existing data
         memcpy(new_index, sub_index, sub_index_count * sizeof(index_s));
      }
      if (sub_index) {
         // free old data
         free(sub_index);
      }
      sub_index = new_index;
   }
   sub_index[sub_index_count].timestamp = strdup(timestamp);
   sub_index[sub_index_count].timestampNum = timeToNum(timestamp);
   sub_index[sub_index_count].offset = filepos;
   sub_index_count++;
}

void sub_set_size(int width, int height) {
   DEBUG3("Set global size (%d, %d)\n", width, height);
   sub_width = width;
   sub_height = height;
}

void sub_set_offset(int x_off, int y_off) {
   DEBUG3("Set global offset (%d, %d)\n", x_off, y_off);
   sub_x_offset = x_off;
   sub_y_offset = y_off;
}

void sub_add_palette(char *value) {
   if (sub_palette_count < SUB_NR_PALETTE) {
      sub_palette[sub_palette_count] = strdup(value);
      sub_palette_count++;
   } else {
      printf("Error: setting palette %d > %d\n", sub_palette_count + 1, SUB_NR_PALETTE);
   }
}

//
// Read the .idx file; only some of the data from the file is retrieved,
// the rest is ignored.
//
void sub_read_idx(char *idx_fname) {
   char buffer[1024];

   FILE *fp = fopen(idx_fname, "rt");
   while (!feof(fp)) {
      buffer[0] = '\0';
      fgets(buffer, sizeof(buffer), fp);
      if (strlen(buffer)) {
         if ((buffer[strlen(buffer) - 1] == '\r')
             || (buffer[strlen(buffer) - 1] == '\n')) {
            buffer[strlen(buffer) - 1] = '\0';
         }
      }
      if (strlen(buffer)) {
         if ((buffer[strlen(buffer) - 1] == '\r')
             || (buffer[strlen(buffer) - 1] == '\n')) {
            buffer[strlen(buffer) - 1] = '\0';
         }
      }

      if ((strlen(buffer) > 6) && !memcmp(buffer, "size: ", 6)) {
         int width, height;
         sscanf(buffer, "size: %dx%d", &width, &height);
         sub_set_size(width, height);
      } else if ((strlen(buffer) > 5) && !memcmp(buffer, "org: ", 5)) {
         int x_off, y_off;
         sscanf(buffer, "org: %d, %d", &x_off, &y_off);
         sub_set_offset(x_off, y_off);
      } else if ((strlen(buffer) > 0) && !memcmp(buffer, "palette: ", 9)) {
         // Palette entry is a line with palette vaues (six hexadecimal vaues),
         // separated by a comma and a space.
         char *ptr = buffer + 9;
         while (strlen(ptr) >= 6) {
            char temp = ptr[6];
            ptr[6] = '\0';
            sub_add_palette(ptr);
            ptr[6] = temp;
            ptr += 6;
            if (*ptr) {
               ptr++;
            }
            if (*ptr) {
               ptr++;
            }
         }
      } else if ((strlen(buffer) == 43) && !memcmp(buffer, "timestamp: ", 11)) {
         //           1         2        3          4
         // 0123456789012345678901234567890123456789012
         // timestamp: 00:01:00:920, filepos: 000000000
         char timestamp[13];
         char filepos_str[10];
         memcpy(timestamp, buffer + 11, 12);
         timestamp[12] = '\0';
         memcpy(filepos_str, buffer + 34, 9);
         filepos_str[9] = '\0';
         long filepos = strtol(filepos_str, NULL, 16);
         sub_add_index_entry(timestamp, filepos);
      }
   }
   fclose(fp);
}

//
// Section to read simulates the subdvd.cpp source. 
//
typedef int_64 qint64;
typedef unsigned char uchar;

unsigned char *sub_buffer = NULL;
long sub_offset = 0L;
long sub_block_length = 0L;

void sub_getBuffer(FILE *fp, long offset, long block_length) {
   DEBUG5("Read data at %ld (0x%lx) size %ld (0x%lx)\n",
          offset, offset, block_length, block_length);

   sub_offset = offset;
   sub_block_length = block_length;
   sub_buffer = malloc(sub_block_length);

   fseek(fp, offset, SEEK_SET);
   fread(sub_buffer, 1, sub_block_length, fp);
}

void sub_releaseBuffer() {
   free(sub_buffer);
   sub_offset = 0L;
   sub_block_length = 0L;
}

int_32 sub_getDWord(int_64 ofs) {
   long real_offset = ofs - sub_offset;
   if ((real_offset < 0) || (real_offset >= sub_block_length)) {
      printf("Error: getDWord(0x%lx) outside of (0x%lx - 0x%lx)\n",
             ofs, sub_offset, sub_offset + sub_block_length);
   }
   return swap_int_32(*((int_32*)(sub_buffer + real_offset)));
}

int_16 sub_getWord(int_64 ofs) {
   long real_offset = ofs - sub_offset;
   if ((real_offset < 0) || (real_offset >= sub_block_length)) {
      printf("Error: getWord(0x%lx) outside of (0x%lx - 0x%lx)\n",
             ofs, sub_offset, sub_offset + sub_block_length);
   }
   return swap_int_16(*((int_16*)(sub_buffer + real_offset)));
}

uchar sub_getByte(int_64 ofs) {
   long real_offset = ofs - sub_offset;
   if ((real_offset < 0) || (real_offset >= sub_block_length)) {
      printf("Error: getByte(0x%lx) outside of (0x%lx - 0x%lx)\n",
             ofs, sub_offset, sub_offset + sub_block_length);
   }
   return *((uchar *)(sub_buffer + real_offset));
}

/* === RLE fragments === */
unsigned char *sub_fragments = NULL;
long sub_fragments_size = 0;

void sub_startRle() {
   sub_fragments = NULL;
   sub_fragments_size = 0;
}

void sub_addRleFrag(long offset, long size) {
   DEBUG3("Add fragment size %ld, prior %ld\n", size, sub_fragments_size);
   offset -= sub_offset;
   if (sub_fragments_size) {
      unsigned char *temp = NULL;
      temp = malloc(size + sub_fragments_size);
      if (temp == NULL) {
         printf("ERROR: malloc failed\n");
         exit(0);
      }
      memcpy(temp, sub_fragments, sub_fragments_size);
      if ((offset + size) >= sub_block_length) {
         printf("ERROR: fragment end %ld+%ld extends beyond %ld\n",
                offset, size, sub_block_length);
         return;
      }
      memcpy(temp + sub_fragments_size, sub_buffer + offset, size);
      free(sub_fragments);
      sub_fragments = temp;
      sub_fragments_size += size;
   } else {
      sub_fragments = malloc(size);
      memcpy(sub_fragments, sub_buffer + offset, size);
      sub_fragments_size = size;
   }
}

void sub_setRleSize(long size) {
   DEBUG2("Set RLE size to %ld\n", size);
   if (size != sub_fragments_size) {
      printf("WARNING: got size %ld, setting to %ld\n",
             sub_fragments_size, size);
   }
}

void sub_endRle() {
   DEBUG2("Done with fragments, total size %ld\n", sub_fragments_size);

   /*
   int i;
   for (i = 0; i < sub_fragments_size; i++) {
      printf(" %02x", sub_fragments[i]);
      if ((i % 16) == 15) {
         printf("\n");
      }
   }
   printf("\n");
   */

   if (sub_fragments_size) {
      free(sub_fragments);
      sub_fragments = NULL;
      sub_fragments_size = 0;
   }
}

/* === RLE palette === */
#define SUB_RLE_PALETTE_SIZE 4
unsigned char sub_rle_palette[SUB_RLE_PALETTE_SIZE];

void sub_setPalette(int nr, unsigned char value) {
   if (nr < SUB_RLE_PALETTE_SIZE) {
      sub_rle_palette[nr] = value;
   } else {
      printf("ERROR: setting palette %d > %d\n", nr, SUB_RLE_PALETTE_SIZE);
   }
}

unsigned char sub_getPalette(int nr) {
   if (nr < SUB_RLE_PALETTE_SIZE) {
      return sub_rle_palette[nr];
   } else {
      printf("ERROR: getting palette %d > %d\n", nr, SUB_RLE_PALETTE_SIZE);
      return 0;
   }
}

/* === RLE alpha === */
#define SUB_RLE_ALPHA_SIZE  4
int sub_rle_alpha[SUB_RLE_ALPHA_SIZE];

void sub_setAlpha(int nr, int value) {
   if (nr < SUB_RLE_ALPHA_SIZE) {
      sub_rle_alpha[nr] = value;
   } else {
      printf("ERROR: setting alpha %d > %d\n", nr, SUB_RLE_ALPHA_SIZE);
   }
   sub_rle_alpha[nr] = value;
}

int sub_getAlpha(int nr) {
   if (nr < SUB_RLE_ALPHA_SIZE) {
      return sub_rle_alpha[nr];
   } else {
      printf("ERROR: getting alpha %d > %d\n", nr, SUB_RLE_ALPHA_SIZE);
      return 0;
   }
   return sub_rle_alpha[nr];
}

bool sub_getFixZeroAlpha() {
   // I guess that we use the feature to use the previous alpha if the
   // new alpha is zero? dunno what this means
   return true;
}

/* === RLE forced frames === */
int sub_numForcedFrames = 0;

void sub_setForced(bool f) {
   // Not sure that I care about forced frames at all
   // printf("Setting forced to %s\n", f ? "true" : "false");
   if (f) {
      sub_numForcedFrames++;
   }
}

/* === RLE image and window size === */

int sub_img_x = 0;
int sub_img_y = 0;
int sub_img_width = 0;
int sub_img_height = 0;

void sub_setImageSizes(int x, int y, int w, int h) {
   DEBUG5("Set image size top left (%d, %d) size (%d, %d)\n",
          x, y, w, h);
   sub_img_x = x;
   sub_img_y = y;
   sub_img_width = w;
   sub_img_height = h;
   if (sub_img_width > sub_width) {
      printf("ERROR: setting image width %d greater than video width %d\n",
             sub_img_width, sub_width);
   }
   if (sub_img_height > sub_height) {
      printf("ERROR: setting image height %d greater than video height %d\n",
             sub_img_height, sub_height);
   }
   Subtitle sbt = subtitle_get_current();
   subtitle_set_size(sbt, w, h);
}

void sub_setWindowSizes(int x, int y, int w, int h) {
   DEBUG5("Set window size top left (%d, %d) size (%d, %d)\n",
          x, y, w, h);
}

void sub_setNumberOfWindows(int nr) {
   DEBUG2("Set number of windows to %d\n", nr);
}

/* === RLE objects === */
void sub_setNumCompObjects(int nr) {
   if (nr != 1) {
      printf("WARNING: got %d objects, expected 1\n", nr);
   }
}

void sub_addObject() {
   // assuming we don't care since it's always one object
}

/* === RLE offsets === */
int sub_odd_offset = 0L;
int sub_even_offset = 0L;
int sub_odd_size = 0L;
int sub_even_size = 0L;

void sub_setOddOffset(int nr) {
   DEBUG2("Setting odd offset to %d\n", nr);
   sub_odd_offset = nr;
}

void sub_setEvenOffset(int nr) {
   DEBUG2("Setting even offset to %d\n", nr);
   sub_even_offset = nr;
}

/* === RLE timing === */
long sub_startTime;
long sub_endTime;

void sub_setStartTime(long t) {
   sub_startTime = t;
}

long sub_getStartTime() {
   return sub_startTime;
}

void sub_setEndTime(long t) {
   sub_endTime = t;
}

long sub_getEndTime() {
   return sub_endTime;
}

/*
 * Read a single frame. Basic information comes from the subdvd.cpp of the
 * BDSub2SubPlusPlus tool, and from the following additional sources:
 * https://en.wikipedia.org/wiki/MPEG_program_stream
 * http://www.mpucoder.com/DVD/pes-hdr.html
 * http://flavor.sourceforge.net/samples/mpeg2ps.htm
 *
 * Variables match the source in BDSub2SubPlusPlus/src/Subtitle/subdvd.c
 * function readSubFrame.
 */
void sub_read_frame(FILE *fp, long offset, long block_length) {
   sub_getBuffer(fp, offset, block_length);

   qint64 endOfs = offset + block_length;
   qint64 ofs = offset;
   qint64 ctrlOfs = -1;
   qint64 nextOfs = 0;       // value: offset inside the buffer
   int ctrlOfsRel = 0;
   int rleSize = 0;
   int rleBufferFound = 0;
   UNUSED(rleBufferFound);
   int ctrlSize = -1;
   int ctrlHeaderCopied = 0;
   unsigned char *ctrlHeader;
   int length;
   int packHeaderSize;
   bool firstPackFound = false;

   // fixed, stream ID should always be zero...??? From subdvd.h.
   int streamID = 0;
   int i;

   int count = 0;
   do {
      count++;
      DEBUG2("got count %d\n", count);
      DEBUG3("Got ofs of buffer+%ld (0x%lx)\n", nextOfs, nextOfs);

      qint64 startOfs = ofs;
      int_32 packetIdentifier = sub_getDWord(ofs);
      // packet identifier is always 0x000001ba
      DEBUG2("Packet identifier 0x%08x\n", packetIdentifier);
      if ((packetIdentifier != 0x000001ba) && (packetIdentifier != 0x000001be)) {
         printf("Unknown packet identifier: %08x instead of 000001ba\n",
                packetIdentifier);
         exit(0);
      }

      ofs += 4;
      // the following 48 bits are about the system clock reference, see the
      // Wikipedia article for details
      ofs += 6;

      // the following 24 bits are about the bit rate, see the Wikipedia article
      // for details
      ofs += 3;

      // lower three bits give a "stuffing length" (in bytes)
      int stuffOfs = sub_getByte(ofs) & 7;

      ofs += 1 + stuffOfs; // skip stuffing info

      // now starting the Packetized Elementary Stream (PES) header
      // Subpacket ID would always be 0x000001bd (private stream 1, non-mpeg
      // audio, subpictures)
      int_32 subpacket_id = sub_getDWord(ofs);
      if (subpacket_id != 0x000001bd) {
         printf("WARNING: Got subpacket ID: %08x instead of 0x000001bd\n", subpacket_id);
      }

      // 2 bytes:  packet length (number of bytes after this entry)
      length = sub_getWord(ofs += 4);
      DEBUG3("Got packet length %d (0x%x)\n", length, length);

      nextOfs = ofs + 2 + length;
      DEBUG3("next offset is %ld (0x%lx)\n",
             (long)(nextOfs + offset), (long)(nextOfs + offset));

      // 2 bytes:  packet type
      ofs += 2;
      packHeaderSize = (int)(ofs - startOfs);
      DEBUG3("packHeaderSize = %d (0x%x)\n", packHeaderSize, packHeaderSize);

      bool firstPack = ((sub_getByte(++ofs) & 0x80) == 0x80);
      DEBUG2("firstPack = %d\n", firstPack);

      // 1 byte    pts length
      int ptsLength = sub_getByte(ofs += 1);
      DEBUG2("PTS length = %d\n", ptsLength);

      ofs += (1 + ptsLength); // skip PTS and stream ID
      int packetStreamID = sub_getByte(ofs++) - 0x20;
      DEBUG2("Got packet stream ID %02x\n", packetStreamID);
      if (packetStreamID != streamID) {
         // packet doesn't begin stream -> skip
         if ((nextOfs % 0x800) != 0) {
            ofs = ((nextOfs / 0x800) + 1) * 0x800;
       
            printf("WARNING: Offset to next fragment is invalid. Fixed to: %ld (0x%lx)\n", ofs, ofs);
         } else {
            ofs = nextOfs;
         }
         ctrlOfs += 0x800;
         continue;
      }

      int headerSize = (int)(ofs - startOfs); // only valid for additional packets
      DEBUG2("Got header size %d\n", headerSize);
      if (firstPack && ptsLength >= 5) {
         int size = sub_getWord(ofs);
         ofs += 2;
         ctrlOfsRel = sub_getWord(ofs);
         rleSize = ctrlOfsRel - 2;             // calculate size of RLE buffer
         ctrlSize = (size - ctrlOfsRel) - 2;       // calculate size of control header
         DEBUG5("found size %d (0x%x), rleSize %d (0x%x),\n",
                size, size, rleSize, rleSize);
         DEBUG5("found ctrlOfsRel %d (0x%x), ctrlSize %d (0x%x)\n",
                ctrlOfsRel, ctrlOfsRel, ctrlSize, ctrlSize);
         if (ctrlSize < 0) {
            printf("ERROR: Invalid control buffer size %d\n", ctrlSize);
            exit(0);
         }
         ctrlHeader = malloc(ctrlSize);
         ctrlOfs = ctrlOfsRel + ofs; // might have to be corrected for multiple packets
         ofs += 2;
         headerSize = (int)(ofs - startOfs);
         DEBUG2("Got header size %d\n", headerSize);
         firstPackFound = true;
      } else {
         if (firstPackFound) {
            ctrlOfs += headerSize; // fix absolute offset by adding header bytes
            DEBUG3("CtrlOfs becomes %ld (0x%lx)\n", ctrlOfs, ctrlOfs);
         } else {
            printf("WARNING: Invalid fragment skipped at offset %ld (0x%lx)\n",
                   startOfs, startOfs);
         }
      }

      // check if control header is (partly) in this packet
      int diff = (int)((nextOfs - ctrlOfs) - ctrlHeaderCopied);
      if (diff < 0) {
         DEBUG2("negative diff %d becomes 0\n", diff);
         diff = 0;
      }

      int copied = ctrlHeaderCopied;
      DEBUG5("Starting loop, diff=%d (0x%x), copied=%d (0x%x)\n",
             diff, diff, copied, copied);
      for (i = 0; (i < diff) && (ctrlHeaderCopied < ctrlSize); ++i) {
         ctrlHeader[ctrlHeaderCopied] = 
            (uchar)sub_getByte(ctrlOfs + i + copied);
         ++ctrlHeaderCopied;
      }

      int rlePacketSize = ((length - headerSize) - diff) + packHeaderSize;
      DEBUG3("RLE fragment offset: 0x%lx, size: 0x%x\n",
             ofs, rlePacketSize);
      
      // This is where the actual RLE stuff is build up.
      sub_addRleFrag(ofs, rlePacketSize);
      /*
      rleFrag = ImageObjectFragment();
      rleFrag.setImageBufferOffset(ofs);
      rleFrag.setImagePacketSize(rlePacketSize);
      pic.rleFragments.push_back(rleFrag);
      */
      rleBufferFound += rlePacketSize;

      if (ctrlHeaderCopied != ctrlSize && ((nextOfs % 0x800) != 0)) {
         ofs = ((nextOfs / 0x800) + 1) * 0x800;
         
         printf("WARNING: Offset to next fragment is invalid. Fixed to: 0x%lx\n",
                ofs);
         rleBufferFound += ofs-nextOfs;
      } else {
         ofs = nextOfs;
      }
      
   } while (ofs < endOfs && ctrlHeaderCopied < ctrlSize);

   // done reading from the file
   sub_releaseBuffer();

   // now start processing the RLE data
   if (ctrlHeaderCopied != ctrlSize) {
      printf("WARNING: Control buffer size inconsistent.\n");
      
      for (i = ctrlHeaderCopied; i < ctrlSize; ++i) {
         ctrlHeader[i] = 0xff;
      }
   }

   if (rleBufferFound != rleSize) {
      printf("WARNING: RLE buffer size inconsistent.\n");
   }

   sub_setRleSize(rleBufferFound);
   int alphaSum = 0;
   int alphaUpdate[4];
   int alphaUpdateSum;
   int delay = -1;
   bool ColAlphaUpdate = false;

   DEBUG3("SP_DSQT at ofs: %ld (0x%lx)\n", ctrlOfs, ctrlOfs);

   int b;
   int index = 0;

   int endSeqOfs = (((ctrlHeader[index + 1] & 0xff) | ((ctrlHeader[index] & 0xff) << 8)) - ctrlOfsRel) - 2;
   DEBUG3("endSeqOfs becomes %d (0x%x)\n", endSeqOfs, endSeqOfs);
   if (endSeqOfs < 0 || endSeqOfs > ctrlSize) {
      printf("WARNING: Invalid end sequence offset (1) -> no end time\n");
      DEBUG4("endSeqOfs becomes %d. ctrlSize is 0x%02x. ctrlOfsRel is 0x%04x.",
             endSeqOfs, ctrlSize, ctrlOfsRel);
      DEBUG2(" index=%d\n", index);
      endSeqOfs = ctrlSize;
   }
   
   index += 2;
   bool atEnd = false;
   while (!atEnd && (index < endSeqOfs)) {
      int cmd = ctrlHeader[index++] & 0xff;
      switch (cmd) {
      case 0: // forced (?)
         {
            sub_setForced(true);
         } break;
      case 1: // start display
         break;
      case 3: // palette info
         {
            b = ctrlHeader[index++] & 0xff;
            // set palette entries 0 through 3
            sub_setPalette(3, (b >> 4));
            sub_setPalette(2, (b & 0x0f));
            b = ctrlHeader[index++] & 0xff;
            sub_setPalette(1, (b >> 4));
            sub_setPalette(0, (b & 0x0f));
            DEBUG5("Palette: %02x, %02x, %02x, %02x\n",
                   sub_getPalette(0), sub_getPalette(1),
                   sub_getPalette(2), sub_getPalette(3));
         } break;
      case 4: // alpha info
         {
            b = ctrlHeader[index++] & 0xff;
            // set alpha entries 0 through 3
            sub_setAlpha(3, (b >> 4));
            sub_setAlpha(2, (b & 0x0f));
            b = ctrlHeader[index++] & 0xff;
            sub_setAlpha(1, (b >> 4));
            sub_setAlpha(0, (b & 0x0f));
            for (i = 0; i<4; i++) {
               alphaSum += sub_getAlpha(i) & 0xff;
            }
            DEBUG5("Alpha: %02x, %02x, %02x, %02x\n",
                   sub_getAlpha(0), sub_getAlpha(1),
                   sub_getAlpha(2), sub_getAlpha(3));
         } break;
      case 5: // coordinates
         {
            // Note: this always defines one window and one image of the same
            // size, so refactored the original code to handle that
            int xOfs = ((ctrlHeader[index] & 0xff) << 4) | ((ctrlHeader[index + 1] & 0xff) >> 4);
            int imageWidth = ((((ctrlHeader[index + 1] & 0xff) & 0xf) << 8) | (ctrlHeader[index + 2] & 0xff));
            int yOfs = ((ctrlHeader[index + 3] & 0xff) << 4) | ((ctrlHeader[index + 4] & 0xff) >> 4);
            int imageHeight = ((((ctrlHeader[index + 4] & 0xff) & 0xf) << 8) | (ctrlHeader[index+5] & 0xff));
            sub_setWindowSizes(sub_x_offset + xOfs, sub_y_offset + yOfs,
                               (imageWidth - xOfs) + 1,
                               (imageHeight - yOfs) + 1);
            sub_setNumCompObjects(1);
            sub_setImageSizes(sub_x_offset + xOfs, sub_y_offset + yOfs,
                              (imageWidth - xOfs) + 1,
                              (imageHeight - yOfs) + 1);
            sub_setNumberOfWindows(1);
            sub_addObject();
            index += 6;
         } break;
      case 6: // offset to RLE buffer
         {
            sub_setEvenOffset(((ctrlHeader[index + 1] & 0xff) | ((ctrlHeader[index] & 0xff) << 8)) - 4);
            sub_setOddOffset(((ctrlHeader[index + 3] & 0xff) | ((ctrlHeader[index + 2] & 0xff) << 8)) - 4);
            index += 4;
         } break;
      case 7: // color/alpha update
         {
            DEBUG1("color/alpha update\n");
            ColAlphaUpdate = true;
            // ignore the details for now, but just get alpha and palette info
            alphaUpdateSum = 0;
            b = ctrlHeader[index + 10] & 0xff;
            // set alpha updates 0 through 3
            alphaUpdate[3] = (b >> 4);
            alphaUpdate[2] = (b & 0x0f);
            b = ctrlHeader[index + 11] & 0xff;
            alphaUpdate[1] = (b >> 4);
            alphaUpdate[0] = (b & 0x0f);
            for (i = 0; i < 4; i++) {
               alphaUpdateSum += alphaUpdate[i] & 0xff;
            }
            // only use more opaque colors
            if (alphaUpdateSum > alphaSum) {
               alphaSum = alphaUpdateSum;
               for (i = 0; i < 4; ++i) {
                  sub_setAlpha(i, alphaUpdate[i]);
               }
               // take over frame palette
               b = ctrlHeader[index + 8] & 0xff;
               sub_setPalette(3, (b >> 4));
               sub_setPalette(2, (b & 0x0f));
               b = ctrlHeader[index + 9];
               sub_setPalette(1, (b >> 4));
               sub_setPalette(0, (b & 0x0f));
            }
            // search end sequence
            index = endSeqOfs;
            delay = ((ctrlHeader[index + 1] & 0xff) | ((ctrlHeader[index] & 0xff) << 8)) * 1024;
            endSeqOfs = (((ctrlHeader[index + 3] & 0xff) | ((ctrlHeader[index + 2] & 0xff) << 8)) - ctrlOfsRel) - 2;
            if (endSeqOfs < 0 || endSeqOfs > ctrlSize) {
               printf("WARNING: Invalid end sequence offset (2) -> no end time\n");
               
               endSeqOfs = ctrlSize;
            }
            index += 4;
         } break;
      case 0xff: // end sequence
         DEBUG1("At end\n");
         atEnd = true;
         break;
      default:
         {
            printf("WARNING: Unknown control sequence %x skipped\n",
                   cmd);
         } break;
      }
   }

   if (endSeqOfs != ctrlSize) {
      DEBUG3("endSeqOfs (%d) != ctrlSize (%d)\n", endSeqOfs, ctrlSize);
      int ctrlSeqCount = 1;
      index = -1;
      int nextIndex = endSeqOfs;
      while (nextIndex != index) {
         index = nextIndex;
         delay = ((ctrlHeader[index + 1] & 0xff) | ((ctrlHeader[index] & 0xff) << 8)) * 1024;
         nextIndex = (((ctrlHeader[index + 3] & 0xff) | ((ctrlHeader[index + 2] & 0xff) << 8)) - ctrlOfsRel) - 2;
         ctrlSeqCount++;
      }
      if (ctrlSeqCount > 2) {
         printf("WARNING: Control sequence(s) ignored - result may be erratic.");
      }
      sub_setEndTime(sub_getStartTime() + delay);
      prevDelay = delay;
   } else {
      // No delay found -- not sure what it means, but set some kind of an
      // end time
      
      sub_setEndTime(sub_getStartTime() + prevDelay);
   }
   
   if (ColAlphaUpdate) {
      printf("WARNING: Palette update/alpha fading detected - result may be erratic.\n");
   }
   
   if (alphaSum == 0) {
      if (sub_getFixZeroAlpha()) {
         for (i = 0; i < 4; ++i) {
            sub_setAlpha(i, sub_lastAlpha[i]);
         }
               
         printf("WARNING: Invisible caption due to zero alpha - used alpha info of last caption.\n");
      } else {
         printf("WARNING: Invisible caption due to zero alpha (not fixed due to user setting).\n");
      }
   }
   
   for (i = 0; i < 4; i++) {
      // remember current alpha settings for next image
      sub_lastAlpha[i] = sub_getAlpha(i);
   }
   
   // pic.setOriginal();
   
   DEBUG1("\n");

}

/* ============================================================ */
/* Decoding the RLE data                                        */
/* This is based on the Subtitles/substreamdvd.cpp file in the  */
/* BDSup2SubPlusPlus project.                                   */
/* ============================================================ */

char *sub_img = NULL;
int sub_img_size = 0;

void sub_imgReset() {
   int i;
   if (sub_img == NULL) {
      sub_img_size = (sub_width + 1) * sub_height + 1;
      DEBUG2("sub image size: %d\n", sub_img_size);
      sub_img = malloc(sub_img_size);
   }
   memset(sub_img, ' ', sub_img_size);
   for (i = 0; i < sub_height; i++) {
      int off = (sub_width + 1) * i + sub_width;
      sub_img[off] = '\n';
   }
   sub_img[sub_img_size - 1] = '\0';
}

void sub_imgSet(int x, int y, char ch) {
   if (sub_img == NULL) {
      sub_imgReset();
   }
   if ((x >= sub_width) || (y >= sub_height)) {
      printf("Setting (%d, %d) >= (%d, %d)\n", x, y, sub_width, sub_height);
      return;
   }
   int offset = (sub_width + 1) * y + x;
   if (sub_offset < sub_img_size) {
      sub_img[offset] = ch;
   }
}

char sub_imgGet(int x, int y) {
   int offset = (sub_width + 1) * y + x;
   if (sub_offset < sub_img_size) {
      return sub_img[offset];
   }
   return ' ';
}

int sub_hex(char c) {
   if ((c >= '0') && (c <= '9')) {
      return c - '0';
   }
   if ((c >= 'a') && (c <= 'f')) {
      return c - 'a' + 10;
   }
   if ((c >= 'A') && (c <= 'F')) {
      return c - 'A' + 10;
   }
   return 0;
}

void sub_set_pixel(Bitmap *bm, int x, int line, uchar c) {
   sub_imgSet(x, line, c + '0');
   if (c >= SUB_RLE_PALETTE_SIZE) {
      printf("ERROR: color value %d > %d\n", c, SUB_RLE_PALETTE_SIZE);
      return;
   }
   if (c >= SUB_RLE_ALPHA_SIZE) {
      printf("ERROR: color value %d > %d\n", c, SUB_RLE_ALPHA_SIZE);
      return;
   }
   uchar color = sub_rle_palette[c];
   uchar alpha = sub_rle_alpha[c];
   if (color >= SUB_NR_PALETTE) {
      printf("ERROR: RLE color value %d > %d\n", color, SUB_NR_PALETTE);
      return;
   }

   // for brightness, add the RGB values and look at the total. Note: 
   // use green twice
   int value = sub_hex(sub_palette[color][0]); // red
   value += sub_hex(sub_palette[color][2]); // green
   value += sub_hex(sub_palette[color][2]); // green again
   value += sub_hex(sub_palette[color][4]); // blue
   // now value is in the range of 0 to 60, indicating brightness

   // apply the alpha value. A value of 15 is fully opague, 0 is fully
   // transparent
   value *= alpha;
   // value is now in the range of 0 to 900
   value /= 9;
   // value is now in the range of 0 to 100

   char pixel = ' ';
   if (value >= util_get_threshold()) {
      pixel = '*';
   }

   // printf("Set pixel (%d, %d) to '%c'\n", x, line, pixel);
   bitmap_set_bit(bm, x, line, (pixel == '*'));
   // img[trgOfs + x] = sub_get_pixel_value((uchar)col);
}


void sub_decodeLine(unsigned char *rleBuf, int srcOfs, int srcLen, Bitmap *bm,
                    int line, int width, int maxPixels) {
   int nibbles_size = srcLen * 2;
   uchar *nibbles = malloc(nibbles_size);
   int i;
   int b;

   int trgOfs = line * width;
   for (i = 0; i < srcLen; i++) {
      b = rleBuf[srcOfs + i] & 0xff;
      nibbles[2 * i] = (uchar)(b >> 4);
      nibbles[2 * i + 1] = (uchar)(b & 0x0f);
   }

   int index = 0;
   int sumPixels = 0;
   int x = 0;

   while (index < nibbles_size && sumPixels < maxPixels) {
      int len;
      int col;
      b = nibbles[index++] & 0xff;
      if (b == 0) {
         // three or four nibble code
         b = nibbles[index++] & 0xff;
         if ((b & 0xc) != 0) {
            // three byte code
            len = b << 2;
            b = nibbles[index++] & 0xff;
            len |= (b >> 2);
         } else {
            // line feed or four nibble code
            len = b << 6;
            b = nibbles[index++] & 0xff;
            len |= (b << 2);
            b = nibbles[index++] & 0xff;
            len |= (b >> 2);
            if (len == 0) {
               // line feed
               len = width - x;
               if (len <= 0 || sumPixels >= maxPixels) {
                  len = 0;
                  // handle line feed
                  trgOfs += 2 * width; // lines are interlaced!
                  line += 2;
                  sumPixels = ((trgOfs / width) / 2) * width;
                  x = 0;
               }
               if ((index & 1) == 1) {
                  ++index;
               }
            }
         }
      } else {
         // one or two nibble code
         len = b >> 2;
         if (len == 0) {
            // two nibble code
            len = b << 2;
            b = nibbles[index++] & 0xff;
            len |= (b >> 2);
         }
      }

      col = b & 0x3;
      sumPixels += len;
      

      // uchar* pixels = trg.bits();
      for (i = 0; i < len; ++i) {
         sub_set_pixel(bm, x, line, col);
         if (++x >= width) {
            trgOfs += (2 * width); // lines are interlaced!
            line += 2;
            x = 0;
            if ((index & 1) == 1) {
               index++;
            }
         }
      }
   }

   free(nibbles);
}

void subformat_decode_rle(Subtitle *subtitle) {
   int width = sub_img_width;
   int height = sub_img_height;
   int sizeEven = 0;
   int sizeOdd = 0;

   if (sub_odd_offset > sub_even_offset) {
      sizeEven = sub_odd_offset - sub_even_offset;
      sizeOdd = sub_fragments_size - sub_odd_offset;
   } else {
      sizeOdd = sub_even_offset - sub_odd_offset;
      sizeEven = sub_fragments_size = sub_even_offset;
   }
   if ((sizeEven <= 0) || (sizeOdd <= 0)) {
      printf("ERROR: corrupt buffer offset information even=%d, odd=%d\n",
             sizeEven, sizeOdd);
      exit(0);
   }
   DEBUG3("Even size %d, odd size %d\n", sizeEven, sizeOdd);
   DEBUG3("Image size (%d x %d)\n", width, height);

   Bitmap bm = subtitle_bitmap(subtitle);

   /*
   int i, j;
   for (j = 0; j < height; j++) {
      for (i = 0; i < width; i++) {
         sub_imgSet(i, j, ' ');
      }
   }
   */
   sub_decodeLine(sub_fragments, sub_even_offset, sizeEven, bm,
                  0, width, width * (height / 2) + (height & 1));
   sub_decodeLine(sub_fragments, sub_odd_offset, sizeOdd, bm,
                  1, width, (height / 2) * width);

   /*
   printf("--- image ---\n");
   for (j = 0; j < height; j++) {
      bool isEmpty = true;
      for (i = 0; i < width; i++) {
         if (sub_imgGet(i, j) != ' ') {
            isEmpty = false;
            break;
         }
      }
      if (!isEmpty) {
         for (i = 0; i < width; i++) {
            printf("%c", sub_imgGet(i, j));
         }
         printf("\n");
      }
   }
   printf("--- end image ---\n");
   */
}

bool subformat_load(char *idx_fname, char *sub_fname) {
   sub_reset();
   sub_read_idx(idx_fname);
   
   FILE *fp = fopen(sub_fname, "rb");
   fseek(fp, 0L, SEEK_END);
   long sub_size = ftell(fp);

   int i;
   for (i = 0; i < sub_index_count; i++) {
      sub_startRle();
      sub_imgReset();
      Subtitle subtitle = subtitle_get_new();
      subtitle_set_video_size(subtitle, sub_width, sub_height);
      long frame_length = sub_size - sub_index[i].offset;
      if (i < (sub_index_count - 1)) {
         frame_length = sub_index[i + 1].offset - sub_index[i].offset;
      }
      sub_setStartTime(sub_index[i].timestampNum);
      sub_read_frame(fp, sub_index[i].offset, frame_length);
      DEBUG4("Got frame %d from %s to %s\n", i, sub_index[i].timestamp, 
             timeFromNum(sub_getEndTime()));
      subtitle_set_start_time(subtitle, sub_getStartTime() / 90);
      subtitle_set_end_time(subtitle, sub_getEndTime() / 90);
      subformat_decode_rle(subtitle);
      sub_endRle();
      DEBUG1("\n");
   }
   return true;
}
