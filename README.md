# Subtitle support tools

This repository contains my personal set of tools to support processing
subtitles from the binary format included in DVD or BluRay disks to the
`.srt` or `.ass` text format.

The repository is self-contained; all it needs is a C compiler to compile
the various applications. The information for how to interpret the various
files has been gleaned from various online descriptions, open source tools,
and the like.

---

## readsup tool ##
In the first version, the `readsup` tool is able to
 - parse idx/sub files (DVD) and idx/sup files (BluRay) and identify individual
   letters used in them as bitmap
 - use a `readsub.data` file that has bitmaps of known letters with the
   character to replace them with, as well as the style (normal or italic)
 - output a `.srt` or `.ass` text file with the subtitle information for the
   known letters
 - output, on standard out, a list of letters that are not yet known.

The workflow for working with `readsub` is:
 - process a subtitle file
 - capture the standard output with the as-of-yet unknown letters
 - edit the output of unkown letters to add the letter and style information
 - add the result to the `readsup.data` archive of known letters
 - re-process the subtitle file; all text should now be recognized
 - spot-check the output

Work is ongoing to make `readsup` guess for characters that it doesn't know
yet.

---

## decode_base tool ##

Decode a "compressed" string to hexadecimal. Usage:
```
./decode_base <character string>
```
The output will be a series of hexadecimal numbers that are the decoded bytes
from the input string.

---

## string_to_char tool ##

Parse a "compressed" string and output the character representation. Usage:
```
./string_to_char <character string>
```
The output will be an ASCII bitmap representation of the character, similar
to how characters are stored in the `readsup.data` file.

---

## Source Code ##

The following describes the different source code modules, in alphabetical
order.

### bitmap ###

Defines a structure `Bitmap` and supports operations on it. The bitmap
structure is how the graphical representation of a character is maintained
within the system. The following functions are supported:
 - `Bitmap bitmap_create()` creates a new bitmap (without data) and returns
   a reference to it.
 - `void bitmap_destroy(Bitmap bm)` destroys a bitmap structure and frees u
   the memory used by it.
 - `void bitmap_set_width(Bitmap bm, int width)` updates the width of a
   bitmap. Note that setting bits in the bitmap also implicitly grows the
   bitmap.
 - `void bitmap_set_height(Bitmap bm, int height)` updates the height of a
   bitmap. Note that setting bits in the bitmap also implicitly grows the
   bitmap.
 - `int bitmap_get_width(Bitmap bm)` returns the current width of the bitmap.
 - `int bitmap_get_height(Bitmap bm)` returns the current height of the bitmap.
 - `void bitmap_set_bit(Bitmap bm, int x, int y, bool bit)` sets the bit at
   the specified position. The bitmap is extended if necessary to accomodate
   the position.
 - `bool bitmap_get_bit(Bitmap bm, int x, int y)` returns the bit at the
   requested position. If the coordinates are outside of the bitmap's size,
   `false` is returned.
 - `void bitmap_dump(Bitmap bm)` dumps the bitmap on `stdout`

### charlist ###

### charmatch ###

### common ###

### output ###

### subformat ###

### subprop ###

### subtitle ###

### supformat ###

### util ###
