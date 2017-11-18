#ifndef _SUBPROP_INCLUDED
#define _SUBPROP_INCLUDED

int get_base_height();
int get_asc_height();
int get_desc_height();
int get_normal_space_width();
int get_italic_space_width();
int get_base_width();

void set_base_height(int height);
void set_asc_height(int height);
void set_desc_height(int height);
void set_normal_space_width(int width);
void override_normal_space_width(int width);
void set_italic_space_width(int width);
void override_italic_space_width(int width);
void set_base_width(int width);

#endif /* _SUBPROP_INCLUDED */
