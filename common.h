#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdlib.h>

typedef enum {
   TOP_LEFT,
   TOP,
   TOP_RIGHT,
   LEFT,
   CENTER,
   RIGHT,
   BOTTOM_LEFT,
   BOTTOM,
   BOTTOM_RIGHT,
} position;

/* Define the different sizes used */
typedef unsigned char    int_8;
typedef unsigned short   int_16;
typedef unsigned int     int_32;
typedef long             int_64;

/* conversion functions */
unsigned int swap_int_32(int_32 i);
unsigned short swap_int_16(int_16 s);

#define UNUSED(e) do { (void)(e); } while(0)
