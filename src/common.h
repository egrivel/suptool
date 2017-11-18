#ifndef _COMMON_INCLUDED
#define _COMMON_INCLUDED

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

// Use a trick to get static asserts
#define CASSERT(predicate, file) _impl_CASSERT_LINE(predicate,__LINE__,file)

#define _impl_PASTE(a,b) a##b
#define _impl_CASSERT_LINE(predicate, line, file) \
    typedef char _impl_PASTE(assertion_failed_##file##_,line)[2*!!(predicate)-1];

// The following assertions, when they fail, will give weird messages
CASSERT(sizeof(int_8) == 1, common_h);
CASSERT(sizeof(int_16) == 2, common_h);
CASSERT(sizeof(int_32) == 4, common_h);
CASSERT(sizeof(int_64) == 8, common_h);

/* conversion functions */
int_16 swap_int_16(int_16 s);
int_32 swap_int_32(int_32 i);

#define UNUSED(e) do { (void)(e); } while(0)

#endif /* _COMMON_INCLUDED */
