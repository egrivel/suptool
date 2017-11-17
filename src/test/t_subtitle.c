#include "unittest.h"
#include "../common.h"
#include "../subtitle.h"
#include "../bitmap.h"

void test1() {
   unit_start("Create and destroy");
   unit_step("get NULL subtitle when starting");
   unit_ptr_null(subtitle_get_current());
   unit_step("number of subtitles should be zero");
   unit_int_equal(subtitle_count(), 0);
   unit_step("Get a new subtitle");
   unit_ptr_non_null(subtitle_get_new());
   unit_step("number of subtitles should be one");
   unit_int_equal(subtitle_count(), 1);
   unit_step("Get current subtitle");
   unit_ptr_non_null(subtitle_get_current());
   unit_step("Get existing subtitle");
   unit_ptr_non_null(subtitle_get(0));
   unit_step("Get non-existing subtitle");
   unit_ptr_null(subtitle_get(1));
   unit_step("Clean subtitle list");
   subtitle_clean();
   unit_step("number of subtitles should be zero");
   unit_int_equal(subtitle_count(), 0);
   unit_done();
}

void test2() {
   unit_start("Access bitmap");
   unit_step("Get a new subtitle");
   Subtitle sbt = subtitle_get_new();
   unit_ptr_non_null(sbt);
   unit_step("Get a bitmap");
   Bitmap bm = subtitle_bitmap(sbt);
   unit_ptr_non_null(bm);
   unit_step("Bitmap width is zero");
   unit_int_equal(bitmap_get_width(bm), 0);
   unit_step("Bitmap height is zero");
   unit_int_equal(bitmap_get_height(bm), 0);
   unit_step("Set subtitle width=3 and height=7");
   subtitle_set_size(sbt, 3, 7);
   unit_step("Check width is 3");
   unit_int_equal(subtitle_get_width(sbt), 3);
   unit_step("Check height is 7");
   unit_int_equal(subtitle_get_height(sbt), 7);
   unit_step("Bitmap width is still zero");
   unit_int_equal(bitmap_get_width(bm), 0);
   unit_step("Bitmap height is still zero");
   unit_int_equal(bitmap_get_height(bm), 0);
   unit_step("Create a new subtitle");
   Subtitle sbt2 = subtitle_get_new();
   unit_ptr_non_null(sbt2);
   unit_step("Make sure new subtitle is different");
   unit_ptr_unequal(sbt, sbt2);
   unit_step("Set subtitle width=4 and height=8");
   subtitle_set_size(sbt2, 4, 8);
   unit_step("Get a bitmap for new subtitle");
   Bitmap bm2 = subtitle_bitmap(sbt2);
   unit_ptr_non_null(bm2);
   unit_step("Make sure new bitmap is different");
   unit_ptr_unequal(bm, bm2);
   unit_step("Check bitmap width is 4");
   unit_int_equal(bitmap_get_width(bm2), 4);
   unit_step("Check bitmap height is 8");
   unit_int_equal(bitmap_get_height(bm2), 8);

   unit_step("Clean subtitle list");
   subtitle_clean();
   unit_done();
}

void test3() {
   unit_start("Width and height");
   unit_step("Set size (3, 7) on NULL subtitle");
   subtitle_set_size(NULL, 3, 7);
   unit_step("Get subtitle");
   Subtitle sbt = subtitle_get_new();
   unit_ptr_non_null(sbt);
   unit_step("Set size (4, 8) on existing subtitle");
   subtitle_set_size(sbt, 4, 8);
   unit_step("Check width is 4");
   unit_int_equal(subtitle_get_width(sbt), 4);
   unit_step("Check height is 8");
   unit_int_equal(subtitle_get_height(sbt), 8);
   unit_step("Increase size to (5, 10)");
   subtitle_set_size(sbt, 5, 10);
   unit_step("Check width is 5");
   unit_int_equal(subtitle_get_width(sbt), 5);
   unit_step("Check height is 10");
   unit_int_equal(subtitle_get_height(sbt), 10);
   unit_step("Decrease size to (2, 6)");
   subtitle_set_size(sbt, 2, 6);
   unit_step("Check width is 2");
   unit_int_equal(subtitle_get_width(sbt), 2);
   unit_step("Check height is 6");
   unit_int_equal(subtitle_get_height(sbt), 6);
   unit_step("Set invalid size (-1, 14)");
   subtitle_set_size(sbt, -1, 14);
   unit_step("Check width is 2");
   unit_int_equal(subtitle_get_width(sbt), 2);
   unit_step("Check height is 14");
   unit_int_equal(subtitle_get_height(sbt), 14);
   unit_step("Set invalid size (7, -3)");
   subtitle_set_size(sbt, 7, -3);
   unit_step("Check width is 7");
   unit_int_equal(subtitle_get_width(sbt), 7);
   unit_step("Check height is 14");
   unit_int_equal(subtitle_get_height(sbt), 14);
   unit_step("Set invalid size (-4, -5)");
   subtitle_set_size(sbt, -4, -5);
   unit_step("Check width is 7");
   unit_int_equal(subtitle_get_width(sbt), 7);
   unit_step("Check height is 14");
   unit_int_equal(subtitle_get_height(sbt), 14);
   unit_step("Make sure (0, 0) is a valid size");
   subtitle_set_size(sbt, 0, 0);
   unit_step("Check width is 0");
   unit_int_equal(subtitle_get_width(sbt), 0);
   unit_step("Check height is 0");
   unit_int_equal(subtitle_get_height(sbt), 0);

   unit_step("Clean subtitle list");
   subtitle_clean();
   unit_done();
}

void test4() {
   unit_start("Start and end time");
   unit_step("Get subtitle");
   Subtitle sbt = subtitle_get_new();
   unit_ptr_non_null(sbt);

   unit_step("Default start time is 0");
   unit_int_equal(subtitle_get_start_time(sbt), 0);
   unit_step("Default end time is 0");
   unit_int_equal(subtitle_get_end_time(sbt), 0);

   unit_step("Set start time to 100");
   subtitle_set_start_time(sbt, 100);
   unit_step("Check start time is 100");
   unit_int_equal(subtitle_get_start_time(sbt), 100);

   unit_step("Set end time to 200");
   subtitle_set_end_time(sbt, 200);
   unit_step("Check end time is 200");
   unit_int_equal(subtitle_get_end_time(sbt), 200);

   unit_step("Clean subtitle list");
   subtitle_clean();
   unit_done();
}

void test5() {
   unit_start("Position");
   unit_step("Get subtitle");
   Subtitle sbt = subtitle_get_new();
   unit_ptr_non_null(sbt);

   unit_step("Set position to (10, 20)");
   subtitle_set_position(sbt, 10, 20);

   unit_step("Clean subtitle list");
   subtitle_clean();
   unit_done();
}

void test_it() {
   test1();
   test2();
   test3();
   test4();
   test5();
}
