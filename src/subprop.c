/*
 * Properties of the current subtitle set.
 */

#include "common.h"
#include "subprop.h"

int gl_subprop_base_height = 0;
int gl_subprop_asc_height = 0;
int gl_subprop_desc_height = 0;
int gl_subprop_normal_space_width = 7;
bool gl_subprop_normal_space_override = false;
int gl_subprop_italic_space_width = 5;
bool gl_subprop_italic_space_override = false;
int gl_subprop_base_width = 0;

void set_base_height(int height) {
   gl_subprop_base_height = height;
}

int get_base_height() {
   return gl_subprop_base_height;
}

void set_asc_height(int height) {
   gl_subprop_asc_height = height;
}

int get_asc_height() {
   return gl_subprop_asc_height;
}

void set_desc_height(int height) {
   gl_subprop_desc_height = height;
}

int get_desc_height() {
   return gl_subprop_desc_height;
}

void set_normal_space_width(int width) {
   if (!gl_subprop_normal_space_override) {
      printf("# Normal space width %d\n", width);
      gl_subprop_normal_space_width = width;
   }
}

//
// Can override space with in data file; after that, stays fixed
// (except for command line override)
//
void override_normal_space_width(int width) {
   printf("# Normal space width overridden %d\n", width);
   gl_subprop_normal_space_width = width;
   gl_subprop_normal_space_override = true;
}

int get_normal_space_width() {
   return gl_subprop_normal_space_width;
}

void set_italic_space_width(int width) {
   if (!gl_subprop_italic_space_override) {
      printf("# Italic space width %d\n", width);
      gl_subprop_italic_space_width = width;
   }
}

//
// Can override space with in data file; after that, stays fixed
// (except for command line override)
//
void override_italic_space_width(int width) {
   printf("# Italic space width overridden %d\n", width);
   gl_subprop_italic_space_width = width;
   gl_subprop_italic_space_override = true;
}

int get_italic_space_width() {
   return gl_subprop_italic_space_width;
}

void set_base_width(int width) {
   gl_subprop_base_width = width;
}

int get_base_width() {
   if (gl_subprop_base_width == 0) {
      return (13 * gl_subprop_base_height) / 15;
   }
   return gl_subprop_base_width;
}
