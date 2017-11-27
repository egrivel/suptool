#include "common.h"

/* ====================================================================== */
/* Support functions to deal with the byte order in the .sup data files.  */
/* This should really be implemented in an architecture-independent way.  */
/* ====================================================================== */

unsigned int swap_int_32(int_32 i) {
   return
      ((i & 0x000000ff) << 24)
      | ((i & 0x0000ff00) << 8)
      | ((i & 0x00ff0000) >> 8)
      | ((i & 0xff000000) >> 24);
}

unsigned short swap_int_16(int_16 s) {
   return
      ((s & 0x00ff) << 8)
      | ((s & 0xff00) >> 8);
}

char *get_style_name(int style) {
  switch (style) {
  case STYLE_NORMAL:  return "normal";
  case STYLE_ITALIC:  return "italic";
  case STYLE_EITHER:  return "either";
  default: return "unknown";
  }
}
