/* ====================================================================== */
/* Definition of the internal data structure that holds a single          */
/* subtitle image and associated data                                     */
/* ====================================================================== */

#ifndef SUBTITLE_INCLUDED
#define SUBTITLE_INCLUDED
#include "bitmap.h"

#ifndef Subtitle
typedef void *Subtitle;
#endif

Subtitle subtitle_get_new();
Subtitle subtitle_get_current();
Subtitle subtitle_get(int nr);
int subtitle_count(); // get_nr_subtitles()
void subtitle_clean();

Bitmap subtitle_bitmap(Subtitle sbt);
int subtitle_get_height(Subtitle sbt);
int subtitle_get_width(Subtitle sbt);
int subtitle_get_video_height(Subtitle sbt);
int subtitle_get_video_width(Subtitle sbt);
long subtitle_get_start_time(Subtitle sbt);
long subtitle_get_end_time(Subtitle sbt);
void subtitle_set_start_time(Subtitle sbt, long start_time);
void subtitle_set_end_time(Subtitle sbt, long end_time);

void subtitle_set_position(Subtitle sbt,
                           int x_offset,
                           int y_offset);
void subtitle_set_size(Subtitle sbt,
                       int width,
                       int height);
void subtitle_set_video_size(Subtitle sbt,
                             int width,
                             int height);

void subtitle_set_palette_ycrcb(Subtitle sbt, int index,
                                int val_Y, int val_Cr, int val_Cb,
                                int val_alpha);

/* Analyze the palette, determining which palette entries will result in
   a true bit and which will result in a false bit */
void subtitle_analyze_palette(Subtitle sbt);

/*
 * Determine if a pixel of palette entry <index> is visible
 */
bool subtitle_is_visible(Subtitle sbt, int index);

/*
 * Get the position of the subtitle on the screen
 */
position subtitle_get_position(Subtitle sbt);

/* backward compatibility */
#define get_new_subtitle()        subtitle_get_new()
#define get_current_subtitle()    subtitle_get_current()
#define get_subtitle(nr)          subtitle_get(nr)
#define get_nr_subtitles()        subtitle_count()

#endif // SUBTITLE_INCLUDED
