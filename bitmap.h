/*
 * bitmap
 *
 * Maintain a bitmap structure
 */

#ifndef BITMAP_INCLUDED
#define BITMAP_INCLUDED

// Declare a generic bitmap structure, exposed outside of the module.
// None of the bitmap structure's internal workings are published.
#ifndef Bitmap
typedef void *Bitmap;
#endif

// A single bit is a boolean
typedef bool Bit;

// Create a new generic bitmap
Bitmap bitmap_create();

// Destroy the bitmap; it cannot be used after this
void bitmap_destroy(Bitmap bm);

// Width and height can be set explicitly, but will also be updated
// implicitly by adding bits
void bitmap_set_width(Bitmap bm, int width);
void bitmap_set_height(Bitmap bm, int height);

int bitmap_get_width(Bitmap bm);
int bitmap_get_height(Bitmap bm);

// Set or clear a bit in the bitmap. The bitmap's size is automatically
// extended if needed.
void bitmap_set_bit(Bitmap bm, int x, int y, Bit bit);

// Get a bit from the bitmap. If the requested bit is outside the
// bitmap's size (even if negative coordinates are used), false is
// returned.
Bit bitmap_get_bit(Bitmap bm, int x, int y);

// Dump the bitmap to stdout
void bitmap_dump(Bitmap bm);

#endif // BITMAP_INCLUDED
