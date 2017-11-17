#include "unittest.h"
#include "../bitmap.h"

void test1() {
   Bitmap bm = NULL;
   unit_start("Create and destroy");
   unit_description("Create a bitmap and destroy it again.");
   unit_step("Create a bitmap");
   bm = bitmap_create();
   unit_step("Destroy a bitmap");
   bitmap_destroy(bm);
   unit_done();
}

void test2() {
   Bitmap bm = NULL;
   unit_start("Operate on NULL bitmap");
   unit_description("Try all operations on a NULL bitmap and make sure there is no crash.");
   unit_step("bitmap_destroy()");
   bitmap_destroy(bm);
   unit_step("bitmap_set_width()");
   bitmap_set_width(bm, 999);
   unit_step("bitmap_set_height()");
   bitmap_set_height(bm, 999);

   unit_step("bitmap_get_width()");
   unit_int_equal(bitmap_get_width(bm), 0);
   unit_step("bitmap_get_height()");
   unit_int_equal(bitmap_get_height(bm), 0);

   unit_step("bitmap_set_bit() clearing bit");
   bitmap_set_bit(bm, 10, 10, false);
   unit_step("bitmap_set_bit() setting bit");
   bitmap_set_bit(bm, 10, 10, true);
   unit_step("bitmap_get_bit()");
   unit_assert(!bitmap_get_bit(bm, 10, 10));

   unit_step("bitmap_dump()");
   bitmap_dump(bm);

   unit_step("destroy bitmap");
   bitmap_destroy(bm);
   unit_done();
}

// can't do this test yet, because I can't assert for segmentation
// violations   
void test3() {
   Bitmap bm = NULL;
   unit_start("Operate on invalid bitmap");
   unit_description("Try all operations on a bitmap that was previously destroyed and make sure they all crash.");
   unit_step("bitmap_create()");
   bm = bitmap_create();
   unit_step("bitmap_destroy()");
   bitmap_destroy(bm);
   unit_step("bitmap_set_width()");
   bitmap_set_width(bm, 999);
   unit_step("bitmap_set_height()");
   bitmap_set_height(bm, 999);

   unit_step("bitmap_get_width()");
   unit_int_equal(bitmap_get_width(bm), 0);
   unit_step("bitmap_get_height()");
   unit_int_equal(bitmap_get_height(bm), 0);

   unit_step("bitmap_set_bit() clearing bit");
   bitmap_set_bit(bm, 10, 10, false);
   unit_step("bitmap_set_bit() setting bit");
   bitmap_set_bit(bm, 10, 10, true);
   unit_step("bitmap_get_bit()");
   unit_assert(!bitmap_get_bit(bm, 10, 10));

   unit_step("bitmap_dump()");
   bitmap_dump(bm);

   unit_step("destroy bitmap");
   bitmap_destroy(bm);
   unit_done();
}

void test4() {
   unit_start("Get and set width");
   unit_step("bitmap_create()");
   Bitmap bm = bitmap_create();
   unit_step("Initial bitmap_get_width() must be zero");
   unit_int_equal(bitmap_get_width(bm), 0);
   unit_step("Initial bitmap_get_height() must be zero");
   unit_int_equal(bitmap_get_height(bm), 0);

   unit_step("set width to 7");
   bitmap_set_width(bm, 7);
   unit_step("verify width is 7");
   unit_int_equal(bitmap_get_width(bm), 7);
   unit_step("set height to 12");
   bitmap_set_height(bm, 12);
   unit_step("verify height is 12");
   unit_int_equal(bitmap_get_height(bm), 12);
   unit_step("verify width is still 7");
   unit_int_equal(bitmap_get_width(bm), 7);
   
   unit_step("change width to 7");
   bitmap_set_width(bm, 9);
   unit_step("verify width is 9");
   unit_int_equal(bitmap_get_width(bm), 9);
   unit_step("verify height is still 12");
   unit_int_equal(bitmap_get_height(bm), 12);
   unit_step("change height to 14");
   bitmap_set_height(bm, 14);
   unit_step("verify height is 14");
   unit_int_equal(bitmap_get_height(bm), 14);
   unit_step("verify width is still 9");
   unit_int_equal(bitmap_get_width(bm), 9);

   unit_step("change width to 5");
   bitmap_set_width(bm, 9);
   unit_step("verify width is still 9");
   unit_int_equal(bitmap_get_width(bm), 9);
   unit_step("change height to 10");
   bitmap_set_height(bm, 10);
   unit_step("verify height is 14");
   unit_int_equal(bitmap_get_height(bm), 14);

   unit_step("destroy bitmap");
   bitmap_destroy(bm);
   unit_done();
}

void test5() {
   unit_start("Getting and setting pixels");
   unit_step("bitmap_create()");
   Bitmap bm = bitmap_create();
   unit_step("Initial bitmap_get_width() must be zero");
   unit_int_equal(bitmap_get_width(bm), 0);
   unit_step("Initial bitmap_get_height() must be zero");
   unit_int_equal(bitmap_get_height(bm), 0);

   unit_step("verify bit(3, 2) is off initially");
   unit_assert(!bitmap_get_bit(bm, 3, 2));
   unit_step("verify width is still 0");
   unit_int_equal(bitmap_get_width(bm), 0);
   unit_step("verify height is still 0");
   unit_int_equal(bitmap_get_height(bm), 0);

   unit_step("set bit(3, 2) to on");
   bitmap_set_bit(bm, 3, 2, true);
   unit_step("verify bit(3, 2) is now on");
   unit_assert(bitmap_get_bit(bm, 3, 2));
   unit_step("verify width is now 4");
   unit_int_equal(bitmap_get_width(bm), 4);
   unit_step("verify height is now 3");
   unit_int_equal(bitmap_get_height(bm), 3);

   unit_step("set bit(3, 2) to off");
   bitmap_set_bit(bm, 3, 2, false);
   unit_step("verify bit(3, 2) is now off");
   unit_assert(!bitmap_get_bit(bm, 3, 2));

   unit_step("destroy bitmap");
   bitmap_destroy(bm);
   unit_done();
}

void test6() {
   unit_start("Ignore negative values on empty bitmap");
   unit_step("bitmap_create()");
   Bitmap bm = bitmap_create();

   unit_step("Set width -3");
   bitmap_set_width(bm, -3);
   unit_step("verify width is still 0");
   unit_int_equal(bitmap_get_width(bm), 0);
   unit_step("Set heigh -5");
   bitmap_set_height(bm, -5);
   unit_step("verify height is still 0");
   unit_int_equal(bitmap_get_height(bm), 0);

   unit_step("get pixel (-3, -2)");
   unit_assert(!bitmap_get_bit(bm, -3, -2));
   unit_step("get pixel (-3, 2)");
   unit_assert(!bitmap_get_bit(bm, -3, 2));
   unit_step("get pixel (3, -2)");
   unit_assert(!bitmap_get_bit(bm, 3, -2));

   unit_step("set pixel (-3, -2)");
   bitmap_set_bit(bm, -3, -2, true);
   unit_step("set pixel (-3, 2)");
   bitmap_set_bit(bm, -3, 2, true);
   unit_step("set pixel (3, -2)");
   bitmap_set_bit(bm, 3, -2, true);
   unit_step("verify width is still 0");
   unit_int_equal(bitmap_get_width(bm), 0);
   unit_step("verify height is still 0");
   unit_int_equal(bitmap_get_height(bm), 0);

   unit_step("destroy bitmap");
   bitmap_destroy(bm);
   unit_done();
}

void test7() {
   unit_start("Ignore negative values on existing bitmap");
   unit_step("bitmap_create()");
   Bitmap bm = bitmap_create();

   unit_step("Create bitmap by setting pixel (7, 4)");
   bitmap_set_bit(bm, 7, 4, true);
   unit_step("verify pixel (7, 4)");
   unit_assert(bitmap_get_bit(bm, 7, 4));
   unit_step("verify width is 8");
   unit_int_equal(bitmap_get_width(bm), 8);
   unit_step("verify height is 5");
   unit_int_equal(bitmap_get_height(bm), 5);

   unit_step("Set width -3");
   bitmap_set_width(bm, -3);
   unit_step("verify width is still 8");
   unit_int_equal(bitmap_get_width(bm), 8);
   unit_step("Set heigh -5");
   bitmap_set_height(bm, -5);
   unit_step("verify height is still 5");
   unit_int_equal(bitmap_get_height(bm), 5);

   unit_step("get pixel (-3, -2)");
   unit_assert(!bitmap_get_bit(bm, -3, -2));
   unit_step("get pixel (-3, 2)");
   unit_assert(!bitmap_get_bit(bm, -3, 2));
   unit_step("get pixel (3, -2)");
   unit_assert(!bitmap_get_bit(bm, 3, -2));
   unit_step("verify pixel (7, 4) is still on");
   unit_assert(bitmap_get_bit(bm, 7, 4));
   unit_step("verify width is still 8");
   unit_int_equal(bitmap_get_width(bm), 8);
   unit_step("verify height is still 5");
   unit_int_equal(bitmap_get_height(bm), 5);

   unit_step("set pixel (-3, -2)");
   bitmap_set_bit(bm, -3, -2, true);
   unit_step("set pixel (-3, 2)");
   bitmap_set_bit(bm, -3, 2, true);
   unit_step("set pixel (3, -2)");
   bitmap_set_bit(bm, 3, -2, true);
   unit_step("verify pixel (7, 4) is still on");
   unit_assert(bitmap_get_bit(bm, 7, 4));
   unit_step("verify width is still 8");
   unit_int_equal(bitmap_get_width(bm), 8);
   unit_step("verify height is still 5");
   unit_int_equal(bitmap_get_height(bm), 5);

   unit_step("destroy bitmap");
   bitmap_destroy(bm);
   unit_done();
}

void test_it() {
   test1();
   test2();
   //test3();
   test4();
   test5();
   test6();
   test7();
}
